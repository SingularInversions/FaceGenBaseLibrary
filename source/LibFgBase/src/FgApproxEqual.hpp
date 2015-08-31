//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Nov 16, 2010
//

#ifndef FGAPPROXEQUAL_HPP
#define FGAPPROXEQUAL_HPP

#include "FgTypes.hpp"
#include "FgMath.hpp"
#include "FgMatrix.hpp"

template<typename T>
bool
fgApproxEqualScalarRel(T v0,T v1,double relDiff)
{
    T   del = v1-v0;
    if (del == T(0))
        return true;
    T   rel = (T(2) * std::abs(del)) / (std::abs(v0) + std::abs(v1));
    return (rel <= relDiff);
}

template<typename T>
bool
fgApproxEqualScalar(T v0,T v1,uint relTolEpsilons=2)
{
    return
        fgApproxEqualScalarRel(
            v0,v1,
            double(T(relTolEpsilons) * std::numeric_limits<T>::epsilon()));
}

inline bool
fgApproxEqual(float v0,float v1,uint relTolEpsilsons=2)
{return fgApproxEqualScalar(v0,v1,relTolEpsilsons); }

inline bool
fgApproxEqual(double v0,double v1,uint relTolEpsilsons=2)
{return fgApproxEqualScalar(v0,v1,relTolEpsilsons); }

inline bool
fgApproxEqualRel(float v0,float v1,double relDiff)
{return fgApproxEqualScalarRel(v0,v1,relDiff); }

inline bool
fgApproxEqualRel(double v0,double v1,double relDiff)
{return fgApproxEqualScalarRel(v0,v1,relDiff); }

inline
bool
fgApproxEqual(
    const vector<float> &   lhs,
    const vector<float> &   rhs,
    float       relAccuracy = 0.000001)
{
    FGASSERT(lhs.size() == rhs.size());
    FGASSERT(relAccuracy > std::numeric_limits<float>::epsilon());
    float   mag1 = fgLengthSqr(lhs),
        mag2 = fgLengthSqr(rhs),
        mag = sqrt((mag1 + mag2) * float(0.5));
    if (mag == float(0))
        return true;
    float   rdelta = fgLength(lhs-rhs) / mag,
        eps =  relAccuracy * float(lhs.size());
    return (rdelta <= eps);
}

template<typename T>
bool
fgApproxEqualMatrix(
    const T &   m0,
    const T &   m1,
    double      relTolerance)
{
    typedef typename T::ValType Scalar;
    Scalar  delSqr = (m1-m0).lengthSqr();
    if (delSqr == 0)
        return true;
    Scalar  relDiff = Scalar(4) * delSqr / (m0+m1).lengthSqr();
    return (fgSqr(relTolerance) >= relDiff);
}

template<typename T,uint nrows,uint ncols>
bool
fgApproxEqual(
    const FgMatrixC<T,nrows,ncols> &    m0,
    const FgMatrixC<T,nrows,ncols> &    m1,
    uint    relTolEpsilons=4)
{return fgApproxEqualMatrix(m0,m1,T(relTolEpsilons)*std::numeric_limits<T>::epsilon()); }

template<typename T>
bool
fgApproxEqual(
    const FgMatrixV<T> &    m0,
    const FgMatrixV<T> &    m1,
    T                       relTolerance)
{return fgApproxEqualMatrix(m0,m1,relTolerance); }

template<typename T,uint nrows,uint ncols>
bool
fgApproxEqual(
    const std::vector<FgMatrixC<T,nrows,ncols> > &  lhs,
    const std::vector<FgMatrixC<T,nrows,ncols> > &  rhs,
    T       relAccuracy = 0.000001)
{
    FGASSERT(lhs.size() == rhs.size());
    FGASSERT(relAccuracy > std::numeric_limits<T>::epsilon());
    T   mag1 = fgLengthSqr(lhs),
        mag2 = fgLengthSqr(rhs),
        mag = sqrt((mag1 + mag2) * T(0.5));
    if (mag == T(0)) return true;
    T   rdelta = fgLength(lhs-rhs) / mag,
        eps =  relAccuracy * T(nrows*ncols*lhs.size());
    return (rdelta <= eps);
}

template<typename T,uint nrows,uint ncols>
bool
fgApproxEqualComponents(
    const FgMatrixC<T,nrows,ncols> &    m0,
    const FgMatrixC<T,nrows,ncols> &    m1,
    uint                                relEpsilons=2)
{
    FGASSERT(m0.numElems() == m1.numElems());
    bool    retval = true;
    for (uint ii=0; ii<m0.numElems(); ++ii)
        retval = retval && fgApproxEqualScalar(m0[ii],m1[ii],relEpsilons);
    return retval;
}

template<typename T,uint nrows,uint ncols>
bool
fgApproxEqualComponentsAbs(
    const FgMatrixC<T,nrows,ncols> &    m0,
    const FgMatrixC<T,nrows,ncols> &    m1,
    T                                   absDelta)
{
    FGASSERT(m0.numElems() == m1.numElems());
    for (uint ii=0; ii<m0.numElems(); ++ii)
        if (std::abs(m0[ii]-m1[ii])>absDelta)
            return false;
    return true;
}

template<typename T,uint nrows,uint ncols>
bool
fgApproxEqualComponents(
    const std::vector<FgMatrixC<T,nrows,ncols> > &  lhs,
    const std::vector<FgMatrixC<T,nrows,ncols> > &  rhs,
    T                                               relTol)
{
    FGASSERT(lhs.size() == rhs.size());
    T   scale = fgMaxElem((fgDims(lhs) + fgDims(rhs)) * T(0.5));
    for (size_t ii=0; ii<lhs.size(); ++ii)
        if (!fgApproxEqualComponentsAbs(lhs[ii],rhs[ii],scale * relTol))
            return false;
    return true;
}

inline
bool
fgApproxEqual(
    uint        lhs,
    uint        rhs,
    uint        maxDelta)
{return (uint(std::abs(int(lhs)-int(rhs))) <= maxDelta); }

// Approximately equal relative to a magnitude (from which epsilon is calculated):
template<typename T>
inline
bool
fgApproxEqualMag(T v0,T v1,T mag)
{return (std::abs(v0-v1) < mag * std::numeric_limits<T>::epsilon() * 10); }
    

#endif
