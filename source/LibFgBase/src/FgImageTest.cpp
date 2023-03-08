//
// Copyright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgImage.hpp"
#include "FgTime.hpp"
#include "FgImgDisplay.hpp"
#include "FgImage.hpp"
#include "FgTestUtils.hpp"
#include "FgApproxEqual.hpp"
#include "FgCommand.hpp"
#include "FgSyntax.hpp"

using namespace std;

namespace Fg {

namespace {

void                testmResize(CLArgs const &)
{
    string          fname("base/test/testorig.jpg");
    ImgRgba8         img = loadImage(dataDir()+fname);
    ImgRgba8         out(img.width()/2+1,img.height()+1);
    imgResize(img,out);
    viewImage(out);
}

void                resamp(CLArgs const &)
{
    Img3F                   in = toUnit3F(loadImage(dataDir()+"base/Mandrill512.png"));
    Vec2D                   lo {94.3,37.8};
    for (uint lnSz = 5; lnSz < 10; ++lnSz) {
        uint                    sz = 2 << lnSz;
        Img3F                   out = resampleAdaptive(in,lo,309.7f,sz);
        viewImage(toRgba8(out));
    }
}

void                testmSfs(CLArgs const &)
{
    Img3F               img = toUnit3F(loadImage(dataDir()+"base/Mandrill512.png"));
    {
        PushTimer           timer {"floating point image smooth"};
        for (uint ii=0; ii<64; ++ii)
            smoothFloat_(img,img,1);
    }
    viewImage(toRgba8(img));
}

void                testBlerp(CLArgs const &)
{
    {
        ImgF            empty;
        Blerp           blerp {{0,0},empty.dims()};
        FGASSERT(!blerp.valid());
    }
    {
        ImgF            two {2,2,{
            1.0f,   2.0f,
            4.0f,   8.0f
        }};
        auto            fn = [&two](Vec2F ircs,float resultZero,float resultClamp)
        {
            {
                Blerp           blerp {ircs,two.dims()};
                float           val = blerp.sample(two).wval;
                FGASSERT(val == resultZero);
            }
            {
                BlerpClamp      blerp {ircs,two.dims()};
                float           val = blerp.sample(two);
                FGASSERT(val == resultClamp);
            }
        };
        fn({-1,-1},     0,1);
        fn({-1,0},      0,1);
        fn({0,-1},      0,1);
        fn({-0.5,0},    0.5,1);
        fn({0,-0.5},    0.5,1);
        fn({0,0},       1,1);
        fn({0.5,0},     1.5f,1.5f);
        fn({0.25,0},    1.25f,1.25f);
        fn({0.75,0.25}, 3.0625f,3.0625f);
        fn({2,0},       0,2);
        fn({0,2},       0,4);
        fn({2,2},       0,8);
    }
}

void                testComposite(CLArgs const &)
{
    String8             dd = dataDir();
    ImgRgba8            overlay = loadImage(dd+"base/Teeth512.png"),
                        base = loadImage(dd+"base/Mandrill512.png");
    testRegressExact(composite(overlay,base),dd+"base/test/imgops/composite.png");
}

void                testDecodeJpeg(CLArgs const &)
{
    String8             imgFile = dataDir() + "base/trees.jpg";
    String              blob = loadRawString(imgFile);
    Uchars              ub; ub.reserve(blob.size());
    for (char ch : blob)
        ub.push_back(scast<uchar>(ch));
    ImgRgba8            tst = decodeJpeg(ub),
                        ref = loadImage(imgFile);
    FGASSERT(isApproxEqual(tst,ref,3U));
}

void                testShrink(CLArgs const &)
{
    ImgUI               in {4,3,
        {
            3,  4,  5,  9,
            6,  7,  8, 10,
            9, 10, 11,  13
        }
    },
                        ref {2,1,{5,8}},
                        out = shrink2Acc(in);
    FGASSERT(out == ref);
}

void                testSmooth(CLArgs const &)
{
    {
        Uints               in {3,5,7},
                            out {0,0,0},
                            refZ {11,20,19},
                            refM {14,20,26};
        smoothAcc1D_<BorderPolicy::zero>(in.data(),out.data(),in.size());
        FGASSERT(out == refZ);
        smoothAcc1D_<BorderPolicy::mirror>(in.data(),out.data(),in.size());
        FGASSERT(out == refM);
    }
    {
        ImgUI               in {3,3,
            {
                3,  4,  5,
                6,  7,  8,
                9, 10,  11
            }
        },
                            out,
                            refZ {3,3,
            {
                39, 60, 51,
                76, 112,92,
                75, 108,87
            }
        },
                            refM {3,3,
            {
                64, 76, 88,
                100,112,124,
                136,148,160
            }
        };
        // cancel out the divide in the smooth for more accurate test:
        for (uint & p : in.m_data) p *= 16;
        smoothAcc_<BorderPolicy::zero>(in,out);
        FGASSERT(out == refZ);
        smoothAcc_<BorderPolicy::mirror>(in,out);
        FGASSERT(out == refM);
    }
    {
        Rgba8 const         z {0},
                            c {32,64,128,255};
        ImgRgba8            in {3,3,
            {   // put value in lower corner since 3rd row/col will be ignored by mipmap:
                c,  z,  z,
                z,  z,  z,
                z,  z,  z
            }
        },
            ref {3,3,
            {
                c,  c,  c,
                c,  c,  c,
                c,  c,  c,
            }
        },
                            out = extrapolateForMipmap(in);
        FGASSERT(out == ref);
    }
}

void                testTransform(CLArgs const &)
{
    AffineEw2F          identity;
    Vec2UIs             dimss {{16,32},{19,47}};
    float               maxDiff = scast<float>(epsBits(20));
    for (Vec2UI dims : dimss) {
        AffineEw2F          xf0 = cIucsToIrcsXf(dims),
                            xf0i = xf0.inverse(),
                            xf1 = cIrcsToIucs(dims),
                            xf1i = xf1.inverse();
        FGASSERT(isApproxEqual(xf0*xf0i,identity,maxDiff));
        FGASSERT(isApproxEqual(xf0*xf1,identity,maxDiff));
        FGASSERT(isApproxEqual(xf1*xf1i,identity,maxDiff));
    }
}

}

void                testmImage(CLArgs const & args)
{
    Cmds                cmds {
        {resamp,"resamp","resample adaptive"},
        {testmResize,"resize","change image pixel size by resampling"},
        {testmSfs,"sfs","smoothFloat speed"},
    };
    doMenu(args,cmds);
}

void                testDecodeJfif(CLArgs const &);

void                testImage(CLArgs const & args)
{
    Cmds                cmds {
        {testBlerp,"blerp","bilinear interpolation"},
        {testComposite,"composite"},
        {testDecodeJfif,"jfif"},
        {testDecodeJpeg,"jpg"},
        {testShrink,"shrink"},
        {testSmooth,"smooth"},
        {testTransform,"xf"},
    };
    doMenu(args,cmds,true,false);
}

}
