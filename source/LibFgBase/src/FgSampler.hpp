//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Adaptive image sampling

#ifndef FG_SAMPLER_HPP
#define FG_SAMPLER_HPP

#include "FgImage.hpp"

namespace Fg {

// Accepts a coordinate in IUCS and returns the RGBA for that position, which must have
// alpha-weighted RGB values for correct averaging. Note that alpha can be between [0,1]
// when transparent surfaces are involved:
typedef Sfun<RgbaF(Vec2F)>  SampleFn;

// Samples RGBA values for each pixel, adaptively sub-sampling based on the difference between
// neighouring values, recursing until the given bit depth is ensured.
// If pixel density <= nyquist frequency implicit in 'sampleFn', artifacts may result.
// Alpha channel has precision errors so will not be exactly 1 even where fully sampled.
// Resulting image RGB values will be alpha-weighted (as 'sampleFn' must provide):
ImgC4F              sampleAdaptiveF(
    Vec2UI              dims,               // Must be non-zero
    SampleFn const &    sampleFn,
    float               channelBound,       // Sampler must return channel values in [0,channelBound)
    uint                antiAliasBitDepth); // Must be in [1,16]

// As above
ImgRgba8            sampleAdaptive(
    Vec2UI              dims,               // Must be non-zero
    SampleFn const &    sampleFn,           // Sampler must return channel values in [0,256)
    uint                antiAliasBitDepth); // Must be in [1,8]

}

#endif

// */
