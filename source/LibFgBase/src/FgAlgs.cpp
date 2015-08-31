//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     May 10, 2005
//

#include "stdafx.h"

#include "FgAlgs.hpp"
#include "FgStdVector.hpp"
#include "FgMath.hpp"

using namespace std;

// Without a hardware nlz (number of leading zeros) instruction, this function has
// to be iterative (C / C++ doesn't have any keyword for this operator):
uint
fgNumLeadingZeros(uint32 x)
{
   uint     n = 0;
   if (x == 0) return(32);
   if (x <= 0x0000FFFF) {n = n +16; x = x <<16;}
   if (x <= 0x00FFFFFF) {n = n + 8; x = x << 8;}
   if (x <= 0x0FFFFFFF) {n = n + 4; x = x << 4;}
   if (x <= 0x3FFFFFFF) {n = n + 2; x = x << 2;}
   if (x <= 0x7FFFFFFF) {n = n + 1;}
   return n;
}

uint
fgLog2Ceil(uint32 xx)
{
    uint    logFloor = fgLog2Floor(xx);
    if (xx == (1u << logFloor))
        return (fgLog2Floor(xx));
    else
        return (fgLog2Floor(xx) + 1u);
}

vector<double>
fgConvolve(
    const vector<double> &     data,
    const vector<double> &     kernel)
{
    FGASSERT(data.size() * kernel.size() > 0);
    FGASSERT((kernel.size() % 2) == 1);
    size_t              kbase = kernel.size()/2;
    vector<double>      ret(data.size(),0.0);
    for (size_t ii=0; ii<ret.size(); ++ii) {
        for (size_t jj=kernel.size()-1; jj<kernel.size(); --jj) {
            size_t      idx = ii + kbase - jj;
            if (idx < data.size())
                ret[ii] += data[idx] * kernel[jj];
        }
    }
    return ret;
}

vector<double>
fgConvolveGauss(
    const std::vector<double> &     in,
    double                          stdev)
{
    FGASSERT(stdev > 0.0);
    // Create kernel w/ 6 stdevs on each side for double since this is 2 parts in 1B:
    size_t          ksize = fgRoundU(stdev * 6);
    if (ksize == 0)
        return in;
    vector<double>  kernel(ksize*2+1);
    for (size_t ii=0; ii<ksize; ++ii) {
        double      val = fgExp(-0.5*fgSqr(ii/stdev));
        kernel[ksize+ii] = val;
        kernel[ksize-ii] = val;
    }
    double fac = 1.0 / fgSum(kernel);
    for (size_t ii=0; ii<kernel.size(); ++ii)
        kernel[ii] *= fac;
    return fgConvolve(in,kernel);
}

vector<double>
fgRelDiff(const vector<double> & a,const vector<double> & b)
{
    vector<double>      ret;
    FGASSERT(a.size() == b.size());
    ret.resize(a.size());
    for (size_t ii=0; ii<a.size(); ++ii)
        ret[ii] = fgRelDiff(a[ii],b[ii]);
    return ret;
}

