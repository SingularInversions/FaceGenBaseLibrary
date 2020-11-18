//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgGuiApiImage.hpp"
#include "FgImage.hpp"

using namespace std;


namespace Fg {

GuiImageDisp
GuiImage::disp(Vec2UI winSize)
{
    FGASSERT(winSize[0]*winSize[1] > 0);
    GuiImageDisp  ret;
    if (allowMouseCtls) {
        // Ensure level has been initialized using window size so dispN will be correctly calculated:
        uint                    level = currLevelN.val();
        if (level == 0) {    // not yet initialized:
            ImgC4UCs const & pyr = pyramidN.cref();
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
        ImgC4UC const & img = dispN.cref();
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

        ret.width = img.width();
        ret.height = img.height();
        ret.dataPtr = img.data();
        ret.offsetx = offset[0];
        ret.offsety = offset[1];
    }
    else {
        ImgC4UC const & img = imgN.cref();
        Vec2I    pad = Vec2I(winSize) - Vec2I(img.dims());
        ret.width = img.width();
        ret.height = img.height();
        ret.dataPtr = img.data();
        ret.offsetx = (pad[0] > 0 ? pad[0]/2 : 0);
        ret.offsety = (pad[1] > 0 ? pad[1]/2 : 0);
    }
    return ret;
}

void
GuiImage::move(Vec2I delta)
{
    if (allowMouseCtls) {
        Vec2I    offset = offsetN.val() + delta;
        offsetN.set(offset);
    }
}

void
GuiImage::zoom(int delta)
{
    if (!allowMouseCtls)
        return;
    int     zoom = zoomN.val();
    zoom += delta;
    if (zoom > 30) {
        zoom = 0;
        ImgC4UCs const &     pyr = pyramidN.cref();
        uint    level = currLevelN.val();
        if (level < pyr.size()-1)
            currLevelN.set(level+1);
    }
    if (zoom < -30) {
        zoom = 0;
        uint    level = currLevelN.val();
        if (level > 7)
            currLevelN.set(level-1);
    }
    zoomN.set(zoom);
}

void
GuiImage::click(Vec2I pos)
{
    if (onClick) {
        if (allowMouseCtls) {
            uint                lev = currLevelN.val();
            ImgC4UC const &     img = pyramidN.cref()[lev];
            Vec2I               imgPos = pos - offsetN.val();
            if (isInBounds(Mat22I(0,img.width(),0,img.height()),imgPos)) {
                Vec2F           iucs = cIrcsToIucsXf(img.dims()) * Vec2F(imgPos);
                onClick(iucs);
            }
        }
        else {
            Vec2F           iucs = cIrcsToIucsXf(imgN.cref().dims()) * Vec2F(pos);
            onClick(iucs);
        }
    }
}

uint
GuiImage::update()
{
    if (updateFill->checkUpdate()) {
        if (allowMouseCtls)
            updateNofill->checkUpdate();
        return 2;
    }
    if (allowMouseCtls && updateNofill->checkUpdate()) {
        // Avoid flickering due to background repaint if image size hasn't changed:
        return 1;
    }
    return 0;
}

static
void
linkPyramid2(ImgC4UC const & img,ImgC4UCs & pyr)
{
    if (img.empty()) {
        pyr.clear();
        return;
    }
    pyr.resize(log2Floor(cMinElem(img.dims()))+1);
    pyr.back() = img;
    for (uint ii=0; ii<pyr.size()-1; ++ii)
        imgShrink2(pyr[pyr.size()-1-ii],pyr[pyr.size()-2-ii]);
    // Add up to 4x expansion for small images:
    for (size_t ii=0; ii<2; ++ii)
        if (cMaxElem(pyr.back().dims()) <= 1024)    // Create up to 2048
            pyr.push_back(fgExpand2(pyr.back()));
}

static
void
linkDisp2(ImgC4UCs const & pyr,const uint & levIn,Vec2Fs const & pts,ImgC4UC & img)
{
    if (pyr.empty()) {
        img = ImgC4UC();
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
}

GuiPtr
guiImage(NPT<ImgC4UC> imgN)
{
    GuiImage       gai;
    gai.imgN = imgN;
    gai.updateFill = makeUpdateFlag(imgN);
    return guiMakePtr(gai);
}

GuiPtr
guiImage(NPT<ImgC4UC> const & imgN,Sfun<void(Vec2F)> const & onClick)
{
    GuiImage                gai;
    gai.imgN = imgN;
    gai.updateFill = makeUpdateFlag(imgN);
    gai.onClick = onClick;
    return guiMakePtr(gai);
}

GuiPtr
guiImage(
    NPT<ImgC4UC> const &        imgN,
    IPT<Vec2Fs> const &         ptsIucsN,
    Sfun<void(Vec2F)> const &   onClick)
{
    GuiImage                gai;
    gai.imgN = imgN;
    gai.allowMouseCtls = true;
    gai.offsetN = makeIPT(Vec2I());
    gai.zoomN = makeIPT(0);
    gai.currLevelN = makeIPT(uint(0));
    gai.pyramidN = link1_<ImgC4UC,ImgC4UCs>(gai.imgN,linkPyramid2);
    gai.pointsN = ptsIucsN;
    gai.dispN = link3_<ImgC4UC,ImgC4UCs,uint,Vec2Fs>(gai.pyramidN,gai.currLevelN,gai.pointsN,linkDisp2);
    if (onClick)
        gai.onClick = onClick;
    gai.updateFill = makeUpdateFlag(gai.imgN,gai.currLevelN);
    gai.updateNofill = makeUpdateFlag(gai.offsetN,gai.pointsN);
    return guiMakePtr(gai);
}

}

// */
