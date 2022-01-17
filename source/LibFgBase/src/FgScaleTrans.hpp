//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
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
    T               scale {1};          // applied first
    Mat<T,dim,1>    trans {0};          // applied second

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
    Mat<T,dim,1>        operator*(Mat<T,dim,1> rhs) const {return rhs * scale + trans; }
    // Operator composition:
    ScaleTrans          operator*(ScaleTrans rhs) const
    {
        // RHS: y = Sx+t
        // LHS: z = S'y+t' = S'(Sx+t)+t' = S'Sx + (S't+t')
        return ScaleTrans{scale*rhs.scale,scale*rhs.trans+trans};
    }
    ScaleTrans          inverse() const
    {
        // y = Sx + t, x = (y-t)/S = y/S - t/S
        FGASSERT(scale != 0);
        T           invScale = 1 / scale;
        return ScaleTrans{invScale,-invScale*trans};
    }
    AffineEw<T,dim>     asAffineEw() const {return AffineEw<T,dim> {Mat<T,dim,1>{scale},trans}; }
    Affine<T,dim>       asAffine() const {return Affine<T,dim>{Mat<T,dim,dim>::diagonal(scale),trans}; }
};
typedef ScaleTrans<float,2>     ScaleTrans2F;
typedef ScaleTrans<double,2>    ScaleTrans2D;
typedef ScaleTrans<float,3>     ScaleTrans3F;
typedef ScaleTrans<double,3>    ScaleTrans3D;

// least squares scale and translation relative to means, for 1-1 corresponding vertex lists:
template<typename T,uint D>
ScaleTrans<T,D>     solveScaleTrans(Svec<Mat<T,D,1>> const & src,Svec<Mat<T,D,1>> const & dst)
{
    size_t              S = src.size();
    FGASSERT(S > 1);
    FGASSERT(dst.size() == S);
    Mat<T,D,1>          meanS = cMean(src),
                        meanD = cMean(dst),
                        trans = meanD-meanS;
    Svec<Mat<T,D,1>>    srcMC = mapSub(src,meanS),
                        dstMC = mapSub(dst,meanD);
    T                   scale = cDot(srcMC,dstMC) / cMag(srcMC);
    return ScaleTrans<T,D>{meanS} * ScaleTrans<T,D>{scale,trans} * ScaleTrans<T,D>{-meanS};
}

}

#endif
