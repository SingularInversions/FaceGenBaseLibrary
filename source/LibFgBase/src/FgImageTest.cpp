//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
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


using namespace std;

namespace Fg {

namespace {

void                testResize(CLArgs const & args)
{
    ImgRgba8            img = loadImage(dataDir()+"base/test/testorig.jpg");
    ImgRgba8            out(img.width()/2+1,img.height()+1);
    imgResize_(img,out);
    if (!isAutomated(args))
        viewImage(out);
}

void                testResamp(CLArgs const & args)
{
    Img3F                   in = toUnit3F(loadImage(dataDir()+"base/Mandrill512.png"));
    Vec2D                   lo {94.3,37.8};
    for (uint lnSz = 5; lnSz < 10; ++lnSz) {
        uint                    sz = 2 << lnSz;
        Img3F                   out = filterResample(in,lo,309.7f,sz);
        if (!isAutomated(args))
            viewImage(toRgba8(out));
    }
}

void                testResampRgba8(CLArgs const & args)
{
    ImgRgba8                in = loadImage(dataDir()+"base/Mandrill512.png");
    Vec2F                   lo {94.3,37.8};
    for (uint lnSz=5; lnSz<10; ++lnSz) {
        uint                    sz = 2 << lnSz;
        ImgRgba8                out = filterResample(in,lo,309.7f,sz);
        if (!isAutomated(args))
            viewImage(out);
    }
}

void                testSmoothFloat(CLArgs const & args)
{
    Img3F               img = toUnit3F(loadImage(dataDir()+"base/Mandrill512.png"));
    {
        PushTimer           timer {"floating point image smooth"};
        for (uint ii=0; ii<64; ++ii)
            smoothFloat_(img,img,1);
    }
    if (!isAutomated(args))
        viewImage(toRgba8(img));
}

void                testLerp(CLArgs const &)
{
    {
        ImgLerp                l {0,2};
        Arr2F               w {1,0};
        FGASSERT(l.lo == 0);
        FGASSERT(l.wgts == w);
    }
    {
        ImgLerp                l {0.25,2};
        Arr2F               w {0.75,0.25};
        FGASSERT(l.lo == 0);
        FGASSERT(l.wgts == w);
    }
    {
        ImgLerp                l {-0.25,2};
        Arr2F               w {0,0.75};
        FGASSERT(l.lo == -1);
        FGASSERT(l.wgts == w);
    }
    {
        ImgLerp                l {0.5,2};
        Arr2F               w {0.5,0.5};
        FGASSERT(l.lo == 0);
        FGASSERT(l.wgts == w);
    }
    {
        ImgLerp                l {-0.5,2};
        Arr2F               w {0,0.5};
        FGASSERT(l.lo == -1);
        FGASSERT(l.wgts == w);
    }
    {
        ImgLerp                l {1,2};
        Arr2F               w {1,0};
        FGASSERT(l.lo == 1);
        FGASSERT(l.wgts == w);
    }
    {
        ImgLerp                l {-1,2};
        Arr2F               w {0,0};
        FGASSERT(l.lo == -1);
        FGASSERT(l.wgts == w);
    }
    {
        ImgLerp                l {1.5,2};
        Arr2F               w {0.5,0};
        FGASSERT(l.lo == 1);
        FGASSERT(l.wgts == w);
    }
}

void                testBlerp(CLArgs const &)
{
    ImgF            two {2,2,{
        1.0f,   2.0f,
        4.0f,   8.0f
    }};
    auto            fn = [&two](Vec2F ircs,float resultZero,float /*resultClamp*/)
    {
        {
            ImgBlerp           blerp {ircs,two.dims()};
            float           val = blerp.sampleZero(two);
            FGASSERT(val == resultZero);
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

void                testResample(CLArgs const &)
{
    Img4F               src {{2,2},{
        {1,1,1,1},  {3,3,3,1},
        {5,5,5,1},  {7,7,7,1},
    }};
    SquareF             regionPacs {{1,0.5},1.5};
    Img4F               dst = resample(src,regionPacs,3,false);
    float               tol = scast<float>(epsBits(20));
    {
        Mat22F              vals {1,3,5,7},
                            wgts = Vec2F{.75,.25} * VecF2{.25,.75},
                            prod = mapMul(vals,wgts);
        float               p = cSumElems(prod);
        FGASSERT(isApproxEqual(p,dst.m_data[0][0],tol));
    }
    {
        Mat22F              vals {3,0,7,0},
                            wgts = Vec2F{.75,.25} * VecF2{.75,.25},
                            prod = mapMul(vals,wgts);
        float               p = cSumElems(prod);
        FGASSERT(isApproxEqual(p,dst.m_data[1][0],tol));
    }
    {
        Mat22F              vals {3,0,7,0},
                            wgts = Vec2F{.75,.25} * VecF2{.25,.75},
                            prod = mapMul(vals,wgts);
        float               p = cSumElems(prod);
        FGASSERT(isApproxEqual(p,dst.m_data[2][0],tol));
    }
    {
        Mat22F              vals {1,3,5,7},
                            wgts = Vec2F{.25,.75} * VecF2{.25,.75},
                            prod = mapMul(vals,wgts);
        float               p = cSumElems(prod);
        FGASSERT(isApproxEqual(p,dst.m_data[3][0],tol));
    }
    {
        Mat22F              vals {3,0,7,0},
                            wgts = Vec2F{.25,.75} * VecF2{.75,.25},
                            prod = mapMul(vals,wgts);
        float               p = cSumElems(prod);
        FGASSERT(isApproxEqual(p,dst.m_data[4][0],tol));
    }
    {
        Mat22F              vals {3,0,7,0},
                            wgts = Vec2F{.25,.75} * VecF2{.25,.75},
                            prod = mapMul(vals,wgts);
        float               p = cSumElems(prod);
        FGASSERT(isApproxEqual(p,dst.m_data[5][0],tol));
    }
    {
        Mat22F              vals {5,7,0,0},
                            wgts = Vec2F{.75,.25} * VecF2{.25,.75},
                            prod = mapMul(vals,wgts);
        float               p = cSumElems(prod);
        FGASSERT(isApproxEqual(p,dst.m_data[6][0],tol));
    }
    {
        Mat22F              vals {7,0,0,0},
                            wgts = Vec2F{.75,.25} * VecF2{.75,.25},
                            prod = mapMul(vals,wgts);
        float               p = cSumElems(prod);
        FGASSERT(isApproxEqual(p,dst.m_data[7][0],tol));
    }
    {
        Mat22F              vals {7,0,0,0},
                            wgts = Vec2F{.75,.25} * VecF2{.25,.75},
                            prod = mapMul(vals,wgts);
        float               p = cSumElems(prod);
        FGASSERT(isApproxEqual(p,dst.m_data[8][0],tol));
    }
}

void                testBlockResample(CLArgs const &)
{
    auto                srcFn = [](size_t ii){float v=scast<float>(ii); return Arr4F{v,v,v,1}; };
    Img4F               src {{4,4},genSvec<Arr4F>(16,srcFn)},
                        dst = blockResample(src,{{1.75f,0.5f},3.0f},2,false);
    float               tol = scast<float>(epsBits(20));
    {
        Mat23F              vals {1,2,3,5,6,7},
                            wghts = Vec2F{0.5f,1.0f} * VecF3{0.25f,1,0.25f},
                            prod = mapMul(wghts,vals);
        float               p = cSumElems(prod) / cSumElems(wghts);
        FGASSERT(isApproxEqual(p,dst.m_data[0][0],tol));
    }
    {
        Mat22F              vals {3,0,7,0},
                            wghts = Vec2F{0.5f,1.0f} * VecF2{0.75f,0.75f},
                            prod = mapMul(wghts,vals);
        float               p = cSumElems(prod) / cSumElems(wghts);
        FGASSERT(isApproxEqual(p,dst.m_data[1][0],tol));
    }
    {
        Mat23F              vals {9,10,11,13,14,15},
                            wghts = Vec2F{1.0f,0.5f} * VecF3{0.25f,1,0.25f},
                            prod = mapMul(wghts,vals);
        float               p = cSumElems(prod) / cSumElems(wghts);
        FGASSERT(isApproxEqual(p,dst.m_data[2][0],tol));
    }
    {
        Mat22F              vals {11,0,15,0},
                            wghts = Vec2F{1.0f,0.5f} * VecF2{0.75f,0.75f},
                            prod = mapMul(wghts,vals);
        float               p = cSumElems(prod) / cSumElems(wghts);
        FGASSERT(isApproxEqual(p,dst.m_data[3][0],tol));
    }
}
void                testComposite(CLArgs const &)
{
    String8             dd = dataDir();
    ImgRgba8            overlay = loadImage(dd+"base/Teeth512.png"),
                        base = loadImage(dd+"base/Mandrill512.png");
    testRegressExact(composite(overlay,base),"base/test/imgops/composite.png");
}

void                testDecodeJpeg(CLArgs const &)
{
    String8             imgFile = dataDir() + "base/trees.jpg";
    Bytes               blob = loadRaw(imgFile);
    ImgRgba8            tst = decodeJpeg(blob),
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
    AxAffine2F          identity;
    Vec2UIs             dimss {{16,32},{19,47}};
    float               maxDiff = scast<float>(epsBits(20));
    for (Vec2UI dims : dimss) {
        AxAffine2F          xf0 = cIucsToIrcs<float>(dims),
                            xf0i = xf0.inverse(),
                            xf1 = cIrcsToIucs<float>(dims),
                            xf1i = xf1.inverse();
        FGASSERT(isApproxEqual(xf0*xf0i,identity,maxDiff));
        FGASSERT(isApproxEqual(xf0*xf1,identity,maxDiff));
        FGASSERT(isApproxEqual(xf1*xf1i,identity,maxDiff));
    }
}

}

void                testDecodeJfif(CLArgs const &);

void                testImage(CLArgs const & args)
{
    Cmds                cmds {
        {testBlerp,"blerp","bilinear interpolation"},
        {testBlockResample,"block","block resample"},
        {testComposite,"composite"},
        {testDecodeJfif,"jfif"},
        {testDecodeJpeg,"jpg"},
        {testLerp,"lerp"},
        {testResample,"resamp","resample scale/trans"},
        {testResamp,"resampf","filter resample RGBF"},
        {testResampRgba8,"resamp2","filter resample RGBA8"},
        {testResize,"resize","change image pixel size by resampling"},
        {testSmoothFloat,"sfs","smoothFloat speed"},
        {testShrink,"shrink"},
        {testSmooth,"smooth"},
        {testTransform,"xf"},
    };
    doMenu(args,cmds,true,false);
}

}
