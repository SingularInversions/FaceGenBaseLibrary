//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     April 6, 2010
//
// Adaptive image sampler

#ifndef FG_SAMPLER_HPP
#define FG_SAMPLER_HPP

#include "FgImage.hpp"

// Accepts a sample coordinate in IUCS and computes the image color at that point:
typedef std::function<FgRgbaF(FgVect2F)>  FgFuncSample;

FgImgRgbaF
fgSamplerF(
    FgVect2UI           dims,               // Must be non-zero
    FgFuncSample        sample,
    uint                antiAliasBitDepth); // Must be in [1,16]

FgImgRgbaUb
fgSampler(
    FgVect2UI           dims,               // Must be non-zero
    FgFuncSample        sample,
    uint                antiAliasBitDepth); // Must be in [1,8]

#endif

// */
