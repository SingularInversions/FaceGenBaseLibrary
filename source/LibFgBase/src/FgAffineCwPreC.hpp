//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Element-wise affine transform of the form: f(x) = Diag(s)(x + b)
//
// USE:
//
// operator*(Affine,Mat) is interpreted as *application* of the operator rather
// than *composition* of operators even when the rhs is a matrix.
//

#ifndef FGAFFINECWPREC_HPP
#define FGAFFINECWPREC_HPP

#include "FgStdLibs.hpp"
#include "FgMatrixC.hpp"

namespace Fg {

template <class T,uint dim>
struct  AffineEwPre
{
    Mat<T,dim,1>        m_trans;      // Applied first
    Mat<T,dim,1>        m_scales;     // Applied second

    AffineEwPre()
    : m_scales(T(1))
    {}

    // Construct from form f(x) = Diag(s)(x + b)
    AffineEwPre(
        Mat<T,dim,1>  trans,
        Mat<T,dim,1>  scales) :
        m_trans(trans),
        m_scales(scales)
        {}

    AffineEwPre(
        Mat<T,dim,2>  domain,
        Mat<T,dim,2>  range)
    {
        Mat<T,dim,1>  domainLo = domain.colVec(0),
                            domainHi = domain.colVec(1),
                            domainSize = domainHi-domainLo,
                            rangeLo = range.colVec(0),
                            rangeHi = range.colVec(1),
                            rangeSize = rangeHi-rangeLo;
        FGASSERT(domainSize.cmpntsProduct() > 0);
        m_scales = mapDiv(rangeSize,domainSize);
        m_trans = mapDiv(rangeLo,m_scales) - domainLo;
    }

    template<uint ncols>
    Mat<T,dim,ncols>
    operator*(const Mat<T,dim,ncols> & vec) const
    {
        Mat<T,dim,ncols> ret;
        for (uint row=0; row<dim; ++row)
            for (uint col=0; col<ncols; ++col)
                ret.cr(col,row) = m_scales[row] * (vec.cr(col,row) + m_trans[row]);
        return ret;
    }
};

typedef AffineEwPre<float,2>      AffineEwPre2F;
typedef AffineEwPre<double,2>     AffineEwPre2D;
typedef AffineEwPre<float,3>      AffineEwPre3F;
typedef AffineEwPre<double,3>     AffineEwPre3D;

template<class T,uint dim>
std::ostream &
operator<<(std::ostream & os,const AffineEwPre<T,dim> & v)
{
    os  << fgnl << " Translation: " << v.m_trans << " Scales: " << v.m_scales;
    return os;
}

}

#endif
