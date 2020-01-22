//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
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
uint constexpr defaultPrecisionBits();

template<>
inline uint constexpr defaultPrecisionBits<float>() {return 20;}

template<>
inline uint constexpr defaultPrecisionBits<double>() {return 40;}

// Are two numbers approximately equal relative to their absolute sizes ?
template<typename T>
bool
approxEqualRel(T v0,T v1,uint precisionBits=defaultPrecisionBits<T>())
{
    double      precision = scast<double>(1ULL << precisionBits);
    FGASSERT(precision > 0.0);
    double      d0 = scast<double>(v0),
                d1 = scast<double>(v1),
                del = d1 - d0;
    if (del == 0.0)
        return true;
    double      rel = std::abs(del) / cMax(std::abs(d0),std::abs(d1));
    return (rel < precision);
}

// Are two number approximately equal relative to the given scale ?
template<typename T>
bool
approxEqualAbs(T v0,T v1,T scale,uint precisionBits=defaultPrecisionBits<T>())
{
    double      precision = scast<double>(1ULL << precisionBits);
    FGASSERT(precision > 0.0);
    double      d0 = scast<double>(v0),
                d1 = scast<double>(v1),
                s = scast<double>(scale),
                rel = std::abs(d1 - d0)/std::abs(s);
    return (rel < precision);
}

// Ensure the L2 norms are equal to the given number of bits of precision.
// The arguments must be numerical containers supporting 'cMag' and subtraction:
template<typename Container>
bool
approxEqualRelMag(Container const & lhs,Container const & rhs,uint precisionBits=20)
{
    FGASSERT(lhs.size() == rhs.size());
    double          precision = scast<double>(1ULL << precisionBits),
                    precSqr = sqr(precision);
    FGASSERT(precSqr > 0.0);
    double          mag = cMax(cMag(lhs),cMag(rhs));
    if (mag == 0.0)
        return true;
    double          rel = cMag(lhs-rhs) / mag;
    return (rel < precSqr);
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
    T   scale = cMaxElem((cDims(lhs) + cDims(rhs)) * T(0.5));
    for (size_t ii=0; ii<lhs.size(); ++ii)
        if (!fgApproxEqualComponentsAbs(lhs[ii],rhs[ii],scale * relTol))
            return false;
    return true;
}

}

#endif
