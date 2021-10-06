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
#include "FgGuiApi.hpp"

using namespace std;

namespace Fg {

void
viewImageFixed(ImgRgba8 const & img)
{
    guiStartImpl(
        makeIPT<String8>("FaceGen SDK viewImageFixed"),
        guiImage(makeIPT(img)),
        getDirUserAppDataLocalFaceGen("SDK","viewImageFixed"));
}

void
viewImage(ImgRgba8 const & img,Vec2Fs const & pts,String const & name)
{
    if (!name.empty())
        fgout << name << ": " << fgpush << img << fgpop;
    guiStartImpl(
        makeIPT<String8>("FaceGen SDK DisplayImage"),
        guiImageCtrls(makeIPT(img),makeIPT(pts)).win,
        getDirUserAppDataLocalFaceGen("SDK","DisplayImage"));
}

void
viewImage(const Img<ushort> & img)
{
    Affine1F      aff(VecF2(cBounds(img.dataVec())),VecF2(0,255));
    ImgRgba8     di(img.dims());
    for (size_t ii=0; ii<img.numPixels(); ++ii)
        di[ii] = Rgba8(aff * img[ii]);
    viewImage(di);
}

void
viewImage(const ImgF & img)
{
    Affine1F          aff(cBounds(img.m_data),VecF2(0,255));
    ImgRgba8         dispImg(img.dims());
    for (size_t ii=0; ii<img.m_data.size(); ++ii)
        dispImg.m_data[ii] = Rgba8(uchar(aff * img.m_data[ii]));
    viewImage(dispImg);
}

void
viewImage(const ImgD & img)
{
    Affine1D          aff(cBounds(img.m_data),VecD2(0,255));
    ImgRgba8         dispImg(img.dims());
    for (size_t ii=0; ii<img.m_data.size(); ++ii)
        dispImg.m_data[ii] = Rgba8(uchar(aff * img.m_data[ii]));
    viewImage(dispImg);
}

void
viewImage(const ImgV3F & img)
{
    VecF2               bounds = cBounds(cBounds(img.dataVec()).m);
    AffineEw3F          xform(Vec3F(-bounds[0]),Vec3F(255.0f/(bounds[1]-bounds[0])));
    ImgV3F               tmp = ImgV3F(img.dims(),mapMul(xform,img.dataVec()));
    ImgRgba8             disp(tmp.dims());
    for (size_t ii=0; ii<disp.numPixels(); ++ii) {
        Vec3UC          clr = mapRound<uchar>(tmp[ii]);
        disp[ii] = Rgba8(clr[0],clr[1],clr[2],uchar(255));
    }
    fgout << fgnl << disp;
    viewImage(disp);
}

ImagePoints
markImage(ImgRgba8 const & img,ImagePoints const & existing,Strings const & newLabels)
{
    FGASSERT(!img.empty());
    AffineEw2F          iucsToIrcs = cIucsToIrcsXf(img.dims()),
                        ircsToIucs = iucsToIrcs.inverse();
    Vec2Fs              existingPtsIucs;
    Strings             labels;
    for (ImagePoint const & e : existing) {
        existingPtsIucs.push_back(ircsToIucs * e.posIrcs);
        labels.push_back(e.label);
    }
    for (String const & nl : newLabels)
        if (!contains(labels,nl))
            labels.push_back(nl);
    String8             store = getDirUserAppDataLocalFaceGen("SDK","MarkImage");
    IPT<size_t>         stepN {0};              // 0 - place points, 1 - confirm
    IPT<ImgRgba8>       imgN {magnify(img,2)};  // Best point pick is 2x magnified
    IPT<Vec2Fs>         ptsIucsN {existingPtsIucs};
    auto                textFn = [&labels](Vec2Fs const & ptsIucs)
    {
        if (ptsIucs.size() == labels.size())
            return String8{"Close window to save"};
        return String8{"Click on "+labels[ptsIucs.size()]};
    };
    auto                backFn = [ptsIucsN]()
    {
        Vec2Fs &            ptsIucs = ptsIucsN.ref();
        if (ptsIucs.size() > 0)
            ptsIucs.pop_back();
    };
    auto                leftClickFn = [&labels,ptsIucsN](Vec2F posIucs)
    {
        Vec2Fs &            ptsIucs = ptsIucsN.ref();
        if (ptsIucs.size() < labels.size())
            ptsIucs.push_back(posIucs);
    };
    OPT<String8>        textN = link1<Vec2Fs,String8>(ptsIucsN,textFn);
    GuiPtr              textW = guiText(textN),
                        buttonW = guiButton("Back",backFn),
                        imgW = guiImageCtrls(imgN,ptsIucsN,leftClickFn).win,
                        mainW = guiSplitV({textW,buttonW,imgW});
    guiStartImpl(IPT<String8>{"FaceGen Mark Image"},mainW,store);
    Vec2Fs const &      ptsIucs = ptsIucsN.cref();
    FGASSERT(ptsIucs.size() <= labels.size());
    ImagePoints         ret; ret.reserve(ptsIucs.size());
    for (size_t ii=0; ii<ptsIucs.size(); ++ii)
        ret.emplace_back(labels[ii],iucsToIrcs * ptsIucs[ii]);
    return ret;
}

void
testmGuiImage(CLArgs const &)
{
    viewImage(loadImage(dataDir()+"base/trees.jpg"));
}

void
viewImages(ImgRgba8s const & images,String8s names)
{
    String8                 store = getDirUserAppDataLocalFaceGen("SDK","ViewImages");
    while (names.size() < images.size())
        names.push_back("Unnamed-"+toStr(names.size()));
    FGASSERT(images.size() == names.size());
    IPT<size_t>             selN {0};
    OPT<ImgRgba8>           imgN = link1<size_t,ImgRgba8>(selN,[&](size_t idx){return images.at(idx); });
    GuiPtr                  imgW = guiImage(imgN),
                            selW = guiRadio(names,selN),
                            mainW = guiSplitH({imgW,selW});
    IPT<String8>            title {"View Images"};
    guiStartImpl(title,mainW,store);
}

void
compareImages(Img3F const & image0,Img3F const & image1)
{
    FGASSERT(image0.dims() == image1.dims());
    Mat32F              bnds0 = cBounds(image0.m_data),
                        bnds1 = cBounds(image1.m_data);
    FGASSERT((cMinElem(bnds0) >= 0) && (cMaxElem(bnds0) <= 1));
    FGASSERT((cMinElem(bnds1) >= 0) && (cMaxElem(bnds1) <= 1));
    auto                hdiffFn = [](Arr3F const & l,Arr3F const & r)
    {
        Arr3F           ret;
        for (uint ii=0; ii<3; ++ii)
            ret[ii] = 0.5f + (l[ii]-r[ii])*0.5f;        // gamma correction not helpful here
        return ret;
    };
    Img3F               img1m0 = mapCall(image1,image0,hdiffFn),
                        img0m1 = mapCall(image0,image1,hdiffFn);
    auto                diffMagFn = [](Arr3F const & l,Arr3F const & r)
    {
        float               m = pow(cMag(l-r),0.5f);    // gamma correct for better viewing
        return Arr3F{m,m,m};
    };
    Img3F               imgdm = mapCall(image0,image1,diffMagFn);
    String8             store = getDirUserAppDataLocalFaceGen("SDK","ImageComparison");
    IPT<size_t>         selN = makeSavedIPT<size_t>(0,store+"Sel");
    auto                selFn = [&](size_t const & sel)
    {
        Svec<Img3F const *> sels {&image0,&image1,&img0m1,&img1m0,&imgdm};
        FGASSERT(sel < sels.size());
        return cMipMapTruncate(*sels[sel]);
    };
    NPT<Img3Fs>         pyramidN = link1<size_t,Img3Fs>(selN,selFn);
    // Image relative view scale is 2^zoom display pixels per image pixel:
    IPT<int>            zoomN = makeSavedIPT(0,store+"zoom");
    IPT<int>            dragAccN {0};
    auto                dragRightFn = [=](Vec2I delta)
    {
        int constexpr       dragThresh = 40;
        int &               dragAcc = dragAccN.ref();
        dragAcc += delta[1];
        if (abs(dragAcc) <= dragThresh)
            return;
        int                 zoom = zoomN.val();
        if ((dragAcc < -dragThresh) && (zoom > -3))
            --zoom;
        if ((dragAcc > dragThresh) && (zoom < 2))
            ++zoom;
        if (zoom != zoomN.val())                // don't trigger background redraw (flicker) unless zoom changed
            zoomN.set(zoom);
        dragAcc = 0;
    };
    IPT<Vec2F>          shiftIucsN {Vec2F{0}};   // image centre relative to display centre
    auto                dragLeftFn = [=](Vec2I delta)
    {
        int                 zoom = zoomN.val();
        Img3Fs const &      pyr = pyramidN.cref();
        Vec2UI              imgDims;
        if (zoom <= 0)
            imgDims = pyr[-zoom].dims();
        else
            imgDims = pyr[0].dims() * (1 << zoom);
        shiftIucsN.ref() += mapDiv(Vec2F(delta),Vec2F(imgDims));
    };
    auto                dispImgFn = [](Img3Fs const & pyr,int const & zoom)
    {
        if (zoom < 0) {
            uint                sl = cMin(uint(-zoom),uint(pyr.size()));
            return toRgba8(pyr[sl]);
        }
        else {
            ImgRgba8            c4uc = toRgba8(pyr[0]);
            uint                fac = 1 << zoom;
            return magnify(c4uc,fac);
        }
    };
    OPT<ImgRgba8>        dispImgN = link2<ImgRgba8,Img3Fs,int>(pyramidN,zoomN,dispImgFn);
    auto                offsetFn = [](uint imgDim,uint winDim,float & shiftIucs)
    {
        int                 offset = (int(winDim) - int(imgDim)) / 2,       // centre image in window
                            offShift = offset;
        if (winDim < imgDim) {                                              // room to shift image
            int                 shift = round<int>(shiftIucs * imgDim),
                                lo = int(winDim) - int(imgDim),
                                hi = 0;
            offShift = clamp(offset+shift,lo,hi);
            // Update shiftIucs to clamp:
            shift = offShift - offset;
            shiftIucs = float(shift) / float(imgDim);
        }
        return offShift;
    };
    auto                getImgFn = [=](Vec2UI winDims)
    {
        ImgRgba8 const &     img = dispImgN.cref();
        Vec2UI              imgDims = img.dims();
        Vec2F &             shiftIucs = shiftIucsN.ref();
        Vec2I               offset;
        for (uint ii=0; ii<2; ++ii)
            offset[ii] = offsetFn(imgDims[ii],winDims[ii],shiftIucs[ii]);
        return GuiImage::Disp{&img,offset};
    };
    GuiImage            gi;
    gi.getImgFn = getImgFn;
    gi.wantStretch = {true,true};
    gi.minSizeN = makeIPT(Vec2UI{128});
    gi.updateFlag = makeUpdateFlag(zoomN);
    gi.updateNofill = makeUpdateFlag(dispImgN,shiftIucsN);
    gi.dragRight = dragRightFn;
    gi.dragLeft = dragLeftFn;
    GuiPtr              imgW = guiMakePtr(gi);
    String8s            labels {"A","B","A-B","B-A","|B-A|",};
    GuiPtr              selW = guiRadio(labels,selN),
                        mainW = guiSplitAdj(true,imgW,selW);
    guiStartImpl(IPT<String8>{"Image Comparison View"},mainW,store);
}

}
