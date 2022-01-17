//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt

#ifndef FGGUIAPIIMAGE_HPP
#define FGGUIAPIIMAGE_HPP

#include "FgStdExtensions.hpp"
#include "FgGuiApiBase.hpp"
#include "FgImageBase.hpp"

namespace Fg {

// This function must be defined in the corresponding OS-specific implementation:
struct  GuiImage;
GuiImplPtr              guiGetOsImpl(GuiImage const & guiApi);

struct  GuiImage : GuiBase
{
    struct  Disp
    {
        ImgRgba8 const *    imgPtr;
        Vec2I               offset;         // from top left of window
    };
    // Callback when image is needed for bitblt to screen.
    // Output node with image data is not sufficient since user controls input state (eg. zoom & offset)
    // need be modified when the window size changes:
    Sfun<Disp(Vec2UI)>  getImgFn;           // Input argument is display win dims in pixels
    Vec2B               wantStretch;
    NPT<Vec2UI>         minSizeN;           // Minimum display window size (in pixels)
    DfgFPtr             updateFlag;         // Display update flag when background should be filled
    DfgFPtr             updateNofill;       // Display update flag when only image pixels need to be updated

    // USER ACTION CALLBACKS:
    Sfun<void(Vec2I)>   dragLeft;           // Argument is delta in pixels
    Sfun<void(Vec2I)>   dragRight;          // "
    Sfun<void(Vec2I)>   clickLeft;          // Argument is display position in pixels

    virtual             GuiImplPtr getInstance() {return guiGetOsImpl(*this); }
};

// Fixed size image with no controls:
GuiPtr          guiImage(NPT<ImgRgba8> imageN);

// Fixed size image that can be clicked on for custom action:
GuiPtr
guiImage(
    NPT<ImgRgba8> const &        imageN,
    Sfun<void(Vec2F)> const &   onClick);   // Arg is IUCS image coordinate of click

struct  GuiImg
{
    GuiPtr              win;
    Sfun<void(void)>    zoomIn;
    Sfun<void(void)>    zoomOut;
};
// Zoom-able, shift-able, click-able image of variable size:
GuiImg
guiImageCtrls(
    NPT<ImgRgba8> const &       imageN,             // source image for display
    // User-selected points in IUCS will be overlaid on image.
    // NB These are NOT corrected for non-power-of-2 pixel truncation:
    IPT<Vec2Fs> const &         ptsIucsN,
    // if true, image zoom goes up to 8x for <=2K images.
    // if false, image zoom goes up to 4x for <=2K images.
    bool                        expertMode=false,
    // If defined, this is called when the user clicks on the image.
    // The first argument is the coordinate in IUCS. The second argument are the image dimensions at the
    // current viewing zoom level. The latter can be useful for determining accidental near double-clicks.
    // Note that unless the image is a power of 2 in size, small discrepencies exist between the coord
    // in different SLs, but they are sub-pixel:
    Sfun<void(Vec2F,Vec2UI)> const & onClick=nullptr);

}

#endif
