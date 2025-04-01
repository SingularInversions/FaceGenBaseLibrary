//
// Copyright (c) 2025 Singular Inversions Inc.
//
// Authors:     Andrew Beatty
// Created:     March 21, 2024
//

#ifndef FGSAMPLER_HPP
#define FGSAMPLER_HPP

#include "FgImage.hpp"
#include "FgArray.hpp"

namespace Fg {

template<class T,class U>
using SampFn = Sfun<T(U const & bin,Vec2D pacs)>;

template<class T,class U>
T                   sampleAdaptR(   // helper function for 'sampleAdapt' below
    U const &           bin,
    SampFn<T,U> const & fn,
    Vec2D               lo,         // low corner sample point
    double              sz,         // (square) region size
    T                   sll,        // sample lo Y lo X, etc:
    T                   slh,
    T                   shl,
    T                   shh,
    double              maxDiff)
{
    typedef typename Traits<T>::Scalar S;
    static_assert(std::is_floating_point_v<S>);
    S                   md = scast<S>(maxDiff);
    double              hsz = sz/2;
    Vec2D               cc = mapAdd(lo,hsz);            // Y centre, X centre
    T                   scc = fn(bin,cc);               // sample at cc
    if ((isApproxEqual(scc,sll,md)) &&
        (isApproxEqual(scc,slh,md)) &&
        (isApproxEqual(scc,shl,md)) &&
        (isApproxEqual(scc,shh,md)))
    {
        return scc * scast<S>(0.5) + (sll+slh+shl+shh) * scast<S>(0.125);
    }
    double              md2 = maxDiff*2;
    Vec2D               lc {lo[0]+hsz,lo[1]},                   // lo Y, centre X, etc:
                        cl {lo[0],lo[1]+hsz},
                        ch {lo[0]+sz,lo[1]+hsz},
                        hc {lo[0]+hsz,lo[1]+sz};
    T                   slc = fn(bin,lc),                       // sample in above order
                        scl = fn(bin,cl),
                        sch = fn(bin,ch),
                        shc = fn(bin,hc),
                        rll = sampleAdaptR(bin,fn,lo,hsz,sll,slc,scl,scc,md2),    // recurse lo Y, lo X, etc:
                        rlh = sampleAdaptR(bin,fn,lc,hsz,slc,slh,scc,sch,md2),
                        rhl = sampleAdaptR(bin,fn,cl,hsz,scl,scc,shl,shc,md2),
                        rhh = sampleAdaptR(bin,fn,cc,hsz,scc,sch,shc,shh,md2);
    return (rll+rlh+rhl+rhh) * scast<S>(0.25);
}

// Adaptive anti-alias sampler using quincunx pattern.
// Explicit use of templated grid index an integer factor smaller than the image allows recursion using
// a single grid bin.
template<class T,class U>
Img<T>              sampleAdapt(
    Img<U> const &          grid,           // grid index
    uint                    pixPerBin,      // linear measure; typically 4 (ie 4x4=16 pixels per bin)
    SampFn<T,U> const &     sampleFn,
    double                  channelBound,
    uint                    antiAliasBitDepth,
    size_t                  maxThreads=0)
{
    FGASSERT(!grid.empty());
    FGASSERT(pixPerBin > 0);
    FGASSERT(sampleFn);
    FGASSERT((antiAliasBitDepth > 0) && (antiAliasBitDepth < 20));
    size_t              GX = grid.width(),
                        GY = grid.height(),
                        IX = GX * pixPerBin,
                        IY = GY * pixPerBin;
    Img<T>              img {IX,IY};
    double              maxDiff = channelBound / scast<double>(1ULL << antiAliasBitDepth);
    Img<T>              corners {IX+1,IY+1};
    ThreadDispatcher    td {maxThreads};
    for (size_t yy=0; yy<IY+1; ++yy) {
        auto                cLine = [&grid,&sampleFn,&corners,IX,GX,GY,pixPerBin,yy]()
        {
            size_t              gy = cMin(yy/pixPerBin,GY-1);
            for (size_t xx=0; xx<IX+1; ++xx) {
                size_t              gx = cMin(xx/pixPerBin,GX-1);      // last sample rounds to lower bin
                corners.xy(xx,yy) = sampleFn(grid.xy(gx,gy),Vec2D(xx,yy));
            }
        };
        td.dispatch(cLine);
    }
    td.finish();
    for (size_t yy=0; yy<IY; ++yy) {
        T const         *cornerPtr0 = corners.rowPtr(yy),
                        *cornerPtr1 = corners.rowPtr(yy+1);
        auto            fn = [&grid,&sampleFn,&img,pixPerBin,IX,maxDiff,yy,cornerPtr0,cornerPtr1]()
        {
            for (size_t xx=0; xx<IX; ++xx) {
                img.xy(xx,yy) = sampleAdaptR(
                    grid.xy(xx/pixPerBin,yy/pixPerBin),
                    sampleFn,Vec2D(xx,yy),1,
                    cornerPtr0[xx],cornerPtr0[xx+1],
                    cornerPtr1[xx],cornerPtr1[xx+1],
                    maxDiff);
            }
        };
        td.dispatch(fn);
    }
    return img;
}

}

#endif
