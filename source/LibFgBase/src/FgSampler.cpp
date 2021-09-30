//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgSampler.hpp"
#include "FgOpt.hpp"
#include "FgBounds.hpp"
#include "FgRgba.hpp"
#include "FgSyntax.hpp"
#include "FgImgDisplay.hpp"
#include "FgTime.hpp"
#include "FgCommand.hpp"

using namespace std;

namespace Fg {

namespace {

bool
valsDiffer(
    RgbaF const &               centre,
    Mat<RgbaF,2,2> const &      corners,
    float                       maxDiff)
{
    // Tried cMag(corner-centre) > sqr(maxDiff)*4 but it was actually SLOWER !
    return (
        (cMax(mapAbs(corners[0].m_c - centre.m_c)) > maxDiff) ||
        (cMax(mapAbs(corners[1].m_c - centre.m_c)) > maxDiff) ||
        (cMax(mapAbs(corners[2].m_c - centre.m_c)) > maxDiff) ||
        (cMax(mapAbs(corners[3].m_c - centre.m_c)) > maxDiff));
}

RgbaF
sampleRecurse(
    SampleFn const &        sampleFn,
    Vec2F                   lc,             // Lower corner sample point
    Vec2F                   uc,             // Upper corner sample point (IUCS so not square)
    Mat<RgbaF,2,2>          bs,             // Bounds samples. No speedup at all from making this const ref
    float                   maxDiff,
    // Invalid if first channel is floatMax(), othwerise gives sl/st from previous call.
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
                    sst {floatMax()},
                    ssl {floatMax()},
                    ssr {floatMax()},
                    ssb {floatMax()};
    if (valsDiffer(sc,bs,maxDiff)) {
        float           md2 = maxDiff * 2.0f;
        RgbaF           sl = (slo[0] == floatMax()) ? sampleFn(centre-hdelx) : slo,
                        sr = sampleFn(centre+hdelx),
                        st = (sto[0] == floatMax()) ? sampleFn(centre-hdely) : sto,
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
        slo[0] = floatMax();    // These invalidations are only needed at the top level, not in recursed calls
        sto[0] = floatMax();
        return (bs[0]+bs[1]+bs[2]+bs[3]) * 0.125f + sc * 0.5f;
    }
}

}

ImgC4F
sampleAdaptiveF(
    Vec2UI              dims,
    SampleFn const &    sampleFn,
    float               channelBound,
    uint                antiAliasBitDepth)
{
    FGASSERT(dims.cmpntsProduct() > 0);
    FGASSERT(sampleFn);
    FGASSERT((antiAliasBitDepth > 0) && (antiAliasBitDepth <= 16));
    ImgC4F          img {dims};
    float           maxDiff = channelBound / float(1 << antiAliasBitDepth);
    float           widf = float(img.width()),
                    hgtf = float(img.height());
    ImgC4F          sampleLines {img.width()+1,2};
    for (uint xx=0; xx<sampleLines.width(); ++xx)
        sampleLines.xy(xx,0) = sampleFn(Vec2F(xx/widf,0));
    RgbaFs          ssts(img.width(),RgbaF{floatMax()});
    for (uint yy=0; yy<img.height(); ++yy) {
        float           yyf0 = yy/hgtf,
                        yyf1 = (yy+1)/hgtf;
        uint            fbit = yy%2,
                        sbit = 1-fbit;
        for (uint xx=0; xx<sampleLines.width(); ++xx)
            sampleLines.xy(xx,sbit) = sampleFn(Vec2F{xx/widf,yyf1});
        RgbaF           ssl {floatMax()};
        for (uint xx=0; xx<img.width(); ++xx) {
            Vec2F           lc {xx/widf,yyf0},
                            uc {(xx+1)/widf,yyf1};
            Mat<RgbaF,2,2>  bs {
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

ImgRgba8
sampleAdaptive(
    Vec2UI              dims,
    SampleFn const &    sampleFn,
    uint                antiAliasBitDepth)
{
    ImgRgba8             img {dims};
    FGASSERT((antiAliasBitDepth > 0) && (antiAliasBitDepth <= 8));
    ImgC4F              fimg = sampleAdaptiveF(img.dims(),sampleFn,256.0f,antiAliasBitDepth);
    for (size_t ii=0; ii<fimg.numPixels(); ++ii) {
        RgbaF const &       in = fimg[ii];
        RgbaUC &            out = img[ii];
        for (uint jj=0; jj<4; ++jj)
            out[jj] = uchar(clamp(in[jj],0.0f,255.0f));
    }
    return img;
}

namespace {

RgbaF
halfMoon(Vec2F ics)
{
    if ((ics[1] > 0.5f) && ((ics-Vec2F(0.5f,0.5f)).len() < 0.3f))
        return RgbaF(255.0f,255.0f,255.0f,255.0f);
    else
        return RgbaF(0.0f,0.0f,0.0f,255.0f);
}

void
testmHalfMoon(CLArgs const &)
{
    viewImage(sampleAdaptive(Vec2UI(128),halfMoon,4));
}

RgbaF
mandelbrot(Vec2F ics)
{
    double              zr = 0.0,
                        zc = 0.0,
                        cr = ics[0]*2.5 - 1.6,
                        cc = ics[1]*2.5 - 1.25;
    uint                ii=0;
    for (; ii<255; ++ii) {
        double              tmp1 = zr*zr - zc*zc;
        if (tmp1 > 4)
            break;
        double              tmp2 = tmp1 + cr;
        zc = 2.0*zr*zc + cc;
        zr = tmp2;
    }
    return RgbaF(float(ii));
}

void
testmMandelbrot(CLArgs const &)
{
    Timer               time;
    ImgRgba8             img = sampleAdaptive(Vec2UI(1024),mandelbrot,3);
    fgout << "Time: " << time.elapsedSeconds() << "s";
    viewImage(img);
}

}

void
testmSampler(CLArgs const & args)
{
    Cmds                cmds {
        {testmHalfMoon,"hm","half moon"},
        {testmMandelbrot,"mandel","Mandelbrot set"},
    };
    doMenu(args,cmds);
}

}

// */
