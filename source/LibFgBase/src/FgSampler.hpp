//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Adaptive image sampling

#ifndef FG_SAMPLER_HPP
#define FG_SAMPLER_HPP

#include "FgImage.hpp"

namespace Fg {

// Accepts a sample coordinate in IUCS and computes the image color at that point:
typedef std::function<RgbaF(Vec2F)>  SampleFunc;

ImgC4F
sampleAdaptiveF(
    Vec2UI              dims,               // Must be non-zero
    SampleFunc          sample,
    uint                antiAliasBitDepth); // Must be in [1,16]

ImgC4UC
sampleAdaptive(
    Vec2UI              dims,               // Must be non-zero
    SampleFunc          sample,
    uint                antiAliasBitDepth); // Must be in [1,8]

}

#endif

// */
