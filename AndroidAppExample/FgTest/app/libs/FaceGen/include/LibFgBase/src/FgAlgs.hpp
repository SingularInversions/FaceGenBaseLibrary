//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Feb 21, 2005
//
// General purpose little algorithms that are used broadly and don't fit readily into
// any other class/file.
//

#ifndef FGALGS_HPP
#define FGALGS_HPP

#include "FgStdLibs.hpp"
#include "FgTypes.hpp"
#include "FgStdVector.hpp"

// Useful for recursive template stub, 3-arg min/max, and when windows.h is included (has min/max macros):

template<class T>
inline T fgMax(T x1,T x2) {return std::max(x1,x2); }

template<class T> 
inline T fgMax(T x1,T x2,T x3) {return std::max(std::max(x1,x2),x3); }

template<class T> 
inline T fgMax(T x1,T x2,T x3,T x4) {return std::max(std::max(x1,x2),std::max(x3,x4)); }

template<class T>
inline T fgMin(T x1,T x2) {return std::min(x1,x2); }

template<class T>
inline T fgMin(T x1,T x2,T x3) {return std::min(std::min(x1,x2),x3); }

// Useful for min/max with different types:
inline uint64 fgMin(uint64 a,uint b) {return std::min(a,uint64(b)); }

inline uint64 fgMin(uint a,uint64 b) {return std::min(uint64(a),b); }

// 1D convolution with zero-value boundary handling (non-optimized):
std::vector<double>
fgConvolve(
    const vector<double> &     data,
    const vector<double> &     kernel);    // Must be odd size with convolution centre in middle.

// 1D Gaussian convolution for large kernels (direct sampled kernel)
// with zero-value boundary handling:
std::vector<double>
fgConvolveGauss(
    const vector<double> &     in,
    double                          stdev); // Kernel stdev relative to sample spacing.

// Gives relative difference for similar values, limit values of +/- 2 for very different values.
// If 'minAbs' is specified, that will be used as the minimum for determining relative difference
// (useful when one value may be zero but we know the scale of the comparison):
inline
double
fgRelDiff(double a,double b,double minAbs=0.0)
{
    double      del = b-a,
                denom = std::abs(b)+std::abs(a);
    denom = (denom < minAbs) ? minAbs : denom;
    return (del == 0) ? 0 : del * 2.0 / denom;
}

vector<double>
fgRelDiff(const vector<double> & a,const vector<double> & b,double minAbs=0.0);

#endif
