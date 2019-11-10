//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGAPPROXEQUAL_HPP
#define FGAPPROXEQUAL_HPP

#include "FgTypes.hpp"
#include "FgMath.hpp"
#include "FgMatrixC.hpp"
#include "FgMatrixV.hpp"

namespace Fg {

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
fgApproxEqualScalar(T v0,T v1,uint relTolEpsilons=8)
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
    const Svec<float> &   lhs,
    const Svec<float> &   rhs,
    float       relAccuracy = 0.000001)
{
    FGASSERT(lhs.size() == rhs.size());
    FGASSERT(relAccuracy > std::numeric_limits<float>::epsilon());
    float   mag1 = cMag(lhs),
        mag2 = cMag(rhs),
        mag = sqrt((mag1 + mag2) * float(0.5));
    if (mag == float(0))
        return true;
    float   rdelta = cLen(lhs-rhs) / mag,
        eps =  relAccuracy * float(lhs.size());
    return (rdelta <= eps);
}

template<typename T,uint nrows,uint ncols>
bool
fgApproxEqual(
    const Mat<T,nrows,ncols> &    m0,
    const Mat<T,nrows,ncols> &    m1,
    uint    relTolEpsilons=4)
{
    T           delSqr = (m1-m0).mag();
    if (delSqr == 0)
        return true;
    double      relTolerance = relTolEpsilons*std::numeric_limits<T>::epsilon();
    T           relDiff = T(4) * delSqr / (m0+m1).mag();
    return (sqr(relTolerance) >= relDiff);
}

template<typename T,uint nrows,uint ncols>
bool
fgApproxEqual(
    const Svec<Mat<T,nrows,ncols> > &  lhs,
    const Svec<Mat<T,nrows,ncols> > &  rhs,
    T       relAccuracy = 0.000001)
{
    FGASSERT(lhs.size() == rhs.size());
    FGASSERT(relAccuracy > std::numeric_limits<T>::epsilon());
    T   mag1 = cMag(lhs),
        mag2 = cMag(rhs),
        mag = sqrt((mag1 + mag2) * T(0.5));
    if (mag == T(0)) return true;
    T   rdelta = cLen(lhs-rhs) / mag,
        eps =  relAccuracy * T(nrows*ncols*lhs.size());
    return (rdelta <= eps);
}

template<typename T,uint nrows,uint ncols>
bool
fgApproxEqualComponents(
    const Mat<T,nrows,ncols> &    m0,
    const Mat<T,nrows,ncols> &    m1,
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
    const Mat<T,nrows,ncols> &    m0,
    const Mat<T,nrows,ncols> &    m1,
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
    const Svec<Mat<T,nrows,ncols> > &  lhs,
    const Svec<Mat<T,nrows,ncols> > &  rhs,
    T                                               relTol)
{
    FGASSERT(lhs.size() == rhs.size());
    T   scale = fgMaxElem((cDims(lhs) + cDims(rhs)) * T(0.5));
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

}

#endif
