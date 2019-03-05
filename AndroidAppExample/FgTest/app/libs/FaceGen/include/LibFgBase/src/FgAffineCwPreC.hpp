//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Sept. 30, 2010
//
// Component-wise affine transform of the form: f(x) = Diag(s)(x + b)
//
// USE:
//
// operator*(FgAffineC,FgMatrixC) is interpreted as *application* of the operator rather
// than *composition* of operators even when the rhs is a matrix.
//

#ifndef FGAFFINECWPREC_HPP
#define FGAFFINECWPREC_HPP

#include "FgStdLibs.hpp"

#include "FgMatrixC.hpp"

template <class T,uint dim>
struct  FgAffineCwPreC
{
    FgMatrixC<T,dim,1>        m_trans;      // Applied first
    FgMatrixC<T,dim,1>        m_scales;     // Applied second

    FgAffineCwPreC()
    : m_scales(T(1))
    {}

    // Construct from form f(x) = Diag(s)(x + b)
    FgAffineCwPreC(
        FgMatrixC<T,dim,1>  trans,
        FgMatrixC<T,dim,1>  scales) :
        m_trans(trans),
        m_scales(scales)
        {}

    FgAffineCwPreC(
        FgMatrixC<T,dim,2>  domain,
        FgMatrixC<T,dim,2>  range)
    {
        FgMatrixC<T,dim,1>  domainLo = domain.colVec(0),
                            domainHi = domain.colVec(1),
                            domainSize = domainHi-domainLo,
                            rangeLo = range.colVec(0),
                            rangeHi = range.colVec(1),
                            rangeSize = rangeHi-rangeLo;
        FGASSERT(domainSize.cmpntsProduct() > 0);
        m_scales = fgMapDiv(rangeSize,domainSize);
        m_trans = fgMapDiv(rangeLo,m_scales) - domainLo;
    }

    template<uint ncols>
    FgMatrixC<T,dim,ncols>
    operator*(const FgMatrixC<T,dim,ncols> & vec) const
    {
        FgMatrixC<T,dim,ncols> ret;
        for (uint row=0; row<dim; ++row)
            for (uint col=0; col<ncols; ++col)
                ret.cr(col,row) = m_scales[row] * (vec.cr(col,row) + m_trans[row]);
        return ret;
    }
};

typedef FgAffineCwPreC<float,2>      FgAffineCwPre2F;
typedef FgAffineCwPreC<double,2>     FgAffineCwPre2D;
typedef FgAffineCwPreC<float,3>      FgAffineCwPre3F;
typedef FgAffineCwPreC<double,3>     FgAffineCwPre3D;

template<class T,uint dim>
std::ostream &
operator<<(std::ostream & os,const FgAffineCwPreC<T,dim> & v)
{
    os  << fgnl << " Translation: " << v.m_trans << " Scales: " << v.m_scales;
    return os;
}

#endif
