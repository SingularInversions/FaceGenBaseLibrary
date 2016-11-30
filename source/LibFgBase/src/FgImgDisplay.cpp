//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     May 6, 2011
//

#include "stdafx.h"

#include "FgImgDisplay.hpp"
#include "FgGuiApiImage.hpp"
#include "FgFileSystem.hpp"
#include "FgAffine1.hpp"
#include "FgAffineCwPreC.hpp"
#include "FgBounds.hpp"
#include "FgCommand.hpp"

using namespace std;

void
fgImgDisplay(const FgImgRgbaUb & img,vector<FgVect2F> pts)
{
    FgString            store = fgDirUserAppDataLocalFaceGen("SDK","DisplayImage");
    g_gg = FgGuiGraph(store);
    fgGuiImplStart(
        FgString("FaceGen SDK DisplayImage"),
        fgGuiImage(g_gg.addNode(img),g_gg.addNode(pts)),
        store);
}

void
fgImgDisplay(const FgImgUC &img)
{
    FgImgRgbaUb        dispImg;
    fgImgConvert(img,dispImg);
    fgImgDisplay(dispImg);
}

void
fgImgDisplay(const FgImage<ushort> & img)
{
    FgAffine1F      aff(FgVectF2(fgBounds(img.dataVec())),FgVectF2(0,255));
    FgImgRgbaUb     di(img.dims());
    for (size_t ii=0; ii<img.numPixels(); ++ii)
        di[ii] = FgRgbaUB(aff * img[ii]);
    fgImgDisplay(di);
}

void
fgImgDisplay(const FgImgF & img)
{
    FgAffine1F          aff(fgBounds(img.m_data),FgVectF2(0,255));
    FgImgRgbaUb         dispImg(img.dims());
    for (size_t ii=0; ii<img.m_data.size(); ++ii)
        dispImg.m_data[ii] = FgRgbaUB(uchar(aff * img.m_data[ii]));
    fgImgDisplay(dispImg);
}

void
fgImgDisplay(const FgImgD & img)
{
    FgAffine1D          aff(fgBounds(img.m_data),FgVectD2(0,255));
    FgImgRgbaUb         dispImg(img.dims());
    for (size_t ii=0; ii<img.m_data.size(); ++ii)
        dispImg.m_data[ii] = FgRgbaUB(uchar(aff * img.m_data[ii]));
    fgImgDisplay(dispImg);
}

void
fgImgDisplay(const FgImg3F & img)
{
    FgVectF2            bounds = fgBounds(fgBounds(img.dataVec()));
    FgAffineCwPre3F     xform(FgVect3F(-bounds[0]),FgVect3F(255.0f/(bounds[1]-bounds[0])));
    FgImg3F             tmp = FgImg3F(img.dims(),fgTransform(img.dataVec(),xform));
    FgImgRgbaUb         disp(tmp.dims());
    for (size_t ii=0; ii<disp.numPixels(); ++ii) {
        FgVect3UC       clr;       
        fgRound_(tmp[ii],clr);
        disp[ii] = FgRgbaUB(clr[0],clr[1],clr[2],uchar(255));
    }
    fgout << fgnl << disp;
    fgImgDisplay(disp);
}

void
fgImgDisplayColorize(const FgImgD & img)
{
    FgVectD2        ib = fgBounds(img.dataVec());
    FgAffine1D aff(ib,FgVectD2(0,4));
    FgImgRgbaUb     di(img.dims());
    for (FgIter2UI it(di.dims()); it.valid(); it.next()) {
        double      sc = aff * img[it()];
        FgRgbaUB    col(0,0,0,255);
        if(sc < 1.0) {
            col.blue() = uchar((1-sc) * 255);
            col.green() = uchar(sc * 255); }
        else if (sc < 2.0) {
            sc -= 1.0;
            col.green() = uchar((1-sc) * 255);
            col.red() = uchar(sc * 255); }
        else if (sc < 3.0) {
            sc -= 2.0;
            col.red() = uchar(255);
            col.blue() = uchar(sc * 255); }
        else {
            sc -= 3.0;
            col.red() = uchar(255);
            col.blue() = uchar(255);
            col.green() = uchar(sc * 255); }
        di[it()] = col;
    }
    fgout << fgnl
        << "Colour scheme blue-green-red-purple-white over bounds: ["
        << ib[0] << "," << ib[1] << "]";
    fgImgDisplay(di);
}

void
fgImgDisplay(const FgImgRgbaF & img)
{
    FgImgRgbaUb     di(img.dims());
    for (size_t ii=0; ii<di.numPixels(); ++ii)
        di[ii] = FgRgbaUB(img[ii]*255.0f + FgRgbaF(0.5f));
    fgImgDisplay(di);
}

void
fgImgGuiTestm(const FgArgs &)
{
    fgImgDisplay(fgLoadImgAnyFormat(fgDataDir()+"base/trees.jpg"));
}
