//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgImgDisplay.hpp"
#include "FgGuiApi.hpp"
#include "FgFileSystem.hpp"
#include "FgTransform.hpp"
#include "FgCommand.hpp"
#include "FgGuiApi.hpp"

using namespace std;

namespace Fg {

ImageLmsName::ImageLmsName(String8 const & imgFname) :
    img {loadImage(imgFname)},
    name {pathToBase(imgFname)}
{
    String8             lmsFname = pathToDirBase(imgFname) + ".lms.txt";
    if (fileExists(lmsFname))
        lmsIrcs = loadLandmarks(lmsFname);
}

ImageLmsName::ImageLmsName(String8 const & imgFname,String8 const & lmsFname) :
    img {loadImage(imgFname)},
    lmsIrcs {loadLandmarks(lmsFname)},
    name {pathToBase(imgFname)}
{}

ostream &           operator<<(ostream & os,ImageLmsName const & iln)
{
    os << iln.name << fgpush << iln.img << fgpop;
    if (!iln.lmsIrcs.empty()) {
        Mat22F              bnds {
            -0.5f,  scast<float>(iln.img.width())-0.5f,
            -0.5,   scast<float>(iln.img.height())-0.5f};
        os << "Landmarks:" << fgpush;
        for (NameVec2F const & lm : iln.lmsIrcs) {
            os << fgnl << lm;
            if (!isInBounds(bnds,lm.vec))
                fgout << " WARNING point not within image boundaries";
        }
        os << fgpop;
    }
    return os;
}

void                viewImage(ImgRgba8 const & img,Vec2Fs const & lmsIrcs)
{
    IPT<String8>        title {"FaceGen SDK viewImage"};
    String8             saveDir = getDirUserAppDataLocalFaceGen({"SDK","viewImage"});
    Affine2F            xf {cIrcsToIucs<float>(img.dims())};
    NameVec2Fs          ptsIucs = mapCall(lmsIrcs,[xf](Vec2F l){return NameVec2F{xf*l}; });
    GuiPtr              win = guiImageCtrls(makeIPT(img),makeIPT(ptsIucs),true).win;
    guiStartImpl(title,win,saveDir);
}

void                viewImages(ImageLmsNames const & ais)
{
    FGASSERT(!ais.empty());
    IPT<String8>        title {"FaceGen SDK viewImages"};
    String8             saveDir = getDirUserAppDataLocalFaceGen({"SDK","viewImages"});
    Svec<NPT<String8>>  currLmNs;
    GuiPtrs             imgWs;
    for (ImageLmsName const & ai : ais) {
        Affine2F            xf {cIrcsToIucs<float>(ai.img.dims())};
        NameVec2Fs          ptsIucs = mapMulR(xf,ai.lmsIrcs);
        IPT<NameVec2Fs>     ptsIucsN {ptsIucs};
        GuiImg              guiImg = guiImageCtrls(makeIPT(ai.img),ptsIucsN,true);
        auto                fn = [ptsIucsN](size_t const & v) -> String8
        {
            if (v < lims<size_t>::max())
                return ptsIucsN.ref().at(v).name;
            else
                return {"none"};
        };
        currLmNs.push_back(link1(guiImg.draggingLmN,fn));
        imgWs.push_back(guiImg.win);
    }
    IPT<size_t>         selN = makeSavedIPTEub<size_t>(0,saveDir+"sel",ais.size());
    OPT<String8>        textN = link1(selN,[&](size_t i){return String8{toStr(ais[i])}; });
    OPT<String8>        lmTextN = linkSelect(currLmNs,selN);
    GuiPtr              selW = guiRadio(mapMember(ais,&ImageLmsName::name),selN),
                        infoW = guiGroupbox("Image Info",guiTextLines(textN,80,80)),
                        imgW = guiSelect(selN,imgWs),
                        currLmW = guiGroupbox("Landmark",guiText(lmTextN)),
                        mainL = guiSplitV({guiSplitScroll({selW}),currLmW,infoW}),
                        mainW = guiSplitH({mainL,imgW});
    guiStartImpl(title,mainW,saveDir);
}

void                viewImages(ImgRgba8s const & imgs)
{
    size_t              idx {0};
    auto                fn = [&](ImgRgba8 const & img){return ImageLmsName{img,{},toStr(idx++)}; };
    viewImages(mapCall(imgs,fn));
}

NameVec2Fs          markImage(ImgRgba8 const & img,NameVec2Fs const & existing,Strings const & newLabels)
{
    FGASSERT(!img.empty());
    AxAffine2F          iucsToIrcs = cIucsToIrcs<float>(img.dims()),
                        ircsToIucs = iucsToIrcs.inverse();
    Strings             labelsAll = mapMember(existing,&NameVec2F::name);    // defined then to-be-defined labels
    for (String const & nl : newLabels)
        if (!contains(existing,nl))
            labelsAll.push_back(nl);
    String8             store = getDirUserAppDataLocalFaceGen({"SDK","MarkImage"});
    IPT<size_t>         stepN {0};                                          // 0 - place points, 1 - confirm
    IPT<ImgRgba8>       imgN {img};
    IPT<NameVec2Fs>     ptsIucsN {mapMulR(ircsToIucs,existing)};             // combined old then new points
    auto                textFn = [&](NameVec2Fs const & ptsIucs)
    {
        size_t              idxNew = ptsIucs.size();
        if (idxNew == labelsAll.size())
            return String8{"Close window to save"};
        return String8{"Ctrl-click on "+labelsAll[idxNew]};
    };
    auto                backFn = [ptsIucsN]()
    {
        NameVec2Fs &        ptsIucs = ptsIucsN.ref();
        if (ptsIucs.size() > 0)
            ptsIucs.pop_back();
    };
    auto                leftClickFn = [&labelsAll,ptsIucsN](Vec2F posIucs,Vec2UI)
    {
        NameVec2Fs &        ptsIucs = ptsIucsN.ref();
        size_t              idx = ptsIucs.size();
        if (idx < labelsAll.size())
            ptsIucs.emplace_back(labelsAll[idx],posIucs);
    };
    OPT<String8>        textN = link1(ptsIucsN,textFn);
    GuiPtr              textW = guiText(textN),
                        buttonW = guiButton("Back",backFn),
                        imgW = guiImageCtrls(imgN,ptsIucsN,false,leftClickFn).win,
                        mainW = guiSplitV({textW,buttonW,imgW});
    guiStartImpl(IPT<String8>{"FaceGen Mark Image"},mainW,store);
    return mapMulR(iucsToIrcs,ptsIucsN.val());
}

Vec2Fs              markImageMulti(ImgRgba8 const & img,String label,Vec2Fs existing)
{
    AxAffine2F          ircsToIucs = cIrcsToIucs<float>(img.dims()),
                        iucsToIrcs = ircsToIucs.inverse();
    IPT<ImgRgba8>       imgN {img};
    IPT<NameVec2Fs>     ptsIucsN;
    for (Vec2F e : existing)
        ptsIucsN.ref().emplace_back(label,ircsToIucs*e);
    auto                leftClickFn = [=](Vec2F iucs,Vec2UI)
    {
        ptsIucsN.ref().emplace_back(label,iucs);
    };
    auto                undoFn = [=]()
    {
        NameVec2Fs &        pts = ptsIucsN.ref();
        if (!pts.empty())
            pts.pop_back();
    };
    GuiImg              gic = guiImageCtrls(imgN,ptsIucsN,false,leftClickFn);
    GuiPtr              zoomW = guiSplitH({
            guiText("Zoom:"),
            guiButton(String8("+"),gic.zoomIn),
            guiButton(String8("-"),gic.zoomOut),
    }),
                        textW = guiText("ctrl-click on points for: "+label),
                        undoW = guiButton("undo",undoFn),
                        imgW = gic.win,
                        mainW = guiSplitH({guiSplitV({textW,undoW,zoomW}),imgW});
    String              id = "markImageMulti";
    guiStartImpl(IPT<String8>{id},mainW,getDirUserAppDataLocalFaceGen({"SDK",id}));
    return mapMulR(iucsToIrcs,mapMember(ptsIucsN.val(),&NameVec2F::vec));
}

void                testGuiImageMark(CLArgs const & args)
{
    NameVec2Fs          pts {
        {"test point",{420.0f,840.0f}},
    };
    if (!isAutomated(args))
        markImage(loadImage(dataDir()+"base/Jane.jpg"),pts,{String{"Label"}});
}

void                compareImages(Img3F const & image0,Img3F const & image1)
{
    FGASSERT(image0.dims() == image1.dims());
    Arr<Arr3F,2>        bnds0 = cBounds(image0.m_data),
                        bnds1 = cBounds(image1.m_data);
    FGASSERT((cMinElem(bnds0[0]) >= 0) && (cMaxElem(bnds0[1]) <= 1));
    FGASSERT((cMinElem(bnds1[0]) >= 0) && (cMaxElem(bnds1[1]) <= 1));
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
        float               m = pow(cMagD(l-r),0.5f);    // gamma correct for better viewing
        return Arr3F{m,m,m};
    };
    Img3F               imgdm = mapCall(image0,image1,diffMagFn);
    String8             store = getDirUserAppDataLocalFaceGen({"SDK","ImageComparison"});
    IPT<size_t>         selN = makeSavedIPT<size_t>(0,store+"Sel");
    auto                selFn = [&](size_t const & sel)
    {
        Svec<Img3F const *> sels {&image0,&image1,&img0m1,&img1m0,&imgdm};
        FGASSERT(sel < sels.size());
        return cMipmap(*sels[sel]);
    };
    NPT<Img3Fs>         pyramidN = link1(selN,selFn);
    // Image relative view scale is 2^zoom display pixels per image pixel:
    IPT<int>            zoomN = makeSavedIPT(0,store+"zoom");
    IPT<int>            dragAccN {0};
    auto                dragRightFn = [=](Vec2I,Vec2I delta)
    {
        int constexpr       dragThresh = 40;
        int &               dragAcc = dragAccN.ref();
        dragAcc += delta[1];
        if (abs(dragAcc) <= dragThresh)
            return GuiCursor::arrow;
        int                 zoom = zoomN.val();
        if ((dragAcc < -dragThresh) && (zoom > -3))
            --zoom;
        if ((dragAcc > dragThresh) && (zoom < 2))
            ++zoom;
        if (zoom != zoomN.val())                // don't trigger background redraw (flicker) unless zoom changed
            zoomN.set(zoom);
        dragAcc = 0;
        return GuiCursor::arrow;
    };
    IPT<Vec2F>          shiftIucsN {Vec2F{0}};   // image centre relative to display centre
    auto                dragLeftFn = [=](Vec2I,Vec2I delta)
    {
        int                 zoom = zoomN.val();
        Img3Fs const &      pyr = pyramidN.val();
        Vec2UI              imgDims;
        if (zoom <= 0)
            imgDims = pyr[-zoom].dims();
        else
            imgDims = pyr[0].dims() * (1 << zoom);
        shiftIucsN.ref() += mapDiv(Vec2F(delta),Vec2F(imgDims));
        return GuiCursor::arrow;
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
    OPT<ImgRgba8>        dispImgN = link2(pyramidN,zoomN,dispImgFn);
    auto                offsetFn = [](uint imgDim,uint winDim,float & shiftIucs)
    {
        int                 offset = (int(winDim) - int(imgDim)) / 2,       // centre image in window
                            offShift = offset;
        if (winDim < imgDim) {                                              // room to shift image
            int                 shift = roundT<int>(shiftIucs * imgDim),
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
        ImgRgba8 const &     img = dispImgN.val();
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
    gi.updateFlag = cUpdateFlagT(zoomN);
    gi.updateNofill = cUpdateFlagT(dispImgN,shiftIucsN);
    gi.mouseMoveFns[1] = dragLeftFn;
    gi.mouseMoveFns[4] = dragRightFn;
    GuiPtr              imgW = guiPtr(gi);
    String8s            labels {"A","B","A-B","B-A","|B-A|",};
    GuiPtr              selW = guiRadio(labels,selN),
                        mainW = guiSplitAdj(true,imgW,selW);
    guiStartImpl(IPT<String8>{"Image Comparison View"},mainW,store);
}

}
