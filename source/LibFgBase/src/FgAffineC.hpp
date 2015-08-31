//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     April 6, 2005
//
// Constant dimension affine transform of the form: f(x) = Mx + b
//
// USE:
//
// operator*(FgAffineC,FgMatrixC) is interpreted as *application* of the operator rather
// than *composition* of operators even when the rhs is a matrix.
//
// All definitinos of operator* other than that above are interpreted as composition of
// operators.
//
// Composition of operators with FgAffineC is NOT commutative.
// 

#ifndef FGAFFINEC_HPP
#define FGAFFINEC_HPP

#include "FgStdLibs.hpp"
#include "FgMatrix.hpp"

template <class T,uint dim>
struct  FgAffineC
{
    FgMatrixC<T,dim,dim>      linear;           // Applied first
    FgMatrixC<T,dim,1>        translation;      // Applied second

    FgAffineC() {linear.setIdentity(); }

    // Construct from form f(x) = x + b
    explicit
    FgAffineC(const FgMatrixC<T,dim,1> & trans)
    : translation(trans)
    {linear.setIdentity(); }

    // Construct from form f(x) = Mx
    explicit
    FgAffineC(const FgMatrixC<T,dim,dim> & mat)
    : linear(mat)
    {}

    // Construct from form f(x) = Mx + b
    FgAffineC(
        const FgMatrixC<T,dim,dim> &    xform,
        const FgMatrixC<T,dim,1> &      trans)
        :
        linear(xform),
        translation(trans)
        {}

    // Construct from form f(x) = M(x+b):
    FgAffineC(
        const FgMatrixC<T,dim,1> &      trans,
        const FgMatrixC<T,dim,dim> &    xform)
        :
        linear(xform),
        translation(xform * trans)
        {}

    // Explicit normal copy constructor:
    FgAffineC(const FgAffineC & v)
    : linear(v.linear), translation(v.translation)
    {}

    // Conversion constructor:
    template<typename U>
    FgAffineC(const FgAffineC<U,dim> & v)
    : linear(v.linear), translation(v.translation)
    {}

    const FgMatrixC<T,dim,1> &
    postTranslation() const
    {return translation; }

    // If 'vec' is a matrix, its columns are transformed as vectors into a new matrix:
    template<uint ncols>
    FgMatrixC<T,dim,ncols>
    operator*(const FgMatrixC<T,dim,ncols> & vec) const
    {
        FgMatrixC<T,dim,ncols> ret = linear * vec;
        for (uint col=0; col<ncols; col++)
            for (uint row=0; row<dim; row++)
                ret.elm(col,row) += translation[row];
        return ret;
    }

    // Operator composition: L*R -> L(Rx+r) + l = LRx + Lr + l = (LR)x + (Lr+l)
    FgAffineC
    operator*(const FgAffineC & rhs) const
    {
        FgAffineC       ret;
        ret.linear = linear * rhs.linear;
        ret.translation = linear * rhs.translation + translation;
        return ret;
    }

    // new = scalar * old:
    void
    postScale(T val)
    {linear *= val; translation *= val; }

    // Ax + a = y -> x = A^-1(y - a) = (A^-1)y - (A^-1a)
    FgAffineC
    inverse() const
    {
        FgAffineC       ret;
        ret.linear = fgMatInverse(linear);
        ret.translation = - ret.linear * translation;
        return ret;
    }

    FgMatrixC<T,dim+1,dim+1>
    asHomogenous() const
    {
        FgMatrixC<T,dim+1,dim+1>    ret;
        ret.setSubMatrix(linear,0,0);
        ret.setSubMatrix(translation,0,dim);
        ret.elm(dim,dim) = T(1);
        return ret;
    }
};

typedef FgAffineC<float,2>        FgAffine2F;
typedef FgAffineC<double,2>       FgAffine2D;
typedef FgAffineC<float,3>        FgAffine3F;
typedef FgAffineC<double,3>       FgAffine3D;

// Operator composition: N(Mx+b) = (NM)x + Nb
template<class T,uint dim>
FgAffineC<T,dim>
operator*(
    const FgMatrixC<T,dim,dim> &    lhs,
    const FgAffineC<T,dim> &        rhs)
{
    return
        FgAffineC<T,dim>(lhs*rhs.linear,lhs*rhs.translation);
}

template<class T,uint dim>
std::ostream &
operator<<(std::ostream & os,const FgAffineC<T,dim> & v)
{
    return
        os  << fgnl << "Linear: " << v.linear 
            << fgnl << " Translation: " << v.translation;
}

#endif
