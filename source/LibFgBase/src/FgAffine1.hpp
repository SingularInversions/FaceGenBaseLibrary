//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Scale (can be negative) and translate transform: x' = sx + t

#ifndef FGAFFINE1_HPP
#define FGAFFINE1_HPP

#include "FgStdLibs.hpp"

namespace Fg {

template <class T>
struct  Affine1
{
    T       m_scale;
    T       m_trans;

    Affine1() : m_scale(T(1)), m_trans(T(0)) {}
    Affine1(T scale,T trans) : m_scale(scale), m_trans(trans) {}

    // Construct from bounding box mapping:
    Affine1(
        Mat<T,1,2> domainBounds,
        Mat<T,1,2> rangeBounds)
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
    Affine1
    inverse() const
    {return Affine1(T(1)/m_scale,-m_trans/m_scale); }

    T
    invXform(T rangeVal) const
    {return ((rangeVal - m_trans) / m_scale); }
};

typedef Affine1<float>       Affine1F;
typedef Affine1<double>      Affine1D;

template<class T>
std::ostream &
operator<<(std::ostream & os,const Affine1<T> & v)
{
    os  << fgnl << "Scale: " << v.m_scale
        << fgnl << " Translation: " << v.m_trans;
    return os;
}

}

#endif
