//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgImageDraw.hpp"
#include "FgMath.hpp"
#include "FgBounds.hpp"
#include "FgImgDisplay.hpp"
#include "FgMain.hpp"
#include "FgSyntax.hpp"
#include "FgCommand.hpp"
#include "FgParse.hpp"

using namespace std;

namespace Fg {

void                drawDotIrcs(ImgRgba8 & img,Vec2I pos,uint radius,Rgba8 color)
{
    FGASSERT(!img.empty());
    int         rad = scast<int>(radius),
                xlo = cMax(pos[0]-rad,0),
                xhi = cMin(pos[0]+rad,int(img.width()-1)),
                ylo = cMax(pos[1]-rad,0),
                yhi = cMin(pos[1]+rad,int(img.height()-1)),
                rr = rad * rad;
    for (int yy=ylo; yy<=yhi; yy++) {
        for (int xx=xlo; xx<=xhi; xx++) {
            if ((sqr(yy-pos[1]) + sqr(xx-pos[0])) <= rr)
                img.xy(xx,yy) = color;
        }
    }
}
// Simple (aliased) Bresenham line draw, single-pixel thickness. Efficiency could be greatly
// improved by clippping begin/end points to valid image area, updating 'acc' appropriately,
// and then calling 'xy' instead of 'paint':
void                drawLineIrcs(
    ImgRgba8 &       img,
    Vec2I            beg,
    Vec2I            end,
    Rgba8            val)
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
void                drawSolidRectangle_(ImgRgba8 & img,Vec2UI pos,Vec2UI sz,Rgba8 clr)
{
    Vec2UI              dims = img.dims();
    for (uint dd=0; dd<2; ++dd) {
        if (pos[dd] >= dims[dd])
            return;
        sz[dd] = cMin(sz[dd],dims[dd]-pos[dd]);
    }
    for (Iter2UI it{sz}; it.valid(); it.next())
        img[pos+it()] = clr;
}
ImgRgba8            cBarGraph(Doubless const & datas,uint pixPerBar,uint pixSpacing)
{
    FGASSERT(!datas.empty());
    size_t              D = datas.size(),                   // data per sample (not all may be valid)
                        S = cMax(cSizes(datas)),            // samples
                        P = D*S*pixPerBar + S*pixSpacing,   // pixel size of image
                        xx {0};                             // image x coord counter (pixels)
    ImgRgba8             img {P,P,Rgba8{0,0,0,255}};
    double              maxVal = cMax(flatten(datas));
    for (size_t ss=0; ss<S; ++ss) {
        size_t              cIdx {0};
        for (Doubles const & data : datas) {
            if (ss < data.size()) {
                Rgba8              clr {0,0,0,255};
                clr[cIdx] = 255;
                double              v = data[ss];
                uint                hgt = round<uint>((P-1) * v / maxVal);
                Vec2UI              posIrcs {uint(xx),img.height()-hgt-1},
                                    sz {pixPerBar,hgt};
                drawSolidRectangle_(img,posIrcs,sz,clr);
            }
            xx += pixPerBar;
            cIdx = (cIdx+1)%3;
        }
        xx += pixSpacing;
    }
    return img;
}
ImgRgba8            cBarGraph(Sizes const & data)
{
    Doubles         dt = mapCast<double>(data);
    return cBarGraph({dt},1);
}
void                drawFunction(
    ImgRgba8 &          img,            // OUTPUT
    Sfun<double(double)> func,
    VecD2               bounds,         // abscissa value at image x bounds
    double              vscale,         // y pixels per unit
    Rgba8               colour)
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
            drawLineIrcs(img,lastPoint,point,colour);   // Ignores out of bounds points
        lastPoint = point;
    }
}
void                drawFunctions(
    MatD const &   funcs)      // Columns are function values
{
    size_t          dim = funcs.numRows(),
                    num = funcs.numCols();
    FGASSERT((dim > 1) && (num > 0));

    // Calculate colours:
    double          clrCode = 0.0,
                    clrStep = 3.0 / num;
    vector<Rgba8> colour(num);
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
    ImgRgba8             img(dim,dim,Rgba8(0,0,0,255));
    uint                margin = round<uint>(double(dim)*0.02)+1;
    for (uint ii=0; ii<dim; ++ii) {
        for (uint jj=0; jj<num; ++jj) {
            double          hgt = (funcs.rc(ii,jj) - fbounds[jj][0]) * fscale[jj];
            img.xy(ii,dim-margin-round<uint>(hgt)) = colour[jj];
        }
    }
    viewImage(img);
}
void                cmdGraph(CLArgs const & args)
{
    Syntax                  syn{args,"[-b <num>] [-g <num>] (<data>.txt)+ [<image>.jpg]\n"
        "   Creates bar graph of given data sequence(s), display, and optionally save.\n"
        "   -b      Pixel thickness of bars (default 6)\n"
        "   -g      Pixel thickness of gap between bars (default 6)\n"
    };
    Doubless                datas;
    uint                    barThick = 8,
                            gapThick = 8;
    while(true) {
        if (!syn.more())
            break;
        if (syn.peekNext()[0] == '-') {
            if (syn.next() == "-b")
                barThick = syn.nextAs<uint>();
            else if (syn.curr() == "-g")
                gapThick = syn.nextAs<uint>();
            else
                syn.error("Unrecognized option",syn.curr());
            continue;
        }
        if (pathToExt(syn.peekNext()) != "txt")
            break;
        Strings             lines = splitLines(loadRaw(syn.next()));
        Doubles             data;
        for (string const & line : lines) {
            Opt<double>         opt = fromStr<double>(line);
            FGASSERT1(opt.valid(),"Invalid line in text file "+syn.curr()+" : "+line);
            data.push_back(opt.val());
        }
        if (data.empty())
            syn.error("Data file is empty",syn.curr());
        datas.push_back(data);
    };
    if (datas.empty())
        syn.error("No data files given");
    viewImage(cBarGraph(datas,barThick,gapThick));
}

Cmd                 getCmdGraph()
{
    return Cmd{cmdGraph,"graph","Create simple bar graphs from text data"};
}

}
