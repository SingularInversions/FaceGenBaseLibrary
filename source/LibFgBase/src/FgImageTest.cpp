//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Sept. 15, 2009
//

#include "stdafx.h"

#include "FgImage.hpp"
#include "FgTime.hpp"
#include "FgImgDisplay.hpp"
#include "FgImage.hpp"
#include "FgTestUtils.hpp"
#include "FgApproxEqual.hpp"
#include "FgCommand.hpp"

using namespace std;

// MANUAL:

static
void
display(const FgArgs &)
{
    FgString    dd = fgDataDir();
    string      testorig_jpg("base/test/testorig.jpg");
    FgImgRgbaUb img;
    fgImgLoadJfif(dd+testorig_jpg,img);
    fgImgDisplay(img);
    string      testorig_bmp("base/test/testorig.bmp");
    fgLoadImgAnyFormat(dd+testorig_bmp,img);
    fgImgDisplay(img);
}

static
void
resize(const FgArgs &)
{
    string              fname("base/test/testorig.jpg");
    FgImgRgbaUb         img;
    fgLoadImgAnyFormat(fgDataDir()+fname,img);
    FgImgRgbaUb         out(img.width()/2+1,img.height()+1);
    fgImgResize(img,out);
    fgImgDisplay(out);
}

static
void
sfs(const FgArgs &)
{
    FgImg4UC            orig = fgLoadImg4UC(fgDataDir()+"base/lenna512.png");
    FgImage<FgVect3F>   img(orig.dims());
    for (size_t ii=0; ii<img.numPixels(); ++ii)
        img[ii] = FgVect3F(orig[ii].subMatrix<3,1>(0,0));
    FgTimer             time;
    for (uint ii=0; ii<100; ++ii)
        fgSmoothFloat(img,img,1);
    double              ms = time.read();
    fgout << fgnl << "smoothFloat time: " << ms;
    fgImgDisplay(img);
}

void
fgImageTestm(const FgArgs & args)
{
    vector<FgCmd>   cmds;
    cmds.push_back(FgCmd(resize,"resize"));
    cmds.push_back(FgCmd(display,"display"));
    cmds.push_back(FgCmd(sfs,"sfs","smoothFloat speed"));
    fgMenu(args,cmds);
}

// AUTOMATIC:

static
void
testConvolve(const FgArgs &)
{
    fgRandSeedRepeatable();
    FgImgF          tst(16,16);
    for (size_t ii=0; ii<tst.numPixels(); ++ii)
        tst[ii] = float(fgRand());
    FgImgF          i0,i1;
    fgSmoothFloat(tst,i0,1);
    fgConvolveFloat(tst,FgMat33F(1,2,1,2,4,2,1,2,1)/16.0f,i1,1);
    //fgout << fgnl << i0.m_data << fgnl << i1.m_data;
    FGASSERT(fgApproxEqual(i0.m_data,i1.m_data));
}

void    fgImgTestWrite(const FgArgs &);

void
fgImageTest(const FgArgs & args)
{
    vector<FgCmd>       cmds;
    cmds.push_back(FgCmd(testConvolve,"conv"));
    cmds.push_back(FgCmd(fgImgTestWrite,"write"));
    fgMenu(args,cmds,true,false,true);
}
