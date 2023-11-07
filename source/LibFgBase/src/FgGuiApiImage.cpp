//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgGuiApi.hpp"
#include "FgGuiApi.hpp"
#include "FgImage.hpp"

using namespace std;


namespace Fg {

GuiImage::GuiImage(NPT<ImgRgba8> imageN) : updateFlag {makeUpdateFlag(imageN)}
{
    updateNofill = updateFlag;
    wantStretch = Vec2B{false,false};
    minSizeN = link1<ImgRgba8,Vec2UI>(imageN,[](ImgRgba8 const & img){return img.dims();});
    getImgFn = [imageN](Vec2UI)
    {
        return GuiImage::Disp {&imageN.cref(),Vec2I{0}};
    };
}

GuiPtr              guiImage(NPT<ImgRgba8> imageN,Sfun<void(Vec2F)> onClick)
{
    GuiImage                gi {imageN};
    auto                    clickFn = [imageN,onClick](Vec2I pos)
    {
        Vec2D           iucs = cIrcsToIucs(imageN.cref().dims()) * Vec2D(pos);
        onClick(Vec2F(iucs));
    };
    gi.clickActionFns[0] = clickFn;
    return make_shared<GuiImage>(gi);
}

ImgRgba8s           cMagmip(ImgRgba8 const & img,uint log2mag)
{
    ImgRgba8s               ret;
    uint                    minDim = cMinElem(img.dims());
    if (minDim == 0)
        return ret;
    uint                    sz = log2Floor(minDim) + 1;
    ret.resize(sz);
    ret[log2mag] = img;
    for (uint ll=0; ll<log2mag; ++ll)
        ret[log2mag-ll-1] = magnify(ret[log2mag-ll],2U);
    for (uint ll=log2mag+1; ll<sz; ++ll)
        ret[ll] = shrink2(ret[ll-1]);
    return ret;
}

GuiImg              guiImageCtrls(
    NPT<ImgRgba8> const &       imageN,
    IPT<Vec2Fs> const &         ptsIucsN,
    bool                        expertMode,
    Sfun<void(Vec2F,Vec2UI)> const & onCtrlClick)
{
    auto                    mipmapFn = [=](ImgRgba8 const & img)
    {
        // some guard rails for performance on slow cpus.
        // it's tempting to set mipmapIdxN here but that undermines the dataflow update algorithm,
        // so you get a shrunk version for large images, which perhaps isn't such a bad thing:
        uint            xprt = expertMode ? 1U : 0U;
        if (img.numPixels() > (1<<24))              // > 16M pix (eg. 4Kx4K) = 64MB
            return cMagmip(img,xprt);
        else if (img.numPixels() > (1<<22))         // > 4M pix (eg. 2Kx2K) = 16MB
            return cMagmip(img,xprt+1);
        else if (img.empty())
            return ImgRgba8s{};
        return cMagmip(img,xprt+2);
    };
    OPT<ImgRgba8s>          mipmapN = link1<ImgRgba8,ImgRgba8s>(imageN,mipmapFn);
    IPT<uint>               mipmapIdxN {(expertMode ? 3U : 2U)};
    auto                    dispImgFn = [=](Vec2Fs const & iucss,ImgRgba8s const & mipmap,uint const & idx)
    {
        ImgRgba8            img = mipmap[idx];
        for (Vec2F iucs : iucss) {
            Vec2I           ircs {mapFloor(mapMul(iucs,Vec2F{img.dims()}))};
            paintCrosshair(img,ircs);
        }
        return img;
    };
    OPT<ImgRgba8>           lmsImageN = link3<ImgRgba8,Vec2Fs,ImgRgba8s,uint>(ptsIucsN,mipmapN,mipmapIdxN,dispImgFn);
    // offset of image centre from view area centre in window pixels:
    IPT<Vec2I>              offsetN {Vec2I{0}};
    // save topleft coord used by last draw so we can calculate where user has clicked:
    IPT<Vec2I>              topleftN {Vec2I{0}};   
    auto                    imgDispFn = [=](Vec2UI winSize)
    {
        uint                mipmapIdx = mipmapIdxN.val();   // don't trigger redraw by using ref()
        ImgRgba8s const &   mipmap = mipmapN.cref();
        {                   // check if image much smaller than window:
            Vec2UI              imgDims = mipmap[mipmapIdx].dims();
            Vec2F               relDims = mapDiv(Vec2F{winSize},Vec2F{imgDims});
            float               winRat = cMinElem(relDims);
            if (winRat > 2) {   // image too small so decrease mipmapIdx
                uint            steps = log2f(winRat);
                if (mipmapIdx > steps)
                    mipmapIdx -= steps;
                else
                    mipmapIdx = 0;
                mipmapIdxN.set(mipmapIdx);          // will trigger another redraw
            }
        }
        ImgRgba8 const &    dispImg = lmsImageN.cref();
        // the image can only be translated in directions in which it's larger than the window,
        // and translation is clamped to keep the window filled.
        // This allows us to avoid painting the background for image translation draws, which 
        // looks horrendous.
        Vec2I               winDims {winSize},
                            imgDims {dispImg.dims()},
                            offset = offsetN.val(),
                            topleft;
        for (uint ii=0; ii<2; ++ii) {
            int             oversize = imgDims[ii] - winDims[ii],
                            padL = oversize/2,
                            padR = oversize - padL;
            topleft[ii] = -padL;
            if (oversize > 0) {
                int             off = clamp(offset[ii],-padL,padR);
                topleft[ii] += off;
                if (off != offset[ii])
                    offsetN.ref()[ii] = off;        // will trigger another redraw
            }
        }
        topleftN.set(topleft);
        return GuiImage::Disp{&dispImg,topleft};
    };
    auto                    zoomInFn = [=]()
    {
        uint                mipmapIdx = mipmapIdxN.val();   // don't trigger redraw
        if (mipmapIdx > 0)
            mipmapIdxN.set(mipmapIdx-1);
    };
    auto                    zoomOutFn = [=]()
    {
        ++mipmapIdxN.ref();     // gets clamped by display function
    };
    IPT<int>                zoomAccN {0};
    IPT<Valid<uint>>        draggingLmN;
    auto                    dragNoneFn = [=](Vec2I winPosIrcs,Vec2I)
    {
        Vec2F                   imgDims {mipmapN.cref()[mipmapIdxN.val()].dims()};
        Vec2I                   topleft = topleftN.val();
        Vec2Fs const &          ptsIucs = ptsIucsN.cref();
        for (uint ii=0; ii<ptsIucs.size(); ++ii) {
            Vec2F               iucs = ptsIucs[ii];
            Vec2F               pacs = mapMul(iucs,imgDims);
            Vec2I               winIrcs = mapCast<int>(pacs) + topleft;
            if (cMag(winIrcs-winPosIrcs) < 17) {
                draggingLmN.ref() = ii;
                return GuiCursor::grab;
            }
        }
        draggingLmN.ref().invalidate();
        return GuiCursor::arrow;
    };
    auto                    dragRightFn = [=](Vec2I,Vec2I delta)
    {
        Vec2F               imgDims {imageN.cref().dims()};
        Vec2F               delIucs = mapDiv(Vec2F{delta},imgDims);
        Vec2Fs &            lms = ptsIucsN.ref();
        for (Vec2F & lm : lms)
            lm += delIucs;
        return GuiCursor::arrow;
    };
    auto                    shiftDragRightFn = [=](Vec2I,Vec2I delta)
    {
        float               scale = exp(delta[1]/256.0f);
        Vec2Fs &            lms = ptsIucsN.ref();
        Vec2F               mean = cMean(lms);
        for (Vec2F & lm : lms)
            lm = mean + (lm-mean) * scale;
        return GuiCursor::arrow;
    };
    auto                    clickDownLFn = [=](Vec2I)
    {
        if (draggingLmN.cref().valid())
            return GuiCursor::grab;
        else
            return GuiCursor::translate;
    };
    auto                    dragLeftFn = [=](Vec2I,Vec2I winDelta)
    {
        Valid<uint>             draggingLm = draggingLmN.val();
        if (draggingLm.valid()) {
            Vec2F               imgDims {mipmapN.cref()[mipmapIdxN.val()].dims()};
            Vec2F               deltaIucs = mapDiv(Vec2F{winDelta},imgDims);
            Vec2Fs &            ptsIucs = ptsIucsN.ref();
            ptsIucs[draggingLm.val()] += deltaIucs;
        }
        else {
            offsetN.ref() += winDelta;
        }
        return GuiCursor::arrow;
    };
    auto                    ctrlClickActionLFn = [=](Vec2I winPosIrcs)
    {
        Vec2I               topleft = topleftN.val(),
                            imgPosIrcs = winPosIrcs - topleft;
        Vec2UI              imgDims = mipmapN.cref()[mipmapIdxN.val()].dims();
        Vec2F               imgPosPacs = Vec2F{imgPosIrcs} + Vec2F{0.5f},
                            imgPosIucs = mapDiv(imgPosPacs,Vec2F{imgDims});
        if ((cMinElem(imgPosIucs)>0) && (cMaxElem(imgPosIucs)<1))
            onCtrlClick(imgPosIucs,imgDims);
    };
    GuiImage                gi;
    gi.getImgFn = imgDispFn;
    gi.wantStretch = Vec2B{true,true};
    gi.minSizeN = makeIPT(Vec2UI{100});
    gi.updateFlag = makeUpdateFlag(imageN,mipmapIdxN);
    gi.updateNofill = makeUpdateFlag(ptsIucsN,offsetN);
    gi.clickDownFns[1] = clickDownLFn;
    gi.clickDownFns[4] = [](Vec2I){return GuiCursor::crosshair; };      // right click
    gi.clickDownFns[12] = [](Vec2I){return GuiCursor::scale; };         // shift right click
    gi.mouseMoveFns[0] = dragNoneFn;
    gi.mouseMoveFns[1] = dragLeftFn;
    gi.mouseMoveFns[4] = dragRightFn;
    gi.mouseMoveFns[12] = shiftDragRightFn;
    if (onCtrlClick)
        gi.clickActionFns[6] = ctrlClickActionLFn;
    return {make_shared<GuiImage>(gi),zoomInFn,zoomOutFn};
}

GuiVal<ImgFormat>   guiImgFormatSelector(ImgFormats const & imgFormats,String8 const & store)
{
    ImgFormatsInfo      imgFormatInfo = getImgFormatsInfo();
    String8s            imgFormatDescs;     // descriptions
    for (ImgFormat fmt : imgFormats)
        imgFormatDescs.push_back(findFirst(imgFormatInfo,fmt).description);
    IPT<size_t>         imgFormatIdxN;      // user selection
    if (store.empty())
        imgFormatIdxN = makeIPT<size_t>(0);
    else
        imgFormatIdxN = makeSavedIPTEub<size_t>(0,store+"ImgFormat",imgFormats.size());
    auto                imgFormatFn = [=](size_t const & idx){return imgFormats[idx]; };
    OPT<ImgFormat>      imgFormatN = link1<size_t,ImgFormat>(imgFormatIdxN,imgFormatFn);
    GuiPtr              imgFormatSelW = guiRadio(imgFormatDescs,imgFormatIdxN);
    return {imgFormatN,imgFormatSelW};
}

}

// */
