//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Affine1: 1D affine
// AffineEw: element-wise affine
// Affine: constant dimension affine transform of the form: f(x) = Mx + b
// * Note that composition of operators with Affine is NOT commutative.
// 

#ifndef FGAFFINEC_HPP
#define FGAFFINEC_HPP

#include "FgSerial.hpp"
#include "FgMatrixC.hpp"
#include "FgMatrixV.hpp"
#include "FgApproxEqual.hpp"

namespace Fg {

// 1D version with scalar members; scale (can be negative) and translate transform: x' = sx + t
template <class T>
struct      Affine1
{
    T               m_scale;            // applied first. Cannot be zero.
    T               m_trans;            // applied second
    FG_SER2(m_scale,m_trans)

    Affine1() : m_scale{1}, m_trans{0} {}
    Affine1(T scale,T trans) : m_scale{scale}, m_trans{trans} {FGASSERT(m_scale != 0); }
    // Construct from domain bounds mapping - conceptually easier and much less likely to
    // make a mistake (but somewhat redundant, you could use any 2 domain points instead of the limits)
    // r0 = d0*s+t
    // r1 = d1*s+t
    // (r1-r0) = s(d1-d0), s = (r1-r0)/(d1-d0)
    // t = r0 - d0 * s
    Affine1(
        T               domainLo,       // domain lower limit
        T               domainHi,       // domain upper limit
        // map values are not the same as range since they can reversed !
        T               mapLo,          // domain lower limit maps to this value
        T               mapHi)          // domain upper limit maps to this value
    {set(domainLo,domainHi,mapLo,mapHi); }
    // Don't let conversion constructor override defaults:
    Affine1(Affine1 const &) = default;
    Affine1 &       operator=(Affine1 const &) = default;
    // conversion constructor:
    template<class U>
    Affine1(Affine1<U> aff) : m_scale{T(aff.m_scale)}, m_trans{T(aff.m_trans)} {}

    void            set(T domainLo,T domainHi,T mapLo,T mapHi)
    {
        T           domainDelta = domainHi - domainLo,
                    rangeDelta = mapHi - mapLo;
        FGASSERT(domainDelta*rangeDelta != 0);
        m_scale = rangeDelta / domainDelta;
        m_trans = mapLo - domainLo * m_scale;
    }
    T               operator*(T domainVal) const {return (m_scale * domainVal + m_trans); }
    // multiplication of Affine1s is function composition:
    // y = Sx + t
    // z = S'y + t' = S'(Sx+t) + t' = (S'S)x + (S't + t')
    // rhs below is 'y' above:
    Affine1         operator*(Affine1 r) const
    {
        return {m_scale*r.m_scale, m_scale*r.m_trans + m_trans};
    }
    // inverse: x = (x'-t)/s = (1/s)x' + (-t/s)
    Affine1         inverse() const {return Affine1(T(1)/m_scale,-m_trans/m_scale); }
    T               invert(T rangeVal) const {return ((rangeVal - m_trans) / m_scale); }
};
typedef Affine1<float>       Affine1F;
typedef Affine1<double>      Affine1D;

template<class T>
std::ostream &      operator<<(std::ostream & os,const Affine1<T> & v)
{
    os  << fgnl << "Scale: " << v.m_scale << " Translation: " << v.m_trans;
    return os;
}

template<class T>
Affine1<T>          interpolate(Affine1<T> lhs,Affine1<T> rhs,T val)
{
    return {
        std::exp(interpolate(std::log(lhs.m_scale),std::log(rhs.m_scale),val)),
        interpolate(lhs.m_trans,rhs.m_trans,val)
    };
}

template<class T>
bool                isApproxEqual(Affine1<T> const & l,Affine1<T> const & r,T maxDiff)
{
    return
        isApproxEqual(l.m_scale,r.m_scale,maxDiff) &&
        isApproxEqual(l.m_trans,r.m_trans,maxDiff);
}

// Full N-dimensional version with matrix/vector members
template <class T,uint D>
struct      Affine
{
    Mat<T,D,D>          linear;           // Applied first
    Mat<T,D,1>          translation;      // Applied second
    FG_SER2(linear,translation)

    Affine() : linear {Mat<T,D,D>::identity()} {}
    // Construct from translation: f(x) = x + b
    explicit Affine(Mat<T,D,1> const & trans) : linear {Mat<T,D,D>::identity()}, translation(trans) {}
    // Construct from linear transform: f(x) = Mx
    explicit Affine(const Mat<T,D,D> & lin) : linear(lin) {}
    // Construct from native form: f(x) = Mx + b
    Affine(Mat<T,D,D> const & lin,Mat<T,D,1> const & trans) : linear(lin), translation(trans) {}
    // Construct from opposite order form: f(x) = M(x+b) = Mx + Mb
    Affine(Mat<T,D,1> const & trans,const Mat<T,D,D> & lin) : linear(lin), translation(lin * trans) {}
    // Don't let conversion constructor override default copy constructor:
    Affine(Affine const &) = default;
    Affine &            operator=(Affine const &) = default;
    // Conversion constructor:
    template<typename U>
    Affine(Affine<U,D> const & v) : linear(v.linear), translation(v.translation) {}

    // If 'vec' is a matrix, its columns are transformed as vectors into a new matrix:
    template<uint ncols>
    Mat<T,D,ncols>      operator*(const Mat<T,D,ncols> & vec) const
    {
        Mat<T,D,ncols> ret = linear * vec;
        for (uint col=0; col<ncols; col++)
            for (uint row=0; row<D; row++)
                ret.cr(col,row) += translation[row];
        return ret;
    }

    // Operator composition: L*R -> L(Rx+r) + l = LRx + Lr + l = (LR)x + (Lr+l)
    Affine              operator*(const Affine & rhs) const
    {
        Affine       ret;
        ret.linear = linear * rhs.linear;
        ret.translation = linear * rhs.translation + translation;
        return ret;
    }

    // new = scalar * old:
    void                postScale(T val) {linear *= val; translation *= val; }

    // Ax + a = y -> x = A^-1(y - a) = (A^-1)y - (A^-1a)
    Affine              inverse() const
    {
        Affine       ret;
        ret.linear = cInverse(linear);
        ret.translation = - ret.linear * translation;
        return ret;
    }
};

typedef Affine<float,2>        Affine2F;
typedef Affine<double,2>       Affine2D;
typedef Affine<float,3>        Affine3F;
typedef Affine<double,3>       Affine3D;

// Operator composition: N(Mx+b) = (NM)x + Nb
template<class T,uint dim>
Affine<T,dim>       operator*(const Mat<T,dim,dim> & lhs,const Affine<T,dim> & rhs)
{
    return Affine<T,dim>(lhs*rhs.linear,lhs*rhs.translation);
}

template<class T,uint dim>
std::ostream &      operator<<(std::ostream & os,const Affine<T,dim> & v)
{
    return os  << fgnl << "Linear: " << v.linear << fgnl << " Translation: " << v.translation;
}

template<typename T,uint dim>
Mat<T,dim+1,dim+1>  asHomogMat(Affine<T,dim> a)
{
    Mat<T,dim+1,dim+1>      ret = asHomogMat(a.linear);
    for (uint ii=0; ii<dim; ++ii)
        ret.rc(ii,dim) = a.translation[ii];
    return ret;
}

// Returns least squares affine transform for sets of points in 1-1 correspondence.
// Must be at least 4 non-degenerate point pairs.
Affine3D            solveAffine(Vec3Ds const & base,Vec3Ds const & targ);

// Element-wise affine (aka Bounding Box, Rectilinear) transform:
// per-axis scaling and translation: x'_i = s * x_i + t
template <class T,uint D>
struct      AffineEw
{
    Arr<Affine1<T>,D>   affs;
    FG_SER1(affs)

    AffineEw() {}
    AffineEw(Mat<T,D,1> const &  scales,Mat<T,D,1> const &  trans)
    {
        for (uint dd=0; dd<D; ++dd)
            affs[dd] = Affine1<T>{scales[dd],trans[dd]};
    }
    // Don't let conversion constructor override defaults:
    AffineEw(AffineEw const &) = default;
    AffineEw &       operator=(AffineEw const &) = default;
    // Conversion constructor:
    template<class U>
    AffineEw(AffineEw<U,D> const & r)
    {
        for (uint dd=0; dd<D; ++dd)
            affs[dd] = Affine1<T>{r.affs[dd]};
    }
    // Construct from domain bounds and the points they map to (see Affine1 for details):
    AffineEw(Arr<T,D> domainLo,Arr<T,D> domainHi,Arr<T,D> mapLo,Arr<T,D> mapHi)
    {
        for (uint dd=0; dd<D; ++dd)
            affs[dd] = {domainLo[dd],domainHi[dd],mapLo[dd],mapHi[dd]};
    }
    // DEPRECATED: as above in matrix form (not as simple to understand)
    AffineEw(
        Mat<T,D,2> const & domBnds,     // Column vectors are lo and hi bounds resp.
        Mat<T,D,2> const & mapBnds)     // "
    {
        for (uint dd=0; dd<D; ++dd)
            affs[dd] = Affine1<T>{domBnds.rc(dd,0),domBnds.rc(dd,1),mapBnds.rc(dd,0),mapBnds.rc(dd,1)};
    }
    // Matrices are treated as collated column vectors:
    template<uint numVecs>
    Mat<T,D,numVecs>  operator*(const Mat<T,D,numVecs> & vec) const
    {
        Mat<T,D,numVecs>    ret;
        for (uint rr=0; rr<D; ++rr)
            for (uint cc=0; cc<numVecs; ++cc)
                ret.rc(rr,cc) = affs[rr] * vec.rc(rr,cc);
        return ret;
    }
    AffineEw<T,D>       operator*(AffineEw<T,D> rhs) const          // function composition
    {
        AffineEw<T,D>       ret;
        for (uint dd=0; dd<D; ++dd)
            ret.affs[dd] = affs[dd] * rhs.affs[dd];
        return ret;
    }
    Affine<T,D>         asAffine() const
    {
        Mat<T,D,D>          scales {0};
        Mat<T,D,1>          trans;
        for (uint ii=0; ii<D; ++ii) {
            scales.cr(ii,ii) = affs[ii].m_scale;
            trans[ii] = affs[ii].m_trans;
        }
        return Affine<T,D>(scales,trans);
    }
    AffineEw<T,D>     inverse() const
    {
        AffineEw        ret;
        for (uint dd=0; dd<D; ++dd)
            ret.affs[dd] = affs[dd].inverse();
        return ret;
    }
    Arr<T,D>            scales() const
    {
        Arr<T,D>            ret;
        for (uint dd=0; dd<D; ++dd)
            ret[dd] = affs[dd].m_scale;
        return ret;
    }
    Arr<T,D>            transs() const
    {
        Arr<T,D>            ret;
        for (uint dd=0; dd<D; ++dd)
            ret[dd] = affs[dd].m_trans;
        return ret;
    }

    static AffineEw     scale(T s) {return AffineEw(Mat<T,D,1>(s),Mat<T,D,1>(0)); }
    static AffineEw     translate(Mat<T,D,1> t) {return AffineEw(Mat<T,D,1>(1),t); }
};

typedef AffineEw<float,2>       AffineEw2F;
typedef AffineEw<double,2>      AffineEw2D;
typedef AffineEw<float,3>       AffineEw3F;
typedef AffineEw<double,3>      AffineEw3D;
typedef AffineEw<float,4>       AffineEw4F;

typedef Svec<AffineEw2F>        AffineEw2Fs;
typedef Svec<AffineEw3F>        AffineEw3Fs;
typedef Svec<AffineEw3D>        AffineEw3Ds;

template<class T,uint dim>
inline std::ostream & operator<<(std::ostream & os,const AffineEw<T,dim> & v)
{
    return os  << v.aff;
}

template<typename T,uint dim>
Mat<T,dim+1,dim+1>      asHomogMat(AffineEw<T,dim> a)
{
    Mat<T,dim+1,dim+1>  ret(0);
    for (uint ii=0; ii<dim; ++ii) {
        ret.rc(ii,ii) = a.affs[ii].m_scale;
        ret.rc(ii,dim) = a.affs[ii].m_trans;
    }
    ret.rc(dim,dim) = T(1);
    return ret;
}

template<uint D>
AffineEw<double,D>      interpolate(AffineEw<double,D> lhs,AffineEw<double,D> rhs,double val)
{
    AffineEw<double,D>      ret;
    for (uint dd=0; dd<D; ++dd)
        ret.affs[dd] = interpolate(lhs.affs[dd],rhs.affs[dd],val);
    return ret;
}

template <class T,uint D>
bool                isApproxEqual(AffineEw<T,D> const & lhs,AffineEw<T,D> const & rhs,T maxDiff)
{
    for (uint dd=0; dd<D; ++dd)
        if (!isApproxEqual(lhs.affs[dd],rhs.affs[dd],maxDiff))
            return false;
    return true;
}

}

#endif
