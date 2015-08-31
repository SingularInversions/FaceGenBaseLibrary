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
#include "FgValidVal.hpp"
#include "FgBounds.hpp"
#include "FgRgba.hpp"

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
    const FgSample &        func,
    FgMat22F             bounds,
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
                    centre(func(lc+delx+dely));
    if (valsDiffer(centre,cornerVals,maxDiff))
    {
        rayCount+=4;
        FgMatrixC<FgRgbaF,3,3>
            vals(
                cornerVals[0],
                func(lc+delx),
                cornerVals[1],
                func(lc+dely),
                centre,
                func(uc-dely),
                cornerVals[2],
                func(uc-delx),
                cornerVals[3]);
        FgRgbaF     acc;
        for (FgIter2UI it(2); it.valid(); it.next())
        {
            FgVect2UI   coord = it();
            FgVect2F    lc2 = lc + FgVect2F(coord) * del[0];
            acc += 
                sampleRecurse(
                    func,
                    fgConcatHoriz(lc2,lc2+del),
                    vals.subMatrix<2,2>(coord[1],coord[0]), // Matrices are (row,col) not (x,y)
                    maxDiff*2.0f);
        }
        ret = acc * 0.25f;
    }
    else
        ret = (cornerVals[0]+cornerVals[1]+cornerVals[2]+cornerVals[3]) * 0.125f +
              centre * 0.5f;

    return ret;
}

void
fgSampler(
    const FgSample &    func,
    FgImgRgbaF &        img,
    uint                antiAliasBitDepth)
{
    FGASSERT((img.width() > 1) && (img.height() > 1));
    FGASSERT((antiAliasBitDepth > 0) && (antiAliasBitDepth <= 8));

    rayCount = (img.width()+1) * (img.height()+1);
    float               widf = float(img.width()),
                        hgtf = float(img.height());
    FgImgRgbaF          sampleLines(img.width()+1,2);
    for (uint col=0; col<sampleLines.width(); ++col)
        sampleLines.elem(col,0) = 
            func(FgVect2F(float(col)/widf,0.0f));
    for (uint row=0; row<img.height(); ++row)
    {
        uint            fbit = row%2,
                        sbit = 1-fbit;
        for (uint col=0; col<sampleLines.width(); ++col)
            sampleLines.elem(col,sbit) = 
                func(FgVect2F(float(col)/widf,float(row+1)/hgtf));
        for (uint col=0; col<img.width(); ++col)
        {
            img.elem(col,row) =
                sampleRecurse(
                    func,
                    FgMat22F(
                        float(col)/widf,
                        float(col+1)/widf,
                        float(row)/hgtf,
                        float(row+1)/hgtf),
                    FgMatrixC<FgRgbaF,2,2>(
                        sampleLines.elem(col,fbit),
                        sampleLines.elem(col+1,fbit),
                        sampleLines.elem(col,sbit),
                        sampleLines.elem(col+1,sbit)),
                    float(1 << (9-antiAliasBitDepth)));
        }
    }
    //fgout << "Raycast count: " << rayCount;
}

void
fgSampler(
    const FgSample &    sample,
    FgImgRgbaUb &       img,
    uint                antiAliasBitDepth)
{
    FgImgRgbaF                  fimg(img.dims());
    fgSampler(sample,fimg,antiAliasBitDepth);
    for (FgIter2UI it(img.dims()); it.valid(); it.next())
    {
        const FgRgbaF & fpix = fimg[it()];
        img[it()] = 
            FgRgbaUB(
                uchar(fgClamp(fpix.red(),0.0f,255.0f)),
                uchar(fgClamp(fpix.green(),0.0f,255.0f)),
                uchar(fgClamp(fpix.blue(),0.0f,255.0f)),
                uchar(fgClamp(fpix.alpha(),0.0f,255.0f)));
    }
}

// */
