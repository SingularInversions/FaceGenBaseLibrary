//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Affine1: 1D affine
// AffineEw: element-wise affine
//
// Affine: constant dimension affine transform of the form: f(x) = Mx + b
// USE:
// * operator*(Affine,Mat) is interpreted as *application* of the operator rather
//   than *composition* of operators even when the rhs is a matrix.
// * All definitinos of operator* other than that above are interpreted as composition of
//   operators.
// * Composition of operators with Affine is NOT commutative.
// 

#ifndef FGAFFINEC_HPP
#define FGAFFINEC_HPP

#include "FgStdLibs.hpp"
#include "FgMatrixC.hpp"
#include "FgMatrixV.hpp"

namespace Fg {

// 1D version with scalar members; scale (can be negative) and translate transform: x' = sx + t
template <class T>
struct  Affine1
{
    T               m_scale;
    T               m_trans;

    Affine1() : m_scale(T(1)), m_trans(T(0)) {}
    Affine1(T scale,T trans) : m_scale(scale), m_trans(trans) {}

    // Construct from bounding box mapping:
    Affine1(Mat<T,1,2> domainBounds,Mat<T,1,2> rangeBounds)
    {
        T   domainDelta = domainBounds[1] - domainBounds[0],
            rangeDelta = rangeBounds[1] - rangeBounds[0];
        FGASSERT(domainDelta*rangeDelta != T(0));
        m_scale = rangeDelta / domainDelta;
        m_trans = rangeBounds[0] - domainBounds[0] * m_scale;
    }

    T               operator*(T domainVal) const {return (m_scale * domainVal + m_trans); }
    // x = (x'-t)/s = (1/s)x' + (-t/s)
    Affine1         inverse() const {return Affine1(T(1)/m_scale,-m_trans/m_scale); }
    T               invXform(T rangeVal) const {return ((rangeVal - m_trans) / m_scale); }
};
typedef Affine1<float>       Affine1F;
typedef Affine1<double>      Affine1D;

template<class T>
std::ostream &      operator<<(std::ostream & os,const Affine1<T> & v)
{
    os  << fgnl << "Scale: " << v.m_scale
        << fgnl << " Translation: " << v.m_trans;
    return os;
}

// Full N-dimensional version with matrix/vector members
template <class T,uint D>
struct      Affine
{
    Mat<T,D,D>          linear;           // Applied first
    Mat<T,D,1>          translation;      // Applied second

    FG_SERIALIZE2(linear,translation);

    Affine() : linear {Mat<T,D,D>::identity()} {}
    // Construct from translation: f(x) = x + b
    explicit Affine(Mat<T,D,1> const & trans) : linear {Mat<T,D,D>::identity()}, translation(trans) {}
    // Construct from linear transform: f(x) = Mx
    explicit Affine(const Mat<T,D,D> & lin) : linear(lin) {}
    // Construct from native form: f(x) = Mx + b
    Affine(Mat<T,D,D> const & lin,Mat<T,D,1> const & trans) : linear(lin), translation(trans) {}
    // Construct from opposite order form: f(x) = M(x+b) = Mx + Mb
    Affine(Mat<T,D,1> const & trans,const Mat<T,D,D> & lin) : linear(lin), translation(lin * trans) {}
    // Conversion constructor:
    template<typename U>
    Affine(Affine<U,D> const & v) : linear(v.linear), translation(v.translation) {}
    // Don't let conversion constructor override default copy constructor:
    Affine(Affine const & v) = default;

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
template <class T,uint dim>
struct      AffineEw
{
    Mat<T,dim,1>      m_scales;       // Applied first
    Mat<T,dim,1>      m_trans;
    FG_SERIALIZE2(m_scales,m_trans);

    AffineEw() : m_scales(T(1)), m_trans(0) {}
    AffineEw(Mat<T,dim,1> const &  scales,Mat<T,dim,1> const &  trans) : m_scales(scales), m_trans(trans) {}
    // Conversion constructor:
    template<class U>
    AffineEw(const AffineEw<U,dim> & rhs) :
        m_scales(Mat<T,dim,1>(rhs.m_scales)),
        m_trans(Mat<T,dim,1>(rhs.m_trans))
    {}
    // Construct from bounding box mapping:
    AffineEw(
        Mat<T,dim,2> const & domainBounds,    // Column vectors are lo and hi bounds resp.
        Mat<T,dim,2> const & rangeBounds)     // "
    {
        Mat<T,dim,1>
            domainDelta = domainBounds.colVec(1) - domainBounds.colVec(0),
            rangeDelta = rangeBounds.colVec(1) - rangeBounds.colVec(0);
        // Note that the deltas can be negative if the transform inverts an axis:
        FGASSERT(noZeroElems(domainDelta) && noZeroElems(rangeDelta));
        for (uint dd=0; dd<dim; ++dd) {
            m_scales[dd] = rangeDelta[dd] / domainDelta[dd];
            m_trans[dd] = rangeBounds.cr(0,dd) - domainBounds.cr(0,dd) * m_scales[dd];
        }
    }

    // Matrices are treated as collated column vectors:
    template<uint numVecs>
    Mat<T,dim,numVecs>  operator*(const Mat<T,dim,numVecs> & vec) const
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
    AffineEw<T,dim>     operator*(AffineEw<T,dim> rhs) const
    {
        AffineEw<T,dim>      ret;
        for (uint dd=0; dd<dim; ++dd) {
            ret.m_scales[dd] = m_scales[dd] * rhs.m_scales[dd];
            ret.m_trans[dd] = m_scales[dd] * rhs.m_trans[dd] + m_trans[dd];
        }
        return ret;
    }

    Affine<T,dim>       asAffine() const
    {
        Mat<T,dim,dim>    scales;
        for (uint ii=0; ii<dim; ++ii)
            scales.cr(ii,ii) = m_scales[ii];
        return Affine<T,dim>(scales,m_trans);
    }

    // y = Sx + t, x = (y - t)/S = (1/S)y - (t/S)
    AffineEw<T,dim>     inverse() const
    {
        AffineEw   ret;
        for (uint dd=0; dd<dim; ++dd) {
            ret.m_scales[dd] = T(1) / m_scales[dd];
            ret.m_trans[dd] = -m_trans[dd] / m_scales[dd];
        }
        return ret;
    }

    static AffineEw     scale(T s) {return AffineEw(Mat<T,dim,1>(s),Mat<T,dim,1>(0)); }
    static AffineEw     translate(Mat<T,dim,1> t) {return AffineEw(Mat<T,dim,1>(1),t); }
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
    os  << "Scales: " << v.m_scales << " Translation: " << v.m_trans;
    return os;
}

template<typename T,uint dim>
Mat<T,dim+1,dim+1>  asHomogMat(AffineEw<T,dim> a)
{
    Mat<T,dim+1,dim+1>  ret(0);
    for (uint ii=0; ii<dim; ++ii) {
        ret.rc(ii,ii) = a.m_scales[ii];
        ret.rc(ii,dim) = a.m_trans[ii];
    }
    ret.rc(dim,dim) = T(1);
    return ret;
}

// concatenate (dimensionally) two affine transforms; it will operate on respectivaly concatenated vectors:
template<class T,uint D0,uint D1>
AffineEw<T,D0+D1>   cat(AffineEw<T,D0> const & l,AffineEw<T,D1> const & r)
{
    return {cat(l.m_scales,r.m_scales),cat(l.m_trans,r.m_trans)};
}

template<uint dim>
AffineEw<double,dim> interpolate(AffineEw<double,dim> a0,AffineEw<double,dim> a1,double val)
{
    return AffineEw<double,dim> {
        mapExp(interpolate(mapLog(a0.m_scales),mapLog(a1.m_scales),val)),
        interpolate(a0.m_trans,a1.m_trans,val)
    };
}

template <class T,uint dim>
bool                isApproxEqual(AffineEw<T,dim> const & l,AffineEw<T,dim> const & r,T maxDiff)
{
    return  isApproxEqual(l.m_scales,r.m_scales,maxDiff) &&
            isApproxEqual(l.m_trans,r.m_trans,maxDiff);
}

}

#endif
