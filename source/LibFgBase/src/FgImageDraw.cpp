//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
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
#include "FgSampler.hpp"

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
    FGASSERT(pixPerBar>0);
    size_t              D = datas.size(),                   // number of data series (may not all may be of same size)
                        S = cMaxElem(cSizes(datas)),        // max number of samples per series
                        P = S*(D*pixPerBar + pixSpacing),   // pixel width of image
                        xx {0};                             // image x coord counter (pixels)
    ImgRgba8            img {P,P,Rgba8{0,0,0,255}};         // make a square image
    Arr2D               bounds = cBounds(flatten(datas));
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

ImgRgba8            cGraphFunctions(Svec<Sfun<double(double)>> const & fns,Arr2D boundsX)
{
    size_t              S = fns.size();
    FGASSERT(S>0);
    uint constexpr      P = 1024;
    double              loX = boundsX[0],
                        szX = boundsX[1]-loX;
    FGASSERT(szX>0);
    Affine1D            ircsToX = Affine1D{szX/P,loX} * Affine1D{1,0.5};
    auto                fnGen = [&](Sfun<double(double)> const & fn)
    {
        return genSvec(P,[&,ircsToX](size_t pp){return fn(ircsToX*scast<double>(pp)); });
    };
    Doubless            data = mapCall(fns,fnGen);
    Arr2D               boundsY = nullBounds<double>();
    for (Doubles const & d : data)
        boundsY = updateBounds(d,boundsY);
    double              loY = boundsY[0],
                        szY = boundsY[1]-loY;
    Affine1D            pacsToY {-szY/P,boundsY[1]},
                        YtoPacs = pacsToY.inverse();
    // create image and put in axes if applicable:
    ImgRgba8            img {P,P,Rgba8{0,0,0,255}};
    double              zeroX = ircsToX.inverse() * 0.0,
                        zeroY = YtoPacs * 0.0;
    int                 zX = scast<int>(zeroX+0.5),
                        zY = scast<int>(zeroY);
    if ((zeroX>-0.5)&&(zeroX<P-0.5))    // vertical line
        drawLineIrcs(img,Vec2I{zX,0},Vec2I{zX,P-1},{127,127,127,255});
    if ((zeroY>0)&&(zeroY<P))
        drawLineIrcs(img,Vec2I{0,zY},Vec2I{P-1,zY},{127,127,127,255});
    for (size_t ss=0; ss<S; ++ss) {
        Arr3UC              c = mapCast<uchar>(cColors6()[ss%6]*255.0f);
        Rgba8               clr8 {c[0],c[1],c[2],255};
        for (uint pp=0; pp<P; ++pp) {
            double              x = ircsToX * scast<double>(pp),
                                y = fns[ss](x),
                                yPacs = YtoPacs * y;
            if ((yPacs>0)&&(yPacs<P)) {
                Vec2UI              ircs {pp,scast<uint>(yPacs)};
                img.paint(ircs,clr8);
            }
        }
    }
    return img;
}

Img3F             cScatterPlot(Arr2Ds const & data)
{
    Arr<Arr2D,2>        bnds = cBounds(data);
    Arr2D               dims = multAcc(bnds,Arr2D{-1,1}),
                        centres = multAcc(bnds,Arr2D{0.5,0.5}),
                        hszs = dims * 0.6,           // add boundary
                        los = centres - hszs;
    size_t constexpr    imgDim = 1024;
    AxAffine2D          pacsToSamp {Vec2D{hszs}*2/imgDim,Vec2D{los}},
                        sampToPacs = pacsToSamp.inverse();
    Vec2Ds              dataPacs = mapCall(data,[&](Arr2D d){return sampToPacs*Vec2D{d};});
    auto                fnSamp = [&](float const &,Vec2D pacs)
    {
        Arr3F               ret {1};
        for (Vec2D samp : dataPacs) {
            if (cMag(pacs-samp) < 4)
                return Arr3F{0};
        }
        return ret;
    };
    ImgF                grid{1,1};  // not used
    return sampleAdapt<Arr3F,float>(grid,imgDim,fnSamp,1,3,thread::hardware_concurrency());
}

ImgRgba8            cLineGraph(MatD const & data,uint pixPerPnt,Strings const & labels)
{
    FGASSERT(!data.empty());
    FGASSERT(labels.empty() || (labels.size() == data.numRows()));
    Arr2D               bounds = cBounds(data.m_data);
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
    -b      Pixel thickness of bars (default 3)
    -g      Pixel thickness of gap between bars (default 1)
    -s      <data>.txt is in text-serialized format (square bracket delimiters)
    -z      Shift the values so that the lowest value is zero
NOTES:
    * <data>.txt default format is space-separated numbers, newline-separated series.
        - text-serialized format delimits lists with square brackets, using any whitespace to delimit elements)"
    };
    uint                    barThick = 3,
                            gapThick = 1;
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
    Strings                 dataFnames {syn.next()};
    String                  imgFname;
    while (syn.more()) {
        Path                path {syn.next()};
        if (contains(getImgExts(),toLower(path.ext).m_str))
            imgFname = syn.curr();
        else
            dataFnames.push_back(syn.curr());
    }
    Doubless                datas;
    for (String const & dataFname : dataFnames) {
        if (textSerial)
            cat_(datas,dsrlzText<Doubless>(loadRawString(dataFname)));
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
    }
    if (datas.empty())
        syn.error("No data could be extracted");
    if (shiftZero) {
        double                  minElem = cMinElem(flatten(datas));
        for (Doubles & data : datas)
            data = mapSub(data,minElem);
    }
    viewImage(cBarGraph(datas,barThick,gapThick));
}

}
