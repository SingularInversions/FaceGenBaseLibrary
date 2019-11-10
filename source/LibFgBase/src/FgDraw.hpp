//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
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

void
fgDrawDotIrcs(
    ImgC4UC &       img,
    Vec2I            pos,
    int                 radius,
    RgbaUC val);

void
fgDrawLineIrcs(
    ImgC4UC &       img,
    Vec2I            beg,
    Vec2I            end,
    RgbaUC            val);

void
fgPaintRectangle(
    ImgC4UC &       img,
    RgbaUC            clr,
    size_t xIrcs,   size_t yIrcs,
    size_t wid,     size_t hgt);

// Very simple single-pixel-wide-bars bar graph:
double
fgDrawBarGraph(
    ImgC4UC &               img,
    const Svec<double> & data,
    RgbaUC                    colour=RgbaUC(255,255,255,255));

template<typename T>
ImgC4UC
fgGraph(const Svec<T> & data)
{
    ImgC4UC         img;
    Svec<double> dt;
    scast_(data,dt);
    fgDrawBarGraph(img,dt);
    return img;
}

void
fgDrawFunction(
    ImgC4UC &       img,            // OUTPUT
    std::function<double(double)> func,
    VecD2            bounds,         // abscissa value at image x bounds
    double              vscale,         // y pixels per unit
    RgbaUC            colour);

void
fgDrawFunctions(
    const MatD &   funcs);     // Columns are function values

}

#endif
