//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Element-wise affine (aka Bounding Box) transform:
// Translation and axial scaling (can be negative): x' = Sx + t
//

#ifndef FGAFFINECWC_HPP
#define FGAFFINECWC_HPP

#include "FgStdLibs.hpp"
#include "FgAffineC.hpp"

namespace Fg {

template <class T,uint dim>
struct  AffineEw
{
    Mat<T,dim,1>      m_scales;       // Applied first
    Mat<T,dim,1>      m_trans;
    FG_SERIALIZE2(m_scales,m_trans);

    AffineEw() : m_scales(T(1)), m_trans(0) {}

    AffineEw(
        const Mat<T,dim,1> &  scales,
        const Mat<T,dim,1> &  trans)
        : m_scales(scales), m_trans(trans) {}

    // Conversion constructor:
    template<class U>
    AffineEw(const AffineEw<U,dim> & rhs) :
        m_scales(Mat<T,dim,1>(rhs.m_scales)),
        m_trans(Mat<T,dim,1>(rhs.m_trans))
    {}

    // Construct from bounding box mapping:
    AffineEw(
        const Mat<T,dim,2> & domainBounds,    // Column vectors are lo and hi bounds resp.
        const Mat<T,dim,2> & rangeBounds)     // "
    {
        Mat<T,dim,1>
            domainDelta = domainBounds.colVec(1) - domainBounds.colVec(0),
            rangeDelta = rangeBounds.colVec(1) - rangeBounds.colVec(0);
        // Note that the deltas can be negative if the transform inverts an axis:
        FGASSERT(fgNoZeros(domainDelta) && fgNoZeros(rangeDelta));
        for (uint dd=0; dd<dim; ++dd) {
            m_scales[dd] = rangeDelta[dd] / domainDelta[dd];
            m_trans[dd] = rangeBounds.cr(0,dd) - domainBounds.cr(0,dd) * m_scales[dd];
        }
    }

    // Matrices are treated as collated column vectors:
    template<uint numVecs>
    Mat<T,dim,numVecs>
    operator*(const Mat<T,dim,numVecs> & vec) const
    {
        Mat<T,dim,numVecs>    ret;
        for (uint rr=0; rr<dim; ++rr)
            for (uint cc=0; cc<numVecs; ++cc)
                ret.rc(rr,cc) = m_scales[rr] * vec.rc(rr,cc) + m_trans[rr];
        return ret;
    }

    // Composition:
    // y = Sx + t
    // z = S'y + t' = S'(Sx+t) + t' = (S'S)x + (S't + t')
    // rhs below is 'y' above:
    AffineEw<T,dim>
    operator*(AffineEw<T,dim> rhs) const
    {
        AffineEw<T,dim>      ret;
        for (uint dd=0; dd<dim; ++dd) {
            ret.m_scales[dd] = m_scales[dd] * rhs.m_scales[dd];
            ret.m_trans[dd] = m_scales[dd] * rhs.m_trans[dd] + m_trans[dd];
        }
        return ret;
    }

    Affine<T,dim>
    asAffine() const
    {
        Mat<T,dim,dim>    scales;
        for (uint ii=0; ii<dim; ++ii)
            scales.cr(ii,ii) = m_scales[ii];
        return Affine<T,dim>(scales,m_trans);
    }

    // y = Sx + t, x = (y - t)/S = (1/S)y - (t/S)
    AffineEw<T,dim>
    inverse() const
    {
        AffineEw   ret;
        for (uint dd=0; dd<dim; ++dd) {
            ret.m_scales[dd] = T(1) / m_scales[dd];
            ret.m_trans[dd] = -m_trans[dd] / m_scales[dd];
        }
        return ret;
    }

    static
    AffineEw
    scale(T s)
    {return AffineEw(Mat<T,dim,1>(s),Mat<T,dim,1>(0)); }

    static
    AffineEw
    translate(Mat<T,dim,1> t)
    {return AffineEw(Mat<T,dim,1>(1),t); }
};

typedef AffineEw<float,2>       AffineEw2F;
typedef AffineEw<double,2>      AffineEw2D;
typedef AffineEw<float,3>       AffineEw3F;
typedef AffineEw<double,3>      AffineEw3D;

typedef Svec<AffineEw2F>        AffineEw2Fs;
typedef Svec<AffineEw3F>        AffineEw3Fs;

template<class T,uint dim>
inline std::ostream &
operator<<(std::ostream & os,const AffineEw<T,dim> & v)
{
    os  << "Scales: " << v.m_scales << " Translation: " << v.m_trans;
    return os;
}

template<uint dim>
AffineEw<float,dim>
fgD2F(const AffineEw<double,dim> & v)
{
    return AffineEw<float,dim>(
        Mat<float,dim,1>(v.m_scales),
        Mat<float,dim,1>(v.m_trans));
}

}

#endif
