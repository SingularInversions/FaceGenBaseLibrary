//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     April 27, 2010
//
// Component-wise affine (aka Bounding Box) transform:
// Translation and axial scaling (can be negative): x' = Sx + t
//

#ifndef FGAFFINECWC_HPP
#define FGAFFINECWC_HPP

#include "FgStdLibs.hpp"
#include "FgAffineC.hpp"

template <class T,uint dim>
struct  FgAffineCwC
{
    FgMatrixC<T,dim,1>      m_scales;       // Applied first
    FgMatrixC<T,dim,1>      m_trans;

    FgAffineCwC() : m_scales(T(1)) {}

    // Conversion constructor:
    template<class U>
    explicit
    FgAffineCwC(const FgAffineCwC<U,dim> & rhs) :
        m_scales(FgMatrixC<T,dim,1>(rhs.m_scales)),
        m_trans(FgMatrixC<T,dim,1>(rhs.m_trans))
    {}

    // Construct from bounding box mapping:
    FgAffineCwC(
        const FgMatrixC<T,dim,2> & domainBounds,    // Column vectors are lo and hi bounds resp.
        const FgMatrixC<T,dim,2> & rangeBounds)     // "
    {
        FgMatrixC<T,dim,1>
            domainDelta = domainBounds.colVec(1) - domainBounds.colVec(0),
            rangeDelta = rangeBounds.colVec(1) - rangeBounds.colVec(0);
        // Note that the deltas can be negative if the transform inverts an axis:
        FGASSERT(fgNoZeros(domainDelta) && fgNoZeros(rangeDelta));
        for (uint dd=0; dd<dim; ++dd) {
            m_scales[dd] = rangeDelta[dd] / domainDelta[dd];
            m_trans[dd] = rangeBounds.elm(0,dd) - domainBounds.elm(0,dd) * m_scales[dd];
        }
    }

    FgAffineCwC(
        const FgMatrixC<T,dim,1> &  scales,
        const FgMatrixC<T,dim,1> &  trans)
        : m_scales(scales), m_trans(trans) {}

    FgMatrixC<T,dim,1>
    operator*(const FgMatrixC<T,dim,1> & vec) const
    {
        FgMatrixC<T,dim,1>  ret;
        for (uint rr=0; rr<dim; ++rr)
            ret[rr] = m_scales[rr] * vec[rr] + m_trans[rr];
        return ret;
    }

    // Composition: y = Sx + t, z = S'y + t' = S'(Sx+t) + t'
    FgAffineCwC<T,dim>
    operator*(FgAffineCwC<T,dim> rhs) const
    {
        FgAffineCwC<T,dim>      ret;
        for (uint dd=0; dd<dim; ++dd) {
            ret.m_scales[dd] = m_scales[dd] * rhs.m_scales[dd];
            ret.m_trans[dd] = m_trans[dd] + m_scales[dd] * rhs.m_trans[dd];
        }
        return ret;
    }

    FgAffineC<T,dim>
    asAffine() const
    {
        FgMatrixC<T,dim,dim>    scales;
        for (uint ii=0; ii<dim; ++ii)
            scales.elm(ii,ii) = m_scales[ii];
        return FgAffineC<T,dim>(scales,m_trans);
    }

    // y = Sx + t, x = (y - t)/S = (1/S)y - (t/S)
    FgAffineCwC<T,dim>
    inverse() const
    {
        FgAffineCwC   ret;
        for (uint dd=0; dd<dim; ++dd) {
            ret.m_scales[dd] = T(1) / m_scales[dd];
            ret.m_trans[dd] = -m_trans[dd] / m_scales[dd];
        }
        return ret;
    }
};

typedef FgAffineCwC<float,2>        FgAffineCw2F;
typedef FgAffineCwC<double,2>       FgAffineCw2D;
typedef FgAffineCwC<float,3>        FgAffineCw3F;
typedef FgAffineCwC<double,3>       FgAffineCw3D;

template<class T,uint dim>
inline std::ostream &
operator<<(std::ostream & os,const FgAffineCwC<T,dim> & v)
{
    os  << "Scales: " << v.m_scales << " Translation: " << v.m_trans;
    return os;
}

// More efficient for abstracting just a translation:
template<class T,uint dim>
struct  FgTranslate
{
    FgMatrixC<T,dim,1>  delta;

    FgMatrixC<T,dim,1>
    operator*(const FgMatrixC<T,dim,1> & rhs) const
    {return rhs + delta; }
};

// Client avoids specifying template parameters:
template<class T,uint dim>
inline
FgTranslate<T,dim>
fgTranslate(FgMatrixC<T,dim,1> delta)
{
    FgTranslate<T,dim>  d;
    d.delta = delta;
    return d;
}

template<typename T>
FgTranslate<T,2>
fgTranslate(T x,T y)
{
    FgTranslate<T,2>    ret;
    ret.delta[0] = x;
    ret.delta[1] = y;
    return ret;
}

template<uint dim>
FgAffineCwC<float,dim>
fgD2F(const FgAffineCwC<double,dim> & v)
{
    return FgAffineCwC<float,dim>(
        FgMatrixC<float,dim,1>(v.m_scales),
        FgMatrixC<float,dim,1>(v.m_trans));
}

#endif
