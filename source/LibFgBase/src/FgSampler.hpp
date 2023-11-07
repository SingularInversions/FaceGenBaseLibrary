//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Adaptive image sampling

#ifndef FG_SAMPLER_HPP
#define FG_SAMPLER_HPP

#include "FgImage.hpp"

namespace Fg {

// Accepts a coordinate in PACS and returns the RGBA for that position, which must have
// linear alpha-premultiplied RGBA values for correct averaging, and alpha must be in [0,1].
typedef Sfun<RgbaF(Vec2F)>  SampleFn;

// Samples RGBA values for each pixel, adaptively sub-sampling based on the difference between
// neighouring values, recursing until the given bit depth is ensured.
// If pixel density <= nyquist frequency implicit in 'sampleFn', artifacts may result.
// Alpha channel may have precision errors and not be exactly 1 even where fully sampled.
// Resulting image RGB values will be alpha-premultiplied (as 'sampleFn' must provide):
ImgC4F              sampleAdaptiveF(
    Vec2UI              dims,                   // Must be non-zero
    SampleFn const &    sampleFn,
    float               channelBound=1,         // Sampler must return channel values in [0,channelBound)
    uint                antiAliasBitDepth=3);   // Must be in [1,16]
}

#endif

// */
