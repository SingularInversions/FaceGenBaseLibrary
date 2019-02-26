//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:		Andrew Beatty
// Created:		April 6, 2010
//

#include "stdafx.h"

#include "FgSampler.hpp"
#include "FgOpt.hpp"
#include "FgBounds.hpp"
#include "FgRgba.hpp"
#include "FgSyntax.hpp"
#include "FgImgDisplay.hpp"
#include "FgTime.hpp"

using namespace std;

static
inline bool
valsDiffer(
    const FgRgbaF &                 centre,
    const FgMatrixC<FgRgbaF,2,2> &  corners,
    float                           maxDiff)
{
    return (
        (fgMaxElem(fgAbs(corners[0].m_c - centre.m_c)) > maxDiff) ||
        (fgMaxElem(fgAbs(corners[1].m_c - centre.m_c)) > maxDiff) ||
        (fgMaxElem(fgAbs(corners[2].m_c - centre.m_c)) > maxDiff) ||
        (fgMaxElem(fgAbs(corners[3].m_c - centre.m_c)) > maxDiff));
}

static uint64 rayCount;

static
FgRgbaF
sampleRecurse(
    FgFuncSample            sample,
    FgMat22F                bounds,
    FgMatrixC<FgRgbaF,2,2>  cornerVals,
    float                   maxDiff)
{
    FgVect2F        lc = bounds.colVec(0),
                    uc = bounds.colVec(1),
                    del = (uc-lc)*0.5f,
                    delx,dely;
    delx[0] = del[0];
    dely[1] = del[1];
    FgRgbaF         ret,
                    centre(sample(lc+delx+dely));
    if (valsDiffer(centre,cornerVals,maxDiff)) {
        rayCount+=4;
        FgMatrixC<FgRgbaF,3,3>  vals(
                cornerVals[0],
                sample(lc+delx),
                cornerVals[1],
                sample(lc+dely),
                centre,
                sample(uc-dely),
                cornerVals[2],
                sample(uc-delx),
                cornerVals[3]);
        FgRgbaF     acc;
        for (FgIter2UI it(2); it.valid(); it.next()) {
            FgVect2UI   coord = it();
            FgVect2F    lc2 = lc + FgVect2F(coord) * del[0];
            acc += 
                sampleRecurse(
                    sample,
                    fgJoinHoriz(lc2,lc2+del),
                    vals.subMatrix<2,2>(coord[1],coord[0]), // Matrices are (row,col) not (x,y)
                    maxDiff*2.0f);
        }
        ret = acc * 0.25f;
    }
    else
        ret = (cornerVals[0]+cornerVals[1]+cornerVals[2]+cornerVals[3]) * 0.125f + centre * 0.5f;
    return ret;
}

FgImgRgbaF
fgSamplerF(
    FgVect2UI           dims,
    FgFuncSample        sample,
    uint                antiAliasBitDepth)
{
    FgImgRgbaF          img(dims);
    FGASSERT(dims.cmpntsProduct() > 0);
    FGASSERT((antiAliasBitDepth > 0) && (antiAliasBitDepth <= 16));
    rayCount = (img.width()+1) * (img.height()+1);
    float               widf = float(img.width()),
                        hgtf = float(img.height());
    FgImgRgbaF          sampleLines(img.width()+1,2);
    for (uint col=0; col<sampleLines.width(); ++col)
        sampleLines.xy(col,0) = 
            sample(FgVect2F(float(col)/widf,0.0f));
    for (uint row=0; row<img.height(); ++row) {
        uint            fbit = row%2,
                        sbit = 1-fbit;
        for (uint col=0; col<sampleLines.width(); ++col)
            sampleLines.xy(col,sbit) = 
                sample(FgVect2F(float(col)/widf,float(row+1)/hgtf));
        for (uint col=0; col<img.width(); ++col) {
            img.xy(col,row) =
                sampleRecurse(
                    sample,
                    FgMat22F(
                        float(col)/widf,
                        float(col+1)/widf,
                        float(row)/hgtf,
                        float(row+1)/hgtf),
                    FgMatrixC<FgRgbaF,2,2>(
                        sampleLines.xy(col,fbit),
                        sampleLines.xy(col+1,fbit),
                        sampleLines.xy(col,sbit),
                        sampleLines.xy(col+1,sbit)),
                    float(1 << (9-antiAliasBitDepth)));
        }
    }
    //fgout << "Raycast count: " << rayCount;
    return img;
}

FgImgRgbaUb
fgSampler(
    FgVect2UI           dims,
    FgFuncSample        sample,
    uint                antiAliasBitDepth)
{
    FgImgRgbaUb         img(dims);
    FGASSERT((antiAliasBitDepth > 0) && (antiAliasBitDepth <= 8));
    FgImgRgbaF          fimg = fgSamplerF(img.dims(),sample,antiAliasBitDepth);
    for (FgIter2UI it(img.dims()); it.valid(); it.next())
    {
        const FgRgbaF & fpix = fimg[it()];
        img[it()] = 
            FgRgbaUB(
                uchar(fgClip(fpix.red(),0.0f,255.0f)),
                uchar(fgClip(fpix.green(),0.0f,255.0f)),
                uchar(fgClip(fpix.blue(),0.0f,255.0f)),
                uchar(fgClip(fpix.alpha(),0.0f,255.0f)));
    }
    return img;
}

static
FgRgbaF
halfMoon(FgVect2F ics)
{
    if ((ics[1] > 0.5f) && ((ics-FgVect2F(0.5f,0.5f)).length() < 0.3f))
        return FgRgbaF(255.0f,255.0f,255.0f,255.0f);
    else
        return FgRgbaF(0.0f,0.0f,0.0f,255.0f);
}

void
fgSamplerTest(const FgArgs &)
{
    fgImgDisplay(fgSampler(FgVect2UI(128),halfMoon,4));
}

static
FgRgbaF
mandelbrot(FgVect2F ics)
{
    double      zr = 0.0,
                zc = 0.0,
                cr = ics[0]*2.5 - 1.6,
                cc = ics[1]*2.5 - 1.25;
    uint        ii=0;
    for (; ii<255; ++ii) {
        double  tmp1 = zr*zr - zc*zc;
        if (tmp1 > 4)
            break;
        double  tmp2 = tmp1 + cr;
        zc = 2.0*zr*zc + cc;
        zr = tmp2;
    }
    return FgRgbaF(float(ii));
}

void
fgSamplerMLTest(const FgArgs &)
{
    FgTimer         time;
    FgImgRgbaUb     img = fgSampler(FgVect2UI(1024),mandelbrot,3);
    fgout << "Time: " << time.read() << "s";
    fgImgDisplay(img);
}

// */
