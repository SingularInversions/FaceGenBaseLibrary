//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgDraw.hpp"
#include "FgMath.hpp"
#include "FgBounds.hpp"
#include "FgImgDisplay.hpp"

using namespace std;

namespace Fg {

// Simple (aliased) filled circle. Radius 0 yields a single pixel:
void
fgDrawDotIrcs(
    ImgC4UC &       img,
    Vec2I            pos,
    int                 radius,
    RgbaUC            val)
{
    FGASSERT(img.height() * img.width() > 0);
    FGASSERT(radius >= 0);
    int         xlo = cMax(pos[0]-radius,0),
                xhi = cMin(pos[0]+radius,int(img.width()-1)),
                ylo = cMax(pos[1]-radius,0),
                yhi = cMin(pos[1]+radius,int(img.height()-1)),
                rr = radius * radius;
    for (int yy=ylo; yy<=yhi; yy++) {
        for (int xx=xlo; xx<=xhi; xx++) {
            if ((sqr(yy-pos[1]) + sqr(xx-pos[0])) <= rr)
                img.xy(xx,yy) = val;
        }
    }
}

// Simple (aliased) Bresenham line draw, single-pixel thickness. Efficiency could be greatly
// improved by clippping begin/end points to valid image area, updating 'acc' appropriately,
// and then calling 'xy' instead of 'paint':
void
fgDrawLineIrcs(
    ImgC4UC &       img,
    Vec2I            beg,
    Vec2I            end,
    RgbaUC            val)
{
    Vec2I    delta = end-beg;
    Vec2I    mag;
    mag[0] = std::abs(delta[0]);
    mag[1] = std::abs(delta[1]);
    Vec2I    dir;
    dir[0] = (delta[0] >= 0) ? 1 : -1;
    dir[1] = (delta[1] >= 0) ? 1 : -1;
    Vec2I    axes;
    if (mag[0] > mag[1])    axes = Vec2I(0,1);
    else                    axes = Vec2I(1,0);
    Vec2I    pos(beg);
    int         acc = 0;

    for (int ii=0; ii<=mag[axes[0]]; ii++) {
        img.paint(pos,val);
        pos[axes[0]] += dir[axes[0]];
        acc += mag[axes[1]];
        if (acc >= mag[axes[0]]) {
            pos[axes[1]] += dir[axes[1]];
            acc -= mag[axes[0]];
        }
    }
}

void
fgPaintRectangle(
    ImgC4UC &       img,
    RgbaUC            clr,
    size_t xIrcs,   size_t yIrcs,
    size_t wid,     size_t hgt)
{
    for (size_t yy=yIrcs; yy<yIrcs+hgt; ++yy)
        for (size_t xx=xIrcs; xx<xIrcs+wid; ++xx)
            img.xy(xx,yy) = clr;
}

double
fgDrawBarGraph(
    ImgC4UC &           img,
    const vector<double> &  data,
    RgbaUC                colour)
{
    FGASSERT(data.size() > 1);
    size_t          numBins = data.size();
    if (img.width() != numBins) {                                   // Start clean if wrong size
        img.resize(uint(numBins),uint(numBins));
        fgImgFill(img,RgbaUC(0,0,0,255));
    }
    double          maxVal = cMax(data),
                    vscale = 0.9 * double(img.height()) / maxVal;
    for (uint xx=0; xx<img.width(); xx++) {
        uint    hgt = round<uint>(data[xx] * vscale);
        for (uint yy=0; yy<hgt; yy++)
            img.xy(xx,img.height()-1-yy) = colour;
    }
    return vscale;
}

void
fgDrawFunction(
    ImgC4UC &       img,            // OUTPUT
    std::function<double(double)> func,
    VecD2            bounds,         // abscissa value at image x bounds
    double              vscale,         // y pixels per unit
    RgbaUC            colour)
{
    FGASSERT(!img.empty());
    int             wid = img.width(),
                    hgt = img.height();
    double          fac = (bounds[1]-bounds[0])/double(wid);    // x pixels to units
    FGASSERT(fac > 0.0);
    Vec2I           lastPoint {0};
    for (int xx=0; xx<wid; xx++) {
        double      abscissa = (double(xx) + 0.5) * fac + bounds[0];
        int         val = round<uint>(func(abscissa)*vscale);
        Vec2I    point(xx,hgt-1-val);
        if (xx > 0)
            fgDrawLineIrcs(img,lastPoint,point,colour);   // Ignores out of bounds points
        lastPoint = point;
    }
}

void
fgDrawFunctions(
    const MatD &   funcs)      // Columns are function values
{
    uint            dim = funcs.numRows(),
                    num = funcs.numCols();
    FGASSERT((dim > 1) && (num > 0));

    // Calculate colours:
    double          clrCode = 0.0,
                    clrStep = 3.0 / num;
    vector<RgbaUC> colour(num);
    for (uint jj=0; jj<num; ++jj) {
        double      dtmp = clrCode;
        if (dtmp < 1.0) {
            colour[jj].red() = uchar(255-round<uint>(dtmp * 255.0));
            colour[jj].green() = 255 - colour[jj].red();
        }
        else if (dtmp < 2.0) {
            dtmp -= 1.0;
            colour[jj].green() = uchar(255-round<uint>(dtmp * 255.0));
            colour[jj].blue() = 255 - colour[jj].green();
        }
        else {
            dtmp -= 2.0;
            colour[jj].blue() = uchar(255-round<uint>(dtmp * 255.0));
            colour[jj].red() = 255 - colour[jj].blue();
        }
        colour[jj].alpha() = 255;
        clrCode += clrStep;
    }

    // Draw:
    VecD2s              fbounds(num);
    Doubles             fscale(num);
    for (uint jj=0; jj<num; ++jj) {
        fbounds[jj] = cBounds(funcs.colVec(jj).m_data);
        fscale[jj] = double(dim) * 0.96 / (fbounds[jj][1] - fbounds[jj][0]);
    }
    ImgC4UC             img(dim,dim,RgbaUC(0,0,0,255));
    uint                margin = round<uint>(double(dim)*0.02)+1;
    for (uint ii=0; ii<dim; ++ii) {
        for (uint jj=0; jj<num; ++jj) {
            double          hgt = (funcs.rc(ii,jj) - fbounds[jj][0]) * fscale[jj];
            img.xy(ii,dim-margin-round<uint>(hgt)) = colour[jj];
        }
    }
    imgDisplay(img);
}

}
