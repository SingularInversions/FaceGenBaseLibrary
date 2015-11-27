//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: Dec 2, 2013
//

#include "stdafx.h"

#include "FgGuiApiImage.hpp"
#include "FgImage.hpp"

using namespace std;

FgGuiImageDisp
FgGuiApiImage::disp(FgVect2UI winSize)
{
    FGASSERT(winSize[0]*winSize[1] > 0);
    FgGuiImageDisp  ret;
    if (allowMouseCtls) {
        // Ensure level has been initialized using window size so dispN will be correctly calculated:
        uint                        level = g_gg.getVal(currLevelN);
        if (level == 0) {    // not yet initialized:
            const vector<FgImgRgbaUb> & pyr = g_gg.getVal(pyramidN);
            while (
                (level+1 < pyr.size()) &&
                (pyr[level].width() < winSize[0]) &&
                (pyr[level].height() < winSize[1]))
                    ++level;
            g_gg.setVal(currLevelN,level);
        }
        // Adjust offset here to be able to respond properly to window resize.
        // Image can only be moved if it's larger than display window, and movement is clamped
        // to keep image boundaries at or outside window boundaries.
        const FgImgRgbaUb & img = g_gg.getVal(dispN);
        FgVect2I            offset = g_gg.getVal(offsetN);
        FgVect2I            delsz = FgVect2I(winSize) - FgVect2I(img.dims());
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
        g_gg.setVal(offsetN,offset);
        g_gg.getVal(offsetN);           // Clean node; don't trigger future repaints

        ret.width = img.width();
        ret.height = img.height();
        ret.dataPtr = img.dataPtr();
        ret.offsetx = offset[0];
        ret.offsety = offset[1];
    }
    else {
        const FgImgRgbaUb & img = g_gg.getVal(imgN);
        FgVect2I    pad = FgVect2I(winSize) - FgVect2I(img.dims());
        ret.width = img.width();
        ret.height = img.height();
        ret.dataPtr = img.dataPtr();
        ret.offsetx = (pad[0] > 0 ? pad[0]/2 : 0);
        ret.offsety = (pad[1] > 0 ? pad[1]/2 : 0);
    }
    return ret;
}

void
FgGuiApiImage::move(FgVect2I delta)
{
    if (allowMouseCtls) {
        FgVect2I    offset = g_gg.getVal(offsetN) + delta;
        g_gg.setVal(offsetN,offset);
    }
}

void
FgGuiApiImage::zoom(int delta)
{
    if (!allowMouseCtls)
        return;
    int     zoom = g_gg.getVal(zoomN);
    zoom += delta;
    if (zoom > 30) {
        zoom = 0;
        const vector<FgImgRgbaUb> &     pyr = g_gg.getVal(pyramidN);
        uint    level = g_gg.getVal(currLevelN);
        if (level < pyr.size()-1)
            g_gg.setVal(currLevelN,level+1);
    }
    if (zoom < -30) {
        zoom = 0;
        uint    level = g_gg.getVal(currLevelN);
        if (level > 7)
            g_gg.setVal(currLevelN,level-1);
    }
    g_gg.setVal(zoomN,zoom);
}

void
FgGuiApiImage::click(FgVect2I pos)
{
    if (!allowMouseCtls)
        return;
    uint                lev = g_gg.getVal(currLevelN);
    const FgImgRgbaUb & img = g_gg.getVal(pyramidN)[lev];
    FgVect2I            imgPos = pos - g_gg.getVal(offsetN);
    if ((imgPos[0] >= 0) && (imgPos[0] < int(img.width())) &&
        (imgPos[1] >= 0) && (imgPos[1] < int(img.height()))) {
        FgVect2F    posr = FgVect2F(imgPos) + FgVect2F(0.5f),
                    posIucs(posr[0]/float(img.width()),posr[1]/float(img.height()));
        vector<FgVect2F> &  points = g_gg.getRef(pointsN);
        points.push_back(posIucs);
        if (onClick != NULL)
            onClick();
    }
}

uint
FgGuiApiImage::update()
{
    if (g_gg.dg.update(updateNodeIdxFill)) {
        if (allowMouseCtls)
            g_gg.dg.update(updateNodeIdxNofill);
        return 2;
    }
    if (allowMouseCtls && g_gg.dg.update(updateNodeIdxNofill)) {
        // Avoid flickering due to background repaint if image size hasn't changed:
        return 1;
    }
    return 0;
}

static
FGLINK(linkPyramid)
{
    FGLINKARGS(1,1);
    FgImgRgbaUb             img = inputs[0]->valueRef();
    vector<FgImgRgbaUb> &   pyr = outputs[0]->valueRef();
    if (img.empty()) {
        pyr.clear();
        return;
    }
    const uint              maxDim = 2048;
    while (fgMaxElem(img.dims()) > maxDim)
        img = fgImgShrink2(img);
    pyr.resize(fgLog2Floor(fgMinElem(img.dims()))+1);
    pyr.back() = img;
    for (uint ii=0; ii<pyr.size()-1; ++ii)
        fgImgShrink2(pyr[pyr.size()-1-ii],pyr[pyr.size()-2-ii]);
    // Add up to 4x expansion for small images:
    for (size_t ii=0; ii<2; ++ii)
        if (fgMaxElem(pyr.back().dims()) <= maxDim/2)
            pyr.push_back(fgExpand2(pyr.back()));
}

static
FGLINK(linkDisp)
{
    FGLINKARGS(3,1);
    const vector<FgImgRgbaUb> & pyr = inputs[0]->valueRef();
    uint                        lev = inputs[1]->valueRef();
    const vector<FgVect2F> &    pts = inputs[2]->valueRef();
    FgImgRgbaUb &               img = outputs[0]->valueRef();
    img = pyr[lev];
    for (size_t ii=0; ii<pts.size(); ++ii){
        int     xx = int(pts[ii][0] * img.width()),
                yy = int(pts[ii][1] * img.height());
        FgRgbaUB    green(0,255,0,255);
        for (int jj=-4; jj<5; ++jj) {
            for (int kk=-1; kk<2; ++kk) {
                img.paint(xx+jj,yy+kk,green);
                img.paint(xx+kk,yy+jj,green);
            }
        }
    }
}

FgGuiPtr
fgGuiImage(FgDgn<FgImgRgbaUb> imgN)
{
    FgGuiApiImage       gai;
    gai.imgN = imgN;
    gai.updateNodeIdxFill = g_gg.addUpdateFlag(imgN);
    gai.allowMouseCtls = false;
    return fgGuiPtr(gai);
}

FgGuiPtr
fgGuiImage(FgDgn<FgImgRgbaUb> imgN,FgDgn<vector<FgVect2F> > ptsIucsN,boost::function<void()> onClick)
{
    FgGuiApiImage       gai;
    gai.imgN = imgN;
    gai.allowMouseCtls = true;
    gai.offsetN = g_gg.addNode(FgVect2I());
    gai.zoomN = g_gg.addNode(0);
    gai.currLevelN = g_gg.addNode(uint(0));
    gai.pyramidN = g_gg.addNode(vector<FgImgRgbaUb>());
    gai.pointsN = ptsIucsN;
    gai.dispN = g_gg.addNode(FgImgRgbaUb());
    gai.onClick = onClick;
    g_gg.addLink(linkPyramid,gai.imgN,gai.pyramidN);
    g_gg.addLink(linkDisp,fgUints(gai.pyramidN,gai.currLevelN,gai.pointsN),gai.dispN);
    gai.updateNodeIdxFill = g_gg.addUpdateFlag(fgUints(gai.imgN,gai.currLevelN));
    gai.updateNodeIdxNofill = g_gg.addUpdateFlag(fgUints(gai.offsetN,gai.pointsN));
    return fgGuiPtr(gai);
}

// */
