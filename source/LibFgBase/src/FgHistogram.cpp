//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgHistogram.hpp"

using namespace std;

namespace Fg {

FgHistogram::FgHistogram(
    VecD2    bounds_,
    size_t      numBins)
    : bounds(bounds_)
{
    FGASSERT(bounds[1] > bounds[0]);
    FGASSERT(numBins > 1);
    binCounts.resize(numBins,0);
}

FgHistogram::FgHistogram(
    const vector<double> &  samples,
    VecD2                bounds_,
    size_t                  numBins)
    : bounds(bounds_)
{
    FGASSERT(samples.size() > 1);
    FGASSERT(bounds[1] > bounds[0]);
    FGASSERT(numBins > 1);
    binCounts.resize(numBins,0);
    double      fac = double(numBins) / (bounds[1]-bounds[0]);
    for (uint ii=0; ii<samples.size(); ++ii) {
        int     bin = round<int>((samples[ii]-bounds[0])*fac);
        if ((bin >= 0) && (bin < int(numBins)))
            ++(binCounts[bin]);
    }
}

bool
FgHistogram::addSample(double val)
{
    double      fac = double(binCounts.size()) / (bounds[1]-bounds[0]);
    int         bin = round<int>((val-bounds[0]) * fac);
    if ((bin >= 0) && (bin < int(binCounts.size()))) {
        ++(binCounts[bin]);
        return true;
    }
    return false;
}

size_t
FgHistogram::numSamples() const
{
    size_t      acc = 0;
    for (size_t ii=0; ii<binCounts.size(); ++ii)
        acc += binCounts[ii];
    return acc;
}

vector<double>
FgHistogram::asDensity() const
{
    // Turn the histogram into L1 normalized density data:
    // * Dividing by numSamples gives an L1ND with 1 unit per bin, then:
    // * Multiplying by numBins/span gives L1ND with span units
    FGASSERT(binCounts.size() > 1);
    double          span = bounds[1]-bounds[0];
    double          fac = binCounts.size() / (span * numSamples());
    vector<double>  ret(binCounts.size());
    for (size_t ii=0; ii<binCounts.size(); ++ii)
        ret[ii] = fac * binCounts[ii];
    return ret;
}

}
