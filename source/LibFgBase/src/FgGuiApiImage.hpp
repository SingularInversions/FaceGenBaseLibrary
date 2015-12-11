//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     May 6, 2011
//

#ifndef FGGUIAPIIMAGE_HPP
#define FGGUIAPIIMAGE_HPP

#include "FgGuiApiBase.hpp"
#include "FgImageBase.hpp"

struct  FgGuiImageDisp
{
    uint        width;
    uint        height;
    const FgRgbaUB * dataPtr;
    int         offsetx;
    int         offsety;
};

struct  FgGuiApiImage : public FgGuiApi<FgGuiApiImage>
{
    uint                    updateNodeIdxFill;
    FgDgn<FgImgRgbaUb>      imgN;
    // If mouse controls are disabled the image is treated as a thumbnail; it's minimum
    // window size is given by the image and is non-adjustable:
    bool                    allowMouseCtls;     // The below are only used when true:
    uint                    updateNodeIdxNofill;
    FgDgn<vector<FgImgRgbaUb> > pyramidN;       // Powers of 2 views up to 2048 max dim
    FgDgn<FgVect2I>         offsetN;            // In pixels (regardless of pyramid level)
    FgDgn<int>              zoomN;
    FgDgn<uint>             currLevelN;         // Which level of pyramid are we looking at ? 0 - uninitialized
    // User-selected points in IUCS. NB These are NOT corrected for non-power-of-2 pixel truncation:
    FgDgn<vector<FgVect2F> > pointsN;           
    FgDgn<FgImgRgbaUb>      dispN;              // Final display image including marked points
    boost::function<void()> onClick;            // if non-NULL, call this after user-selected point click

    FgGuiImageDisp
    disp(FgVect2UI winSize);

    void
    move(FgVect2I delta);       // in pixels

    void
    zoom(int delta);            // in pixels

    void
    click(FgVect2I pos);        // in pixels of entire view area

    // Sets the update flag to false and returns the currently required update state:
    // 0 - unchanged, 1 - changed but same area, 2 - size may have changed
    uint
    update();
};

// Thumbnail image of fixed size with no controls:
FgGuiPtr
fgGuiImage(FgDgn<FgImgRgbaUb> imgN);

inline
FgGuiPtr
fgGuiImage(const FgImgRgbaUb & img)
{return fgGuiImage(g_gg.addNode(img)); }

// Zoomable, scrollable, clickable image of variable size:
FgGuiPtr
fgGuiImage(
    FgDgn<FgImgRgbaUb>          imgN,           // input
    FgDgn<vector<FgVect2F> >    ptsIucsN,       // output: user-selected points
    boost::function<void()>     onClick=NULL);

#endif
