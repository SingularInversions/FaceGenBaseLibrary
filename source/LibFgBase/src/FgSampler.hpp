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

struct  FgSample
{
    virtual ~FgSample() {}

    virtual FgRgbaF
    operator()(FgVect2F posIucs) const = 0;
};

void
fgSampler(
    const FgSample &    sample,
    FgImgRgbaF &        img,    // Contents modified. Must be square power of 2 dimensions
    uint                antiAliasBitDepth); // Must be in [1,8]

void
fgSampler(
    const FgSample &    sample,
    FgImgRgbaUb &       img,    // Contents modified. Must be square power of 2 dimensions
    uint                antiAliasBitDepth); // Must be in [1,8]

#endif

// */
