//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
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

// Simple (aliased) filled circle. Radius 0 yields a single pixel:
template<class T>
void                drawDotIrcs(Img<T> & img,Vec2I centreIrcs,uint radius,T color)
{
    int                 rad = scast<int>(radius),
                        radSqr = sqr(rad);
    Vec2I               lo = mapMax(centreIrcs - Vec2I{rad},Vec2I{0}),
                        hi = mapMin(centreIrcs + Vec2I{rad},Vec2I{img.dims()});
    for (int yy=lo[1]; yy<=hi[1]; ++yy)
        for (int xx=lo[0]; xx<=hi[0]; ++xx)
            if (cMag(Vec2I{xx,yy}-centreIrcs) <= radSqr)
                img.xy(xx,yy) = color;
}
void                drawLineIrcs(ImgRgba8 & img,Vec2I begin,Vec2I end,Rgba8 color);
void                drawSolidRectangle_(ImgRgba8 & img,Vec2UI posIrcs,Vec2UI szPixels,Rgba8 clr);

// Square bar graph of one or more data series, each series colored in order R,G,B, ... repeat
// data series can be different lengths but are assumed to start at same point and be equispaced
ImgRgba8            cBarGraph(Doubless data,uint pixPerBar=3,uint pixSpacing=1);
ImgRgba8            cBarGraph(Sizes const & data);
ImgRgba8            cGraphFunctions(Svec<Sfun<double(double)>> const & fns,Arr2D boundsX);
Img3F               cScatterPlot(Arr2Ds const & data);
ImgRgba8            cLineGraph(
    MatD const &            data,           // rows are measures, cols are series to be graphed
    uint                    pixPerPnt,      // abscissa pixels per series element
    Strings const &         labels={});     // if non-empty must be 1-1 with data rows. Print label-color guide to CL

}

#endif
