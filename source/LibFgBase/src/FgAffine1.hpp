//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     March 25, 2011
//
// Scale (can be negative) and translate transform: x' = sx + t

#ifndef FGAFFINE1_HPP
#define FGAFFINE1_HPP

#include "FgStdLibs.hpp"

template <class T>
struct  FgAffine1
{
    T       m_scale;
    T       m_trans;

    FgAffine1() : m_scale(T(1)), m_trans(T(0)) {}
    FgAffine1(T scale,T trans) : m_scale(scale), m_trans(trans) {}

    // Construct from bounding box mapping:
    FgAffine1(
        FgMatrixC<T,1,2> domainBounds,
        FgMatrixC<T,1,2> rangeBounds)
    {
        T   domainDelta = domainBounds[1] - domainBounds[0],
            rangeDelta = rangeBounds[1] - rangeBounds[0];
        FGASSERT(domainDelta*rangeDelta != T(0));
        m_scale = rangeDelta / domainDelta;
        m_trans = rangeBounds[0] - domainBounds[0] * m_scale;
    }

    T
    operator*(T domainVal) const
    {return (m_scale * domainVal + m_trans); }

    // x = (x'-t)/s = (1/s)x' + (-t/s)
    FgAffine1
    inverse() const
    {return FgAffine1(T(1)/m_scale,-m_trans/m_scale); }

    T
    invXform(T rangeVal) const
    {return ((rangeVal - m_trans) / m_scale); }
};

typedef FgAffine1<float>       FgAffine1F;
typedef FgAffine1<double>      FgAffine1D;

template<class T>
std::ostream &
operator<<(std::ostream & os,const FgAffine1<T> & v)
{
    os  << fgnl << "Scale: " << v.m_scale
        << fgnl << " Translation: " << v.m_trans;
    return os;
}

#endif
