//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
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

void
testmCompare(CLArgs const &)
{
    String8             dd = dataDir();
    Img3F               img0 = toUnit3F(loadImage(dd+"base/Mandrill512.png")),
                        img1 = toUnit3F(loadImage(dd+"base/test/imgops/composite.png"));
    compareImages(img0,img1);
}

void
testmDisplay(CLArgs const &)
{
    String8     dd = dataDir();
    string      testorig_jpg("base/test/testorig.jpg");
    ImgRgba8     img = loadImage(dd+testorig_jpg);
    viewImage(img);
    string      testorig_bmp("base/test/testorig.bmp");
    loadImage_(dd+testorig_bmp,img);
    viewImage(img);
}

void
testmResize(CLArgs const &)
{
    string          fname("base/test/testorig.jpg");
    ImgRgba8         img = loadImage(dataDir()+fname);
    ImgRgba8         out(img.width()/2+1,img.height()+1);
    imgResize(img,out);
    viewImage(out);
}

void
resamp(CLArgs const &)
{
    Img3F                   in = toUnit3F(loadImage(dataDir()+"base/Mandrill512.png"));
    Vec2D                   lo {94.3,37.8};
    for (uint lnSz = 5; lnSz < 10; ++lnSz) {
        uint                    sz = 2 << lnSz;
        Img3F                   out = resampleAdaptive(in,lo,309.7,sz);
        viewImageFixed(toRgba8(out));
    }
}

void
testmSfs(CLArgs const &)
{
    ImgRgba8         orig = loadImage(dataDir()+"base/Mandrill512.png");
    Img<Vec3F>      img(orig.dims());
    for (size_t ii=0; ii<img.numPixels(); ++ii)
        img[ii] = Vec3F(mapCast<float>(cHead<3>(orig[ii].m_c)));
    Timer             time;
    for (uint ii=0; ii<100; ++ii)
        smoothFloat_(img,img,1);
    double              ms = time.elapsedSeconds();
    fgout << fgnl << "smoothFloat time: " << ms;
    viewImage(img);
}

void
testComposite(CLArgs const &)
{
    String8             dd = dataDir();
    ImgRgba8             overlay = loadImage(dd+"base/Teeth512.png"),
                        base = loadImage(dd+"base/Mandrill512.png");
    regressTest(composite(overlay,base),dd+"base/test/imgops/composite.png");
}

void
testConvolve(CLArgs const &)
{
    randSeedRepeatable();
    ImgF          tst(16,16);
    for (size_t ii=0; ii<tst.numPixels(); ++ii)
        tst[ii] = float(randUniform());
    ImgF          i0,i1;
    smoothFloat_(tst,i0,1);
    fgConvolveFloat(tst,Mat33F(1,2,1,2,4,2,1,2,1)/16.0f,i1,1);
    //fgout << fgnl << i0.m_data << fgnl << i1.m_data;
    FGASSERT(isApproxEqualRelMag(i0.m_data,i1.m_data));
}

void
testTransform(CLArgs const &)
{
    AffineEw2F          identity;
    Vec2UIs             dimss {{16,32},{19,47}};
    float               maxDiff = scast<float>(epsBits(20));
    for (Vec2UI dims : dimss) {
        AffineEw2F          xf0 = cIucsToIrcsXf(dims),
                            xf0i = xf0.inverse(),
                            xf1 = cIrcsToIucsXf(dims),
                            xf1i = xf1.inverse();
        FGASSERT(isApproxEqual(xf0*xf0i,identity,maxDiff));
        FGASSERT(isApproxEqual(xf0*xf1,identity,maxDiff));
        FGASSERT(isApproxEqual(xf1*xf1i,identity,maxDiff));
    }
}

}

void
testmImage(CLArgs const & args)
{
    Cmds                cmds {
        {testmCompare,"comp","view to compare images"},
        {testmDisplay,"disp","display a single image"},
        {resamp,"resamp","resample adaptive"},
        {testmResize,"resize","change image pixel size by resampling"},
        {testmSfs,"sfs","smoothFloat speed"},
    };
    doMenu(args,cmds);
}

void    fgImgTestWrite(CLArgs const &);

void
testImage(CLArgs const & args)
{
    Cmds                cmds {
        {testComposite,"composite"},
        {testConvolve,"conv"},
        {testTransform,"xf"},
        {fgImgTestWrite,"write"},
    };
    doMenu(args,cmds,true,false,true);
}

}
