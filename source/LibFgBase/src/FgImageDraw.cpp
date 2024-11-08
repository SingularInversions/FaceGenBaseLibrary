//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgImageDraw.hpp"
#include "FgMath.hpp"
#include "FgBounds.hpp"
#include "FgImgDisplay.hpp"
#include "FgMain.hpp"
#include "FgFileSystem.hpp"
#include "FgCommand.hpp"
#include "FgParse.hpp"

using namespace std;

namespace Fg {

Arr<Arr3F,6>        cColors6()
{
    return {
        {1,0,0},
        {0,1,0},
        {0,0,1},
        {1,1,0},
        {0,1,1},
        {1,0,1},
    };
}

Arr<Rgba8,6>        cColors6Rgba()
{
    uchar constexpr     m = 255;
    return {
        {m,0,0,m},
        {0,m,0,m},
        {0,0,m,m},
        {m,m,0,m},
        {0,m,m,m},
        {m,0,m,m},
    };
}

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
void                drawLineIrcs(ImgRgba8 & img,Vec2I beg,Vec2I end,Rgba8 val)
{
    Vec2I               delta = end-beg,
                        delAbs = mapAbs(delta),
                        delDir = mapCall(delta,[](int v){return (v >= 0) ? 1 : -1; }),
                        axes = (delAbs[0] > delAbs[1]) ? Vec2I{0,1} : Vec2I{1,0},
                        pos = beg;
    int                 acc = 0;
    for (int ii=0; ii<=delAbs[axes[0]]; ii++) {
        img.paint(pos,val);
        pos[axes[0]] += delDir[axes[0]];
        acc += delAbs[axes[1]];
        if (acc >= delAbs[axes[0]]) {
            pos[axes[1]] += delDir[axes[1]];
            acc -= delAbs[axes[0]];
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

ImgRgba8            cBarGraph(Doubless datas,uint pixPerBar,uint pixSpacing)
{
    FGASSERT(!datas.empty());
    size_t              D = datas.size(),                   // number of data series (may not all may be of same size)
                        S = cMaxElem(cSizes(datas)),        // max number of samples per series
                        P = S*(D*pixPerBar + pixSpacing),   // pixel width of image
                        xx {0};                             // image x coord counter (pixels)
    ImgRgba8            img {P,P,Rgba8{0,0,0,255}};         // make a square image
    VecD2               bounds = cBounds(flatten(datas));
    fgout << fgnl << "Data bounds: " << bounds;
    if (bounds[1]-bounds[0] == 0)
        FGASSERT_FALSE1("data has no range");
    if (bounds[0] < 0)                                      // bar graph below assumes no negative values:
        for (Doubles & data : datas)
            data = mapAdd(data,bounds[0]);
    double              maxVal = bounds[1];
    for (size_t ss=0; ss<S; ++ss) {
        size_t              cIdx {0};
        for (size_t dd=0; dd<D; ++dd) {
            Doubles const &     data = datas[dd];
            if (ss < data.size()) {
                double              v = data[ss];
                uint                hgt = roundT<uint>((P-1) * v / maxVal);
                if (hgt > 0) {
                    Vec2UI              posIrcs {uint(xx),img.height()-hgt-1},
                                        sz {pixPerBar,hgt};
                    drawSolidRectangle_(img,posIrcs,sz,cColors6Rgba()[dd%6]);
                }
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

ImgRgba8            cLineGraph(MatD const & data,uint pixPerPnt,Strings const & labels)
{
    FGASSERT(!data.empty());
    FGASSERT(labels.empty() || (labels.size() == data.numRows()));
    VecD2               bounds = cBounds(data.m_data);
    double              range = bounds[1] - bounds[0];
    FGASSERT(range > 0);
    size_t              S = data.numCols(),     // sample series
                        M = data.numRows(),     // measurement type
                        P = S * pixPerPnt;
    ImgRgba8            img {P,P,Rgba8{0,0,0,255}};
    double              fac = (P-1) / range;
    for (size_t mm=0; mm<M; ++mm) {
        Arr3UC              c = mapCast<uchar>(cColors6()[mm%6]*255.0f);
        Rgba8               clr8 {c[0],c[1],c[2],255};
        Vec2UI              lastPnt {0};        // init to avoid warning
        for (size_t ss=0; ss<S; ++ss) {
            double              Y = (data.rc(mm,ss) - bounds[0]) * fac;
            Vec2UI              currPnt {
                scast<uint>(ss*pixPerPnt),
                scast<uint>(P - 1 - scast<size_t>(Y + 0.5))
            };
            if (ss > 0)
                drawLineIrcs(img,Vec2I{lastPnt},Vec2I{currPnt},clr8);
            lastPnt = currPnt;
        }
        if (!labels.empty())
            fgout << fgnl << labels[mm] << ": " << c;
    }
    return img;
}

void                cmdGraph(CLArgs const & args)
{
    Syntax                  syn{args,R"([-b <num>] [-g <num>] [-s] [-z] (<data>.txt)+ [<image>.jpg]
    Creates bar graph of given data sequence(s), display, and optionally save.
    -b      Pixel thickness of bars (default 6)
    -g      Pixel thickness of gap between bars (default 6)
    -s      <data>.txt is in text-serialized format (square bracket delimiters)
    -z      Shift the values so that the lowest value is zero
NOTES:
    * <data>.txt default format is space-separated numbers, newline-separated series.
        - text-serialized format delimits lists with square brackets, using any whitespace to delimit elements)"
    };
    String                  dataFname,
                            imgFname;
    uint                    barThick = 8,
                            gapThick = 8;
    bool                    shiftZero = false,
                            textSerial = false;
    while (syn.peekNext()[0] == '-') {
        if (syn.next() == "-b")
            barThick = syn.nextAs<uint>();
        else if (syn.curr() == "-g")
            gapThick = syn.nextAs<uint>();
        else if (syn.curr() == "-z")
            shiftZero = true;
        else if (syn.curr() == "-s")
            textSerial = true;
        else
            syn.error("Unrecognized option",syn.curr());
    }
    dataFname = syn.next();
    if (syn.more())
        imgFname = syn.next();
    syn.noMoreArgsExpected();
    Doubless                datas;
    if (textSerial)
        datas = dsrlzText<Doubless>(loadRawString(dataFname));
    else {
        Strings             lines = splitLines(loadRawString(dataFname));
        Doubles             data;
        for (String const & line : lines) {
            Opt<double>         opt = fromStr<double>(line);
            if (opt.has_value())
                data.push_back(opt.value());
        }
        if (!data.empty())
            datas.push_back(data);
    };
    if (datas.empty())
        syn.error("No data could be extracted from",dataFname);
    double                  minElem = cMinElem(flatten(datas));
    if (shiftZero)
        for (Doubles & data : datas)
            data = mapSub(data,minElem);
    viewImage(cBarGraph(datas,barThick,gapThick));
}

}
