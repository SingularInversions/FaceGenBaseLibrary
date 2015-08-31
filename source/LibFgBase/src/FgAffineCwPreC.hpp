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
class FgAffineCwPreC
{
public:
    // Default constructor sets to identity transform:
    FgAffineCwPreC()
    : m_scales(T(1))
    {}

    // Construct from form f(x) = Diag(s)(x + b)
    FgAffineCwPreC(
        const FgMatrixC<T,dim,1> &      trans,
        const FgMatrixC<T,dim,1> &      scales)
        :
        m_trans(trans),
        m_scales(scales)
        {}

    template<uint ncols>
    FgMatrixC<T,dim,ncols>
    operator*(const FgMatrixC<T,dim,ncols> & vec) const
    {
        FgMatrixC<T,dim,ncols> ret;
        for (uint row=0; row<dim; ++row)
            for (uint col=0; col<ncols; ++col)
                ret.elm(col,row) = m_scales[row] * (vec.elm(col,row) + m_trans[row]);
        return ret;
    }

    std::ostream &
    print(std::ostream & os) const
    {
        os  << fgnl << " Translation: " << m_trans
            << fgnl << " Scales: " << m_scales;
        return os;
    }

private:
    FgMatrixC<T,dim,1>        m_trans;
    FgMatrixC<T,dim,1>        m_scales;
};

typedef FgAffineCwPreC<float,2>      FgAffineCwPre2F;
typedef FgAffineCwPreC<double,2>     FgAffineCwPre2D;
typedef FgAffineCwPreC<float,3>      FgAffineCwPre3F;
typedef FgAffineCwPreC<double,3>     FgAffineCwPre3D;

template<class T,uint dim>
inline std::ostream &
operator<<(std::ostream & os,const FgAffineCwPreC<T,dim> & v)
{return v.print(os); }

#endif
