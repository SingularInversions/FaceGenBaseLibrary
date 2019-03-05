//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Feb 20, 2009
//

#ifndef FGHISTOGRAM_HPP
#define FGHISTOGRAM_HPP

#include "FgMatrixC.hpp"
#include "FgMatrixV.hpp"

struct  FgHistogram
{
    vector<size_t>          binCounts;
    FgVectD2                bounds;         // outer bounds of bin range. bounds[1] > bounds[0].

    FgHistogram(
        FgVectD2            bounds,
        size_t              numBins);

    FgHistogram(
        const vector<double> & samples,
        FgVectD2            bounds,
        size_t              numBins);

    bool                                    // Returns true if sample falls into bounds
    addSample(double val);

    size_t
    numSamples() const;

    size_t
    operator[](size_t ii) const
    {return binCounts[ii]; }

    // Return an L1 normalized density of the samples over the range:
    vector<double>
    asDensity() const;
};

#endif
