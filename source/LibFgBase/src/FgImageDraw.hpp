//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Drawing and 2D graphics functions
//

#ifndef FGDRAW_HPP
#define FGDRAW_HPP

#include "FgSerial.hpp"
#include "FgImage.hpp"

namespace Fg {

Arr<Arr3F,6>        cColors6();
Arr<Rgba8,6>        cColors6Rgba();     // return the 6 colors composed of either 1 or 2 channels max others zero.

// Simple (aliased) filled circle. Radius 0 yields a single pixel (ie. actual radius is 0.5 larger):
void                drawDotIrcs(ImgRgba8 & img,Vec2I posIrcs,uint radius,Rgba8 color);
void                drawLineIrcs(ImgRgba8 & img,Vec2I begin,Vec2I end,Rgba8 color);
void                drawSolidRectangle_(ImgRgba8 & img,Vec2UI posIrcs,Vec2UI szPixels,Rgba8 clr);

// Square bar graph of one or more data series, each series colored in order R,G,B, ... repeat
// data series can be different lengths but are assumed to start at same point and be equispaced
ImgRgba8            cBarGraph(Doubless data,uint pixPerBar,uint pixSpacing=0);
ImgRgba8            cBarGraph(Sizes const & data);

ImgRgba8            cLineGraph(
    MatD const &            data,           // rows are measures, cols are series to be graphed
    uint                    pixPerPnt,      // abscissa pixels per series element
    Strings const &         labels={});     // if non-empty must be 1-1 with data rows. Print label-color guide to CL

}

#endif
