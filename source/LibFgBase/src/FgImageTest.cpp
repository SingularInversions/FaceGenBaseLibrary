//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
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

// MANUAL:

void
display(CLArgs const &)
{
    String8     dd = dataDir();
    string      testorig_jpg("base/test/testorig.jpg");
    ImgC4UC     img = loadImage(dd+testorig_jpg);
    viewImage(img);
    string      testorig_bmp("base/test/testorig.bmp");
    loadImage_(dd+testorig_bmp,img);
    viewImage(img);
}

void
resize(CLArgs const &)
{
    string          fname("base/test/testorig.jpg");
    ImgC4UC         img = loadImage(dataDir()+fname);
    ImgC4UC         out(img.width()/2+1,img.height()+1);
    imgResize(img,out);
    viewImage(out);
}

void
sfs(CLArgs const &)
{
    ImgC4UC         orig = loadImage(dataDir()+"base/Mandrill512.png");
    Img<Vec3F>      img(orig.dims());
    for (size_t ii=0; ii<img.numPixels(); ++ii)
        img[ii] = Vec3F(mapCast<float>(cHead<3>(orig[ii].m_c)));
    Timer             time;
    for (uint ii=0; ii<100; ++ii)
        smoothFloat(img,img,1);
    double              ms = time.read();
    fgout << fgnl << "smoothFloat time: " << ms;
    viewImage(img);
}

// AUTOMATIC:

void
testComposite(CLArgs const &)
{
    String8             dd = dataDir();
    ImgC4UC             overlay = loadImage(dd+"base/Teeth512.png"),
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
    smoothFloat(tst,i0,1);
    fgConvolveFloat(tst,Mat33F(1,2,1,2,4,2,1,2,1)/16.0f,i1,1);
    //fgout << fgnl << i0.m_data << fgnl << i1.m_data;
    FGASSERT(isApproxEqualRelMag(i0.m_data,i1.m_data));
}

}

void
fgImageTestm(CLArgs const & args)
{
    Cmds   cmds;
    cmds.push_back(Cmd(resize,"resize"));
    cmds.push_back(Cmd(display,"display"));
    cmds.push_back(Cmd(sfs,"sfs","smoothFloat speed"));
    doMenu(args,cmds);
}

void    fgImgTestWrite(CLArgs const &);

void
fgImageTest(CLArgs const & args)
{
    Cmds       cmds;
    cmds.push_back(Cmd(testComposite,"composite"));
    cmds.push_back(Cmd(testConvolve,"conv"));
    cmds.push_back(Cmd(fgImgTestWrite,"write"));
    doMenu(args,cmds,true,false,true);
}

}
