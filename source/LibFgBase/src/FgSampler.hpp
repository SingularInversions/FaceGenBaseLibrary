//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Adaptive image sampling

#ifndef FG_SAMPLER_HPP
#define FG_SAMPLER_HPP

#include "FgImage.hpp"

namespace Fg {

// Accepts a sample coordinate in IUCS and computes the image color at that point.
// Note that sharp discontinuities will cause sampling to subdivide to the given
// bit depth so using alpha channel to separate background (versus background color)
// may slow down rendering:
typedef Sfun<RgbaF(Vec2F)>      SampleFunc;

ImgC4F
sampleAdaptiveF(
    Vec2UI              dims,               // Must be non-zero
    SampleFunc const &  sampleFn,
    float               channelBound,       // Sampler must return channel values in [0,channelBound)
    uint                antiAliasBitDepth); // Must be in [1,16]

ImgC4UC
sampleAdaptive(
    Vec2UI              dims,               // Must be non-zero
    SampleFunc const &  sampleFn,           // Sampler must return channel values in [0,256)
    uint                antiAliasBitDepth); // Must be in [1,8]

}

#endif

// */
