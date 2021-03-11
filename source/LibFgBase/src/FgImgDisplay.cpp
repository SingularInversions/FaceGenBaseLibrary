//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgImgDisplay.hpp"
#include "FgGuiApiImage.hpp"
#include "FgFileSystem.hpp"
#include "FgAffine1.hpp"
#include "FgBounds.hpp"
#include "FgCommand.hpp"

using namespace std;

namespace Fg {

void
viewImage(ImgC4UC const & img,Vec2Fs const & pts,String const & name)
{
    if (!name.empty())
        fgout << name << ": " << fgpush << img << fgpop;
    guiStartImpl(
        makeIPT<String8>("FaceGen SDK DisplayImage"),
        guiImage(makeIPT(img),makeIPT(pts)),
        getDirUserAppDataLocalFaceGen("SDK","DisplayImage"));
}

void
viewImage(const ImgUC &img)
{
    ImgC4UC        dispImg;
    imgConvert_(img,dispImg);
    viewImage(dispImg);
}

void
viewImage(const Img<ushort> & img)
{
    Affine1F      aff(VecF2(cBounds(img.dataVec())),VecF2(0,255));
    ImgC4UC     di(img.dims());
    for (size_t ii=0; ii<img.numPixels(); ++ii)
        di[ii] = RgbaUC(aff * img[ii]);
    viewImage(di);
}

void
viewImage(const ImgF & img)
{
    Affine1F          aff(cBounds(img.m_data),VecF2(0,255));
    ImgC4UC         dispImg(img.dims());
    for (size_t ii=0; ii<img.m_data.size(); ++ii)
        dispImg.m_data[ii] = RgbaUC(uchar(aff * img.m_data[ii]));
    viewImage(dispImg);
}

void
viewImage(const ImgD & img)
{
    Affine1D          aff(cBounds(img.m_data),VecD2(0,255));
    ImgC4UC         dispImg(img.dims());
    for (size_t ii=0; ii<img.m_data.size(); ++ii)
        dispImg.m_data[ii] = RgbaUC(uchar(aff * img.m_data[ii]));
    viewImage(dispImg);
}

void
viewImage(const ImgV3F & img)
{
    VecF2               bounds = cBounds(cBounds(img.dataVec()).m);
    AffineEw3F          xform(Vec3F(-bounds[0]),Vec3F(255.0f/(bounds[1]-bounds[0])));
    ImgV3F               tmp = ImgV3F(img.dims(),mapMul(xform,img.dataVec()));
    ImgC4UC             disp(tmp.dims());
    for (size_t ii=0; ii<disp.numPixels(); ++ii) {
        Vec3UC          clr = round<uchar>(tmp[ii]);
        disp[ii] = RgbaUC(clr[0],clr[1],clr[2],uchar(255));
    }
    fgout << fgnl << disp;
    viewImage(disp);
}

void
fgImgDisplayColorize(const ImgD & img)
{
    VecD2        ib = cBounds(img.dataVec());
    Affine1D aff(ib,VecD2(0,4));
    ImgC4UC     di(img.dims());
    for (Iter2UI it(di.dims()); it.valid(); it.next()) {
        double      sc = aff * img[it()];
        RgbaUC    col(0,0,0,255);
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
    viewImage(di);
}

void
fgImgGuiTestm(CLArgs const &)
{
    viewImage(loadImage(dataDir()+"base/trees.jpg"));
}

}
