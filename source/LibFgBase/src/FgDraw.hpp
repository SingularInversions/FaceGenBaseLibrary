//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Feb 12, 2009
//
// Drawing and 2D graphics functions
//

#ifndef FGDRAW_HPP
#define FGDRAW_HPP

#include "FgStdLibs.hpp"

#include "FgImage.hpp"

void
fgDrawDotIrcs(
    FgImgRgbaUb &       img,
    FgVect2I            pos,
    int                 radius,
    FgRgbaUB val);

void
fgDrawLineIrcs(
    FgImgRgbaUb &       img,
    FgVect2I            beg,
    FgVect2I            end,
    FgRgbaUB            val);

void
fgPaintRectangle(
    FgImgRgbaUb &       img,
    FgRgbaUB            clr,
    size_t xIrcs,   size_t yIrcs,
    size_t wid,     size_t hgt);

// Very simple single-pixel-wide-bars bar graph:
double
fgDrawBarGraph(
    FgImgRgbaUb &               img,
    const std::vector<double> & data,
    FgRgbaUB                    colour=FgRgbaUB(255,255,255,255));

template<typename T>
FgImgRgbaUb
fgGraph(const std::vector<T> & data)
{
    FgImgRgbaUb         img;
    std::vector<double> dt;
    fgCast_(data,dt);
    fgDrawBarGraph(img,dt);
    return img;
}

void
fgDrawFunction(
    FgImgRgbaUb &       img,            // OUTPUT
    boost::function<double(double)> func,
    FgVectD2            bounds,         // abscissa value at image x bounds
    double              vscale,         // y pixels per unit
    FgRgbaUB            colour);

void
fgDrawFunctions(
    const FgMatrixD &   funcs);     // Columns are function values


#endif
