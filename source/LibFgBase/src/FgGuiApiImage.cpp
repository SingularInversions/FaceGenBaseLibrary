//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
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
        Vec2D           iucs = cIrcsToIucsXf(imageN.cref().dims()) * Vec2D(pos);
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

GuiImg
guiImageCtrls(
    NPT<ImgRgba8> const &        imageN,
    IPT<Vec2Fs> const &         ptsIucsN,
    Sfun<void(Vec2F)> const &   onClick)
{
    IPT<Vec2I>              offsetN = makeIPT(Vec2I());     // In pixels (regardless of pyramid level)
    // Which level of pyramid are we looking at ? 0 - uninitialized:
    IPT<uint>               currLevelN = makeIPT(uint(0));
    auto                    pyramidFn = [](ImgRgba8 const & img,ImgRgba8s & pyr)
    {
        if (img.empty()) {
            pyr.clear();
            return;
        }
        pyr.resize(log2Floor(cMinElem(img.dims()))+1);
        pyr.back() = img;
        for (uint ii=0; ii<pyr.size()-1; ++ii)
            shrink2_(pyr[pyr.size()-1-ii],pyr[pyr.size()-2-ii]);
        // Add up to 4x expansion for small images:
        for (size_t ii=0; ii<2; ++ii)
            if (cMaxElem(pyr.back().dims()) <= 1024)    // Create up to 2048
                pyr.push_back(expand2(pyr.back()));
    };
    // Powers of 2 views from smallest to 2048 max dim
    NPT<ImgRgba8s>           pyramidN = link1_<ImgRgba8,ImgRgba8s>(imageN,pyramidFn);
    // Final display image including marked points:
    auto                    dispFn = [](ImgRgba8s const & pyr,const uint & levIn,Vec2Fs const & pts,ImgRgba8 & img)
    {
        if (pyr.empty()) {
            img = ImgRgba8();
            return;
        }
        uint                        maxLev = uint(pyr.size())-1,
                                    lev = levIn;
        if (lev > maxLev)
            lev = maxLev;
        img = pyr[lev];
        for (size_t ii=0; ii<pts.size(); ++ii){
            int     xx = int(pts[ii][0] * img.width()),
                    yy = int(pts[ii][1] * img.height());
            paintCrosshair(img,Vec2I{xx,yy});
        }
    };
    NPT<ImgRgba8>            dispN = link3_<ImgRgba8,ImgRgba8s,uint,Vec2Fs>(pyramidN,currLevelN,ptsIucsN,dispFn);
    IPT<int>                zoomN = makeIPT(0);
    auto                    imgDispFn = [currLevelN,pyramidN,dispN,offsetN](Vec2UI winSize)
    {
        GuiImage::Disp          ret;
        if (winSize[0]*winSize[1] > 0) {
            // Ensure level has been initialized using window size so dispN will be correctly calculated:
            uint                    level = currLevelN.val();
            if (level == 0) {    // not yet initialized:
                ImgRgba8s const & pyr = pyramidN.cref();
                while (
                    (level+1 < pyr.size()) &&
                    (pyr[level].width() < winSize[0]) &&
                    (pyr[level].height() < winSize[1]))
                        ++level;
                currLevelN.set(level);
            }
            // Adjust offset here to be able to respond properly to window resize.
            // Image can only be moved if it's larger than display window, and movement is clamped
            // to keep image boundaries at or outside window boundaries.
            ImgRgba8 const & img = dispN.cref();
            Vec2I            offset = offsetN.val();
            Vec2I            delsz = Vec2I(winSize) - Vec2I(img.dims());
            if (delsz[0] < 0) {
                if (offset[0] < delsz[0]) offset[0] = delsz[0];
                if (offset[0] > 0) offset[0] = 0;
            }
            else
                offset[0] = delsz[0] / 2;       // If window bigger than image, centre image.
            if (delsz[1] < 0) {
                if (offset[1] < delsz[1]) offset[1] = delsz[1];
                if (offset[1] > 0) offset[1] = 0;
            }
            else
                offset[1] = delsz[1] / 2;
            offsetN.set(offset);
            offsetN.val();                  // Clean node; don't trigger future repaints
            ret.imgPtr = &img;
            ret.offset = offset;
        }
        return ret;
    };
    auto                    zoomInFn = [pyramidN,currLevelN]()
    {
        ImgRgba8s const &    pyr = pyramidN.cref();
        uint                level = currLevelN.val();
        if (level < pyr.size()-1)
            currLevelN.set(level+1);
    };
    auto                    zoomOutFn = [currLevelN]()
    {
        uint                level = currLevelN.val();
        if (level > 7)
            currLevelN.set(level-1);
    };
    auto                    dragRightFn = [zoomN,zoomInFn,zoomOutFn](Vec2I delta)
    {
        int                 zoom = zoomN.val() + delta[1];
        if (abs(zoom) > 30) {
            if (zoom > 0)
                zoomInFn();
            else
                zoomOutFn();
            zoom = 0;
        }
        zoomN.set(zoom);
    };
    auto                    clickLeftFn = [onClick,offsetN,pyramidN,currLevelN](Vec2I pos)
    {
        uint                lev = currLevelN.val();
        ImgRgba8 const &     img = pyramidN.cref()[lev];
        Vec2I               imgPos = pos - offsetN.val();
        if (isInBounds(Mat22I(0,img.width(),0,img.height()),imgPos)) {
            Vec2D           iucs = cIrcsToIucsXf(img.dims()) * Vec2D(imgPos);
            onClick(Vec2F(iucs));
        }
    };
    GuiImage                gi;
    gi.getImgFn = imgDispFn;
    gi.wantStretch = Vec2B{true,true};
    gi.minSizeN = makeIPT(Vec2UI{100});
    gi.updateFlag = makeUpdateFlag(imageN,currLevelN);
    // offset never needs background repaint since only images that more than cover the view area
    // can be translated. Image smaller than the view area are fixed in the centre:
    gi.updateNofill = makeUpdateFlag(offsetN,ptsIucsN);
    gi.dragLeft = [offsetN](Vec2I delta) {offsetN.ref() += delta; };
    gi.dragRight = dragRightFn;
    if (onClick)
        gi.clickLeft = clickLeftFn;
    return {guiMakePtr(gi),zoomInFn,zoomOutFn};
}

// PREVIOUS REFACTOR ATTEMPT:
    //IPT<Vec2F>              transIucsN {Vec2F{0}};          // Image centre relative to view area centre
    // Returns the display window raster coordinate for the upper left image pixel:
    //auto                    getOffsetFn = [](uint imgDim,uint winDim,float lastIucs)
    //{
    //    // No range of motion when image size is <= window size:
    //    int                     rangeTotal = max(int(imgDim)-int(winDim),0),
    //                            rangeHi = rangeTotal / 2,
    //                            rangeLo = rangeHi - rangeTotal,
    //                            lastIrcs = round<int>(lastIucs * imgDim);
    //    return clamp(lastIrcs,rangeLo,rangeHi);
    //};
    // Returns the updated IUCS image offset when a user attempts to move the image:
    //auto                    updateIucsFn = [](uint imgDim,uint winDim,float lastIucs,int userDelta)
    //{
    //    int                     rangeTotal = max(int(imgDim)-int(winDim),0),        // As above
    //                            rangeHi = rangeTotal / 2,                           // "
    //                            rangeLo = rangeHi - rangeTotal,                     // "
    //                            lastIrcs = round<int>(lastIucs * imgDim),           // "
    //                            currIrcs = clamp(lastIrcs,rangeLo,rangeHi),
    //                            nextIrcs = clamp(currIrcs-userDelta,rangeLo,rangeHi);
    //    return float(nextIrcs) / float(imgDim);
    //};
    //auto                    dragLeftFn = [=](Vec2I delta)
    //{
    //    Vec2UI                  imgDims = pyramidN.cref()[currLevelN.val()].dims();
    //    Vec2UI                  winDims = dispSizeN.val();
    //    Vec2F &                 iucs = transIucsN.ref();
    //    for (uint ii=0; ii<2; ++ii)
    //        iucs[ii] = updateIucsFn(imgDims[ii],winDims[ii],iucs[ii],delta[ii]);
    //};
    //auto                    offsetFn = [=](ImgRgba8s const & pyr,uint const & sl,Vec2UI const & winDims,Vec2F const & iucs)
    //{
    //    Vec2UI              imgDims = pyr[sl].dims();
    //    Vec2I               offset;
    //    for (uint ii=0; ii<2; ++ii)
    //        offset[ii] = getOffsetFn(imgDims[ii],winDims[ii],iucs[ii]);
    //    return offset;
    //};

}

// */
