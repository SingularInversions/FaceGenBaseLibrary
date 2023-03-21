//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGHISTOGRAM_HPP
#define FGHISTOGRAM_HPP

#include "FgMatrixC.hpp"
#include "FgMatrixV.hpp"

namespace Fg {

struct  FgHistogram
{
    Svec<size_t>          binCounts;
    VecD2                bounds;         // outer bounds of bin range. bounds[1] > bounds[0].

    FgHistogram(
        VecD2            bounds,
        size_t              numBins);

    FgHistogram(
        const Doubles & samples,
        VecD2            bounds,
        size_t              numBins);

    bool                                    // Returns true if sample falls into bounds
    addSample(double val);

    size_t
    numSamples() const;

    size_t
    operator[](size_t ii) const
    {return binCounts[ii]; }

    // Return an L1 normalized density of the samples over the range:
    Doubles
    asDensity() const;
};

}

#endif
