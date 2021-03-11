//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt

#ifndef FGGUIAPIIMAGE_HPP
#define FGGUIAPIIMAGE_HPP

#include "FgStdExtensions.hpp"
#include "FgGuiApiBase.hpp"
#include "FgImageBase.hpp"

namespace Fg {

struct  GuiImageDisp
{
    uint                width;
    uint                height;
    RgbaUC const *      dataPtr;
    int                 offsetx;
    int                 offsety;
};

// This function must be defined in the corresponding OS-specific implementation:
struct  GuiImage;
GuiImplPtr guiGetOsImpl(GuiImage const & guiApi);

struct  GuiImage : GuiBase
{
    DfgFPtr             updateFill;
    NPT<ImgC4UC>        imgN;
    // If defined, this is called when the user clicks on the image.
    // The argument is the coordinate in IUCS (cannot be in pixels because view scale varies):
    Sfun<void(Vec2F)>   onClick;
    // If mouse controls are disabled the image is treated as a thumbnail; it's minimum
    // window size is given by the image and is non-adjustable:
    bool                allowMouseCtls = false; // The below are only used when true:
    DfgFPtr             updateNofill;
    NPT<ImgC4UCs>       pyramidN;           // Powers of 2 views from smallest to 2048 max dim
    IPT<Vec2I>          offsetN;            // In pixels (regardless of pyramid level)
    IPT<int>            zoomN;
    IPT<uint>           currLevelN;         // Which level of pyramid are we looking at ? 0 - uninitialized
    // User-selected points in IUCS. NB These are NOT corrected for non-power-of-2 pixel truncation:
    IPT<Vec2Fs>         pointsN;           
    NPT<ImgC4UC>        dispN;              // Final display image including marked points

    virtual
    GuiImplPtr getInstance() {return guiGetOsImpl(*this); }

    GuiImageDisp
    disp(Vec2UI winSize);

    void
    move(Vec2I delta);          // in pixels

    void
    zoom(int delta);            // in pixels

    void
    click(Vec2I posIrcs);       // Position in display window

    // Sets the update flag to false and returns the currently required update state:
    // 0 - unchanged, 1 - changed but same area, 2 - size may have changed
    uint
    update();
};

// Thumbnail image of fixed size with no controls:
GuiPtr
guiImage(NPT<ImgC4UC> imgN);

// Thumbnail image that can be clicked on for custom action:
GuiPtr
guiImage(NPT<ImgC4UC> const & imgN,Sfun<void(Vec2F)> const & onClick);

// Zoomable, scrollable, clickable image of variable size:
GuiPtr
guiImage(
    NPT<ImgC4UC> const &        imgN,           // input
    IPT<Vec2Fs> const &         ptsIucsN,       // output: user-selected points
    Sfun<void(Vec2F)> const &   onClick=nullptr);

}

#endif
