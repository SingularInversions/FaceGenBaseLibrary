//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgGuiApiImage.hpp"
#include "FgImage.hpp"

using namespace std;


namespace Fg {

GuiPtr
guiImage(NPT<ImgRgba8> imageN)
{
    GuiImage                gi;
    gi.updateFlag = makeUpdateFlag(imageN);
    gi.updateNofill = gi.updateFlag;
    gi.wantStretch = Vec2B{false,false};
    gi.minSizeN = link1<ImgRgba8,Vec2UI>(imageN,[](ImgRgba8 const & img){return img.dims();});
    auto                    getImgFn = [imageN](Vec2UI)
    {
        return GuiImage::Disp {&imageN.cref(),Vec2I{0}};
    };
    gi.getImgFn = getImgFn;
    return guiMakePtr(gi);
}

GuiPtr
guiImage(NPT<ImgRgba8> const & imageN,Sfun<void(Vec2F)> const & onClick)
{
    GuiImage                gi;
    gi.updateFlag = makeUpdateFlag(imageN);
    gi.updateNofill = gi.updateFlag;
    gi.wantStretch = Vec2B{false,false};
    gi.minSizeN = link1<ImgRgba8,Vec2UI>(imageN,[](ImgRgba8 const & img){return img.dims();});
    auto                    clickFn = [imageN,onClick](Vec2I pos)
    {
        Vec2D           iucs = cIrcsToIucs(imageN.cref().dims()) * Vec2D(pos);
        onClick(Vec2F(iucs));
    };
    gi.clickLeft = clickFn;
    auto                    getImgFn = [imageN](Vec2UI)
    {
        return GuiImage::Disp {&imageN.cref(),Vec2I{0}};
    };
    gi.getImgFn = getImgFn;
    return guiMakePtr(gi);
}

ImgRgba8s
cMagmip(ImgRgba8 const & img,uint log2mag)
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

GuiImg
guiImageCtrls(
    NPT<ImgRgba8> const &       imageN,
    IPT<Vec2Fs> const &         ptsIucsN,
    bool                        expertMode,
    Sfun<void(Vec2F,Vec2UI)> const & onClick)
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
    auto                    dragRightFn = [=](Vec2I delta)
    {
        int                 zoomAcc = zoomAccN.val() + delta[1];
        if (abs(zoomAcc) > 40) {
            if (zoomAcc > 0)
                zoomInFn();
            else
                zoomOutFn();
            zoomAcc = 0;
        }
        zoomAccN.set(zoomAcc);
    };
    auto                    dragLeftFn = [=](Vec2I winDelta)
    {
        offsetN.ref() += winDelta;
    };
    auto                    clickLeftFn = [=](Vec2I winIrcs)
    {
        Vec2I               topleft = topleftN.val(),
                            imgIrcs = winIrcs - topleft;
        Vec2UI              imgDims = mipmapN.cref()[mipmapIdxN.val()].dims();
        Vec2F               imgIpcs = Vec2F{imgIrcs} + Vec2F{0.5f},
                            imgIucs = mapDiv(imgIpcs,Vec2F{imgDims});
        if ((cMinElem(imgIucs)>0) && (cMaxElem(imgIucs)<1))
            onClick(imgIucs,imgDims);
    };
    GuiImage                gi;
    gi.getImgFn = imgDispFn;
    gi.wantStretch = Vec2B{true,true};
    gi.minSizeN = makeIPT(Vec2UI{100});
    gi.updateFlag = makeUpdateFlag(imageN,mipmapIdxN);
    gi.updateNofill = makeUpdateFlag(ptsIucsN,offsetN);
    gi.dragLeft = dragLeftFn;
    gi.dragRight = dragRightFn;
    if (onClick)
        gi.clickLeft = clickLeftFn;
    return {guiMakePtr(gi),zoomInFn,zoomOutFn};
}

}

// */
