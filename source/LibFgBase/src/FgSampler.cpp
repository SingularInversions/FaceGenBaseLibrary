//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgSampler.hpp"
#include "FgSerial.hpp"
#include "FgBounds.hpp"
#include "FgRgba.hpp"

#include "FgImgDisplay.hpp"
#include "FgTime.hpp"
#include "FgCommand.hpp"

using namespace std;

namespace Fg {

namespace {

bool                valsDiffer(
    RgbaF const &               centre,
    Mat<RgbaF,2,2> const &      corners,
    float                       maxDiff)
{
    // Tried cMag(corner-centre) > sqr(maxDiff)*2 but it yielded worse discontinuities (and *4 much worse)
    return (
        (cMax(mapAbs(corners[0].m_c - centre.m_c)) > maxDiff) ||
        (cMax(mapAbs(corners[1].m_c - centre.m_c)) > maxDiff) ||
        (cMax(mapAbs(corners[2].m_c - centre.m_c)) > maxDiff) ||
        (cMax(mapAbs(corners[3].m_c - centre.m_c)) > maxDiff));
}

RgbaF               sampleRecurse(
    SampleFn const &        sampleFn,
    Vec2F                   lc,             // Lower corner sample point
    Vec2F                   uc,             // Upper corner sample point (IUCS so not square)
    Mat<RgbaF,2,2>          bs,             // Bounds samples. No speedup at all from making this const ref
    float                   maxDiff,
    // Invalid if first channel is lims<float>::max(), othwerise gives sl/st from previous call.
    // Returned values must be sr/sb resp. for use in raster order next calls.
    // This optimization only saved 5% on time (granted for very simple raycaster)
    // so likely not worth saving values across deeper recursion boundaries.
    RgbaF &                 slo,
    RgbaF &                 sto)
{
    Vec2F           del = uc-lc,
                    hdel = del*0.5f,
                    centre = lc + hdel,
                    hdelx {hdel[0],0.0f},
                    hdely {0.0f,hdel[1]};
    RgbaF           sc {sampleFn(centre)},
                    sst {lims<float>::max()},
                    ssl {lims<float>::max()},
                    ssr {lims<float>::max()},
                    ssb {lims<float>::max()};
    if (valsDiffer(sc,bs,maxDiff)) {
        float           md2 = maxDiff * 2.0f;
        RgbaF           sl = (slo[0] == lims<float>::max()) ? sampleFn(centre-hdelx) : slo,
                        sr = sampleFn(centre+hdelx),
                        st = (sto[0] == lims<float>::max()) ? sampleFn(centre-hdely) : sto,
                        sb = sampleFn(centre+hdely),
                        stl = sampleRecurse(sampleFn,lc,centre,{bs[0],st,sl,sc},md2,sst,ssl),
                        str = sampleRecurse(sampleFn,lc+hdelx,centre+hdelx,{st,bs[1],sc,sr},md2,sst,ssr),
                        sbl = sampleRecurse(sampleFn,lc+hdely,centre+hdely,{sl,sc,bs[2],sb},md2,ssb,ssl),
                        sbr = sampleRecurse(sampleFn,centre,uc,{sc,sr,sb,bs[3]},md2,ssb,ssr);
        slo = sr;
        sto = sb;
        return (stl+str+sbl+sbr) * 0.25f;
    }
    else {
        slo[0] = lims<float>::max();    // These invalidations are only needed at the top level, not in recursed calls
        sto[0] = lims<float>::max();
        return (bs[0]+bs[1]+bs[2]+bs[3]) * 0.125f + sc * 0.5f;
    }
}

}

ImgC4F              sampleAdaptiveF(
    Vec2UI              dims,
    SampleFn const &    sampleFn,
    float               channelBound,
    uint                antiAliasBitDepth)
{
    FGASSERT(dims.cmpntsProduct() > 0);
    FGASSERT(sampleFn);
    FGASSERT((antiAliasBitDepth > 0) && (antiAliasBitDepth <= 16));
    ImgC4F              img {dims};
    float               maxDiff = channelBound / float(1 << antiAliasBitDepth);
    ImgC4F              sampleLines {img.width()+1,2};
    for (uint xx=0; xx<sampleLines.width(); ++xx)
        sampleLines.xy(xx,0) = sampleFn(Vec2F(xx,0));
    RgbaFs              ssts (img.width(),RgbaF{lims<float>::max()});
    for (uint yy=0; yy<img.height(); ++yy) {
        float               yyf0 = yy,
                            yyf1 = yy+1;
        uint                fbit = yy%2,
                            sbit = 1-fbit;
        for (uint xx=0; xx<sampleLines.width(); ++xx)
            sampleLines.xy(xx,sbit) = sampleFn(Vec2F(xx,yyf1));
        RgbaF               ssl {lims<float>::max()};
        for (uint xx=0; xx<img.width(); ++xx) {
            Vec2F               lc (xx,yyf0),
                                uc (xx+1,yyf1);
            Mat<RgbaF,2,2>      bs {
                sampleLines.xy(xx,fbit),
                sampleLines.xy(xx+1,fbit),
                sampleLines.xy(xx,sbit),
                sampleLines.xy(xx+1,sbit)
            };
            img.xy(xx,yy) = sampleRecurse(sampleFn,lc,uc,bs,maxDiff,ssl,ssts[xx]);
        }
    }
    return img;
}

namespace {

void                testHalfMoon(CLArgs const & args)
{
    size_t const        dim = 128,
                        hd = dim/2,
                        td = dim/3;
    auto                sampFn = [=](Vec2F ipcs)
    {
        if ((ipcs[1] > hd) && ((ipcs-Vec2F(hd,hd)).len() < td))
            return RgbaF{1,1,1,1};
        else
            return RgbaF{0,0,0,1};
    };
    ImgRgba8            img = toRgba8(mapGamma(sampleAdaptiveF(Vec2UI{dim},sampFn,1,4),0.5));
    if (!isAutomated(args))
        viewImage(img);
}

void                testMandelbrot(CLArgs const & args)
{
    size_t const        dim = 1024;
    auto                sampFn = [](Vec2F ipcs)
    {
        complex<double>     z {0,0},
                            c {(ipcs[0]-750.0)/512.0,(ipcs[1]-512.0)/512.0};
        size_t              ii {0};
        for (; ii<255; ++ii) {
            z = z*z + c;
            if (cMag(z) > 4)            // guaranteed to diverge
                break;
        }
        float               v = scast<float>(ii) / 255.0f;
        return RgbaF(v,v,v,1);
    };
    Timer               time;
    ImgRgba8            img = toRgba8(mapGamma(sampleAdaptiveF(Vec2UI{dim},sampFn,1,4),0.5));
    fgout << "time: " << time.elapsedSeconds() << "s";
    if (!isAutomated(args))
        viewImage(img);
}

}

void                testSampler(CLArgs const & args)
{
    Cmds                cmds {
        {testMandelbrot,"mand","Mandelbrot set"},
        {testHalfMoon,"moon","half moon"},
    };
    doMenu(args,cmds,true);
}

}

// */
