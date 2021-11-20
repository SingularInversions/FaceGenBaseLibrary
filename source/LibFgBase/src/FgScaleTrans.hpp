//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGSCALETRANS_HPP
#define FGSCALETRANS_HPP

#include "FgStdLibs.hpp"
#include "FgAffine.hpp"

namespace Fg {

template<typename T,uint dim>
struct  ScaleTrans
{
    T               scale {1};          // Apply scale first
    Mat<T,dim,1>    trans {0};          // Apply translation second

    ScaleTrans() {}
    explicit ScaleTrans(double s) : scale(s) {}
    explicit ScaleTrans(Mat<T,dim,1> t) : trans(t) {}
    ScaleTrans(double s,Mat<T,dim,1> t) : scale(s), trans(t) {}

    // explicitly enable default constructor to avoid disabling by conversion constructor:
    ScaleTrans(ScaleTrans const &) = default;

    // Conversion constructor
    template<typename U>
    explicit ScaleTrans(ScaleTrans<U,dim> const & v) : scale(v.scale), trans(v.trans) {}

    // Operator application:
    Mat<T,dim,1>
    operator*(Mat<T,dim,1> rhs) const
    {return rhs * scale + trans; }

    // Operator composition:
    // RHS: y = Sx+t
    // LHS: z = S'y+t' = S'(Sx+t)+t' = S'Sx + (S't+t')
    ScaleTrans
    operator*(ScaleTrans rhs) const
    {return ScaleTrans(scale*rhs.scale,scale*rhs.trans+trans); }

    ScaleTrans
    inverse() const
    {
        // y = Sx + t, x = (y-t)/S = y/S - t/S
        FGASSERT(scale != 0);
        T           invScale = 1 / scale;
        return ScaleTrans(invScale,-invScale*trans);
    }

    AffineEw<T,dim>
    asAffineEw() const
    {return AffineEw<T,dim> {Mat<T,dim,1>{scale},trans}; }

    Affine<T,dim>
    asAffine() const
    {return Affine<T,dim>{Mat<T,dim,dim>::diagonal(scale),trans}; }
};

typedef ScaleTrans<float,2>     ScaleTrans2F;
typedef ScaleTrans<double,2>    ScaleTrans2D;
typedef ScaleTrans<float,3>     ScaleTrans3F;
typedef ScaleTrans<double,3>    ScaleTrans3D;

}

#endif
