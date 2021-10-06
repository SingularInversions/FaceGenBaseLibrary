//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Drawing and 2D graphics functions
//

#ifndef FGDRAW_HPP
#define FGDRAW_HPP

#include "FgStdLibs.hpp"
#include "FgImage.hpp"

namespace Fg {

// Simple (aliased) filled circle. Radius 0 yields a single pixel (ie. actual radius is 0.5 larger):
void        drawDotIrcs(ImgRgba8 & img,Vec2I posIrcs,uint radius,Rgba8 color);
void        drawDotIucs(ImgRgba8 & img,Vec2D posIucs,uint radius,Rgba8 color);
void        drawLineIrcs(ImgRgba8 & img,Vec2I begin,Vec2I end,Rgba8 color);
void        drawSolidRectangle_(ImgRgba8 & img,Vec2UI posIrcs,Vec2UI szPixels,Rgba8 clr);

// Square bar graph of one or more data series:
ImgRgba8     cBarGraph(Doubless const & data,uint pixPerBar,uint pixSpacing=0);
ImgRgba8     cBarGraph(Sizes const & data);

void
drawFunction(
    ImgRgba8 &               img,            // OUTPUT
    Sfun<double(double)>    func,
    VecD2                   bounds,         // abscissa value at image x bounds
    double                  vscale,         // y pixels per unit
    Rgba8                  colour);

void        drawFunctions(MatD const & funcs);  // Columns are function values

}

#endif
