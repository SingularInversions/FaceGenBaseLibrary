//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Functions operating on Mat<>

#ifndef FGMATRIXC_HPP
#define FGMATRIXC_HPP

#include "FgStdLibs.hpp"
#include "FgMatrixCBase.hpp"
#include "FgMath.hpp"
#include "FgOpt.hpp"

namespace Fg {

template<class T,uint nrows,uint ncols>
T
cLen(Mat<T,nrows,ncols> const & m)
{return cLen(m.m); }

template<class T,uint nrows,uint ncols>
Mat<T,nrows,ncols>
operator*(T val,const Mat<T,nrows,ncols> & mat)
{return (mat * val); }

template <class T,uint nrows,uint ncols,uint ncols2>
Mat<T,nrows,ncols2>
operator*(
    const Mat<T,nrows,ncols> &    v1,
    const Mat<T,ncols,ncols2> &   v2)
{
    Mat<T,nrows,ncols2>      newMat;
    for (uint ii=0; ii<nrows; ii++) {
        for (uint jj=0; jj<ncols2; jj++) {
            newMat[ii*ncols2+jj] = T(0);
            for (uint kk=0; kk<ncols; kk++)
                newMat[ii*ncols2+jj] += v1[ii*ncols+kk] * v2[kk*ncols2+jj];
        }
    }
    return newMat;
}

template<class T,uint nrows,uint ncols>
Mat<T,nrows,ncols>
expFast(Mat<T,nrows,ncols> const & m)
{
    Mat<T,nrows,ncols>  ret;
    for (uint ii=0; ii<nrows*ncols; ++ii)
        ret.m[ii] = expFast(m.m[ii]);
    return ret;
}

template <class T>
Mat<T,2,2>
matRotate(T radians)
{
    Mat<T,2,2> mat;
    T ct = T(cos(radians));
    T st = T(sin(radians));
    mat.rc(0,0)=ct;    mat.rc(0,1)=-st;
    mat.rc(1,0)=st;    mat.rc(1,1)=ct;
    return mat;
}

template <class T>
Mat<T,3,3>
matRotateX(T radians)        // RHR rotation around X axis
{
    Mat<T,3,3> mat;
    T ct = (T)cos(radians);
    T st = (T)sin(radians);
    mat.rc(0,0)=1.0;   mat.rc(0,1)=0.0;   mat.rc(0,2)=0.0;
    mat.rc(1,0)=0.0;   mat.rc(1,1)=ct;    mat.rc(1,2)=-st;
    mat.rc(2,0)=0.0;   mat.rc(2,1)=st;    mat.rc(2,2)=ct;
    return mat;
}

template <class T>
Mat<T,3,3>
matRotateY(T radians)        // RHR rotation around Y axis
{
    Mat<T,3,3> mat;
    T ct = (T)cos(radians);
    T st = (T)sin(radians);
    mat.rc(0,0)=ct;    mat.rc(0,1)=0.0;   mat.rc(0,2)=st;
    mat.rc(1,0)=0.0;   mat.rc(1,1)=1.0;   mat.rc(1,2)=0.0;
    mat.rc(2,0)=-st;   mat.rc(2,1)=0.0;   mat.rc(2,2)=ct;
    return mat;
}

template <class T>
Mat<T,3,3>
matRotateZ(T radians)        // RHR rotation around Z axis
{
    Mat<T,3,3> mat;
    T ct = (T)cos(radians);
    T st = (T)sin(radians);
    mat.rc(0,0)=ct;    mat.rc(0,1)=-st;   mat.rc(0,2)=0.0;
    mat.rc(1,0)=st;    mat.rc(1,1)=ct;    mat.rc(1,2)=0.0;
    mat.rc(2,0)=0.0;   mat.rc(2,1)=0.0;   mat.rc(2,2)=1.0;
    return mat;
}

template <class T>
Mat<T,3,3>
matRotateAxis(                   // RHR rotation around an arbitrary axis
    T           radians, 
    const       Mat<T,3,1> &axis)             // vector describing that axis
{
    Mat<T,3,3> mat;
    T           len = axis.len();
    FGASSERT(len > T(0));
    Mat<T,3,1>    ax = axis * (T(1) / len);   // Ensure the rotation axis is normalized
    T           ct = (T)cos(radians);
    T           st = (T)sin(radians);
    T           vt = T(1)-ct;
    mat.rc(0,0) = ax[0]*ax[0]*vt + ct;
    mat.rc(0,1) = ax[0]*ax[1]*vt - ax[2]*st;
    mat.rc(0,2) = ax[0]*ax[2]*vt + ax[1]*st;
    mat.rc(1,0) = ax[0]*ax[1]*vt + ax[2]*st;
    mat.rc(1,1) = ax[1]*ax[1]*vt + ct;
    mat.rc(1,2) = ax[1]*ax[2]*vt - ax[0]*st;
    mat.rc(2,0) = ax[0]*ax[2]*vt - ax[1]*st;
    mat.rc(2,1) = ax[1]*ax[2]*vt + ax[0]*st;
    mat.rc(2,2) = ax[2]*ax[2]*vt + ct;
    return mat;
}

template<class T>
T
determinant(const Mat<T,2,2> & mat)
{return (mat[0]*mat[3] - mat[1]*mat[2]); }

// Useful for finding the discriminant, for instance of aspect ratios:
template<class T>
T
determinant(const Mat<T,2,1> & col0,const Mat<T,2,1> & col1)
{return col0[0]*col1[1] - col0[1]*col1[0]; }

template<class T>
T
determinant(const Mat<T,3,3> & mat)
{
    return (
        mat[0]*mat[4]*mat[8] +
        mat[1]*mat[5]*mat[6] +
        mat[2]*mat[3]*mat[7] -
        mat[2]*mat[4]*mat[6] -
        mat[1]*mat[3]*mat[8] -
        mat[0]*mat[5]*mat[7]);
}

// Concatenate an element onto a column Svec:
template<class T,uint dim>
Mat<T,dim+1,1>
cat(const Mat<T,dim,1> & vec,T val)
{
    Mat<T,dim+1,1>    ret;
    for (uint ii=0; ii<dim; ++ii)
        ret[ii] = vec[ii];
    ret[dim] = val;
    return ret;
}
// Concatenate an element onto a row Svec:
template<class T,uint dim>
Mat<T,1,dim+1>
cat(const Mat<T,1,dim> & vec,T val)
{
    Mat<T,1,dim+1>    ret;
    for (uint ii=0; ii<dim; ++ii)
        ret[ii] = vec[ii];
    ret[dim] = val;
    return ret;
}

// Flatten a Svec of matrices into a Svec of scalars:
template<class T,uint nrows,uint ncols>
Svec<T>
flatten(const Svec<Mat<T,nrows,ncols> > & ms)
{
    Svec<T>       ret;
    ret.reserve(ms.size()*nrows*ncols);
    for (size_t ii=0; ii<ms.size(); ++ii)
        for (uint jj=0; jj<nrows*ncols; ++jj)
            ret.push_back(ms[ii].m[jj]);
    return ret;
}

inline Doubles
toDoubles(Doubless const & v)
{return flatten(v); }

Doubles
toDoubles(Floatss const & v);

template<class T,uint nrows,uint ncols>
Doubles
toDoubles(Mat<T,nrows,ncols> const & mat)
{
    Doubles         ret;
    ret.reserve(mat.numElems());
    for (T e : mat.m)
        ret.push_back(scast<double>(e));
    return ret;
}

template<class T,uint nrows,uint ncols>
Doubles
toDoubles(const Svec<Mat<T,nrows,ncols> > & ms)
{
    Doubles         ret;
    ret.reserve(ms.size()*nrows*ncols);
    for (Mat<T,nrows,ncols> const & m : ms)
        for (T e : m.m)
            ret.push_back(scast<double>(e));
    return ret;
}

template<class T,uint nrows,uint ncols>
Doubles
toDoubles(const Svec<Svec<Mat<T,nrows,ncols> > > & mss)
{
    size_t          sz = 0;
    for (Svec<Mat<T,nrows,ncols> > const & ms : mss)
        sz += ms.size() * nrows * ncols;
    Doubles         ret;
    ret.reserve(sz);
    for (Svec<Mat<T,nrows,ncols> > const & ms : mss)
        for (Mat<T,nrows,ncols> const & m : ms)
            for (T e : m.m)
                ret.push_back(scast<double>(e));
    return ret;
}

template<class T,uint dim>
Mat<T,dim+1,1>
asHomogVec(Mat<T,dim,1> v)
{
    Mat<T,dim+1,1>    ret;
    for (uint ii=0; ii<dim; ++ii)
        ret[ii] = v[ii];
    ret[dim] = T(1);
    return ret;
}

template<class T,uint dim>
Mat<T,dim-1,1>
fromHomogVec(Mat<T,dim,1> v)
{
    Mat<T,dim-1,1>    ret;
    T                 w = v[dim-1];
    FGASSERT(w != T(0));
    for (uint ii=0; ii<dim-1; ++ii)
        ret[ii] = v[ii] / w;
    return ret;
}

// Return homogeneous matrix representation of an affine transform:
template<class T, uint dims>
Mat<T,dims+1,dims+1>
asHomogMat(
    const Mat<T,dims,dims>  & linTrans,
    const Mat<T,dims,1>     & translation)
{
    Mat<T,dims+1,dims+1>    ret;
    for (uint rr=0; rr<dims; rr++)
        for (uint cc=0; cc<dims; cc++)
            ret.rc(rr,cc) = linTrans.rc(rr,cc);
    for (uint rr=0; rr<dims; rr++)
        ret.rc(rr,dims) = translation[rr];
    ret.rc(dims,dims) = 1;
    return ret;
}

// Return homogeneous matrix representation of a linear transform:
template<class T, uint dims>
Mat<T,dims+1,dims+1>
asHomogMat(const Mat<T,dims,dims> & linear)
{
    Mat<T,dims+1,dims+1>    ret;
    for (uint rr=0; rr<dims; ++rr)
        for (uint cc=0; cc<dims; ++cc)
            ret.rc(rr,cc) = linear.rc(rr,cc);
    ret.rc(dims,dims) = T(1);
    return ret;
}

// Return homogeneous matrix representation of a translation:
template<class T, uint dims>
Mat<T,dims+1,dims+1>
asHomogMat(const Mat<T,dims,1> & translation)
{
    Mat<T,dims+1,dims+1>    ret;
    ret.setIdentity();
    for (uint rr=0; rr<dims; rr++)
        ret.rc(rr,dims) = translation[rr];
    return ret;
}

// RETURNS: The inverse of an invertible matrix. Throws an FGASSERT if not invertible.
template <class T>
Mat<T,2,2>
cInverse(Mat<T,2,2> const & m)
{
    static_assert(std::is_floating_point<T>::value,"Mat inverse requires floating point type");
    Mat<T,2,2>     ret;
    T   fac = (m.rc(0,0) * m.rc(1,1) - m.rc(0,1) * m.rc(1,0));
    FGASSERT(fac != T(0));
    fac = T(1) / fac;
    ret.rc(0,0) = m.rc(1,1) * fac;
    ret.rc(0,1) = - m.rc(0,1) * fac;
    ret.rc(1,0) = - m.rc(1,0) * fac;
    ret.rc(1,1) = m.rc(0,0) * fac;
    return ret;
}
template <class T>
Mat<T,3,3>
cInverse(Mat<T,3,3> const & m)
{
    static_assert(std::is_floating_point<T>::value,"Mat inverse requires floating point type");
    Mat<T,3,3>     r;
    T   fac = (m.rc(0,0)*m.rc(1,1)*m.rc(2,2) - m.rc(0,0)*m.rc(1,2)*m.rc(2,1) +
               m.rc(1,0)*m.rc(0,2)*m.rc(2,1) - m.rc(1,0)*m.rc(0,1)*m.rc(2,2) +
               m.rc(2,0)*m.rc(0,1)*m.rc(1,2) - m.rc(2,0)*m.rc(1,1)*m.rc(0,2));
    FGASSERT(fac != T(0));
    r.rc(0,0) = m.rc(1,1) * m.rc(2,2) - m.rc(1,2) * m.rc(2,1);
    r.rc(0,1) = m.rc(0,2) * m.rc(2,1) - m.rc(0,1) * m.rc(2,2);
    r.rc(0,2) = m.rc(0,1) * m.rc(1,2) - m.rc(0,2) * m.rc(1,1);
    r.rc(1,0) = m.rc(1,2) * m.rc(2,0) - m.rc(1,0) * m.rc(2,2);
    r.rc(1,1) = m.rc(0,0) * m.rc(2,2) - m.rc(0,2) * m.rc(2,0);
    r.rc(1,2) = m.rc(0,2) * m.rc(1,0) - m.rc(0,0) * m.rc(1,2);
    r.rc(2,0) = m.rc(1,0) * m.rc(2,1) - m.rc(1,1) * m.rc(2,0);
    r.rc(2,1) = m.rc(0,1) * m.rc(2,0) - m.rc(0,0) * m.rc(2,1);
    r.rc(2,2) = m.rc(0,0) * m.rc(1,1) - m.rc(1,0) * m.rc(0,1);
    r *= T(1) / fac;
    return r;
}

template <class T,uint nrows,uint ncols>
T
cDot(
    const Mat<T,nrows,ncols> & lhs,
    const Mat<T,nrows,ncols> & rhs)
{
    T           acc(0);
    for (uint ii=0; ii<nrows*ncols; ++ii)
        acc += lhs[ii] * rhs[ii];
    return acc;
}

template<typename T>
Mat<T,3,1>
crossProduct(
    const Mat<T,3,1> & v1,
    const Mat<T,3,1> & v2)
{
    Mat<T,3,1>      r;
    r[0] = v1[1] * v2[2] - v1[2] * v2[1];
    r[1] = v1[2] * v2[0] - v1[0] * v2[2];
    r[2] = v1[0] * v2[1] - v1[1] * v2[0];
    return r;
}

// Element-wise multiplication (aka Hadamard product):
template<typename T,uint nrows,uint ncols>
Mat<T,nrows,ncols>
mapMul(
    const Mat<T,nrows,ncols> &    lhs,
    const Mat<T,nrows,ncols> &    rhs)
{
    Mat<T,nrows,ncols>    ret;
    for (uint ii=0; ii<nrows*ncols; ++ii)
        ret[ii] = lhs[ii] * rhs[ii];
    return ret;
}
template<typename T,uint nrows,uint ncols>
Mat<T,nrows,ncols>
mapMul(const Mat<T,nrows,ncols> & m0,const Mat<T,nrows,ncols> & m1,const Mat<T,nrows,ncols> & m2)
{
    Mat<T,nrows,ncols>    ret;
    for (uint ii=0; ii<nrows*ncols; ++ii)
        ret[ii] = m0[ii] * m1[ii] * m2[ii];
    return ret;
}

// Faster equivalent to lhs^T * rhs:
template<typename T,uint n0,uint n1,uint n2>
Mat<T,n0,n1>
fgTransposeMul(
    const Mat<T,n2,n0> &    lhs,
    const Mat<T,n2,n1> &    rhs)
{
    Mat<T,n0,n1>      ret(T(0));
    for (uint i0=0; i0<n0; ++i0)
        for (uint i1=0; i1<n1; ++i1)
            for (uint i2=0; i2<n2; ++i2)
                ret.rc(i0,i1) += lhs.rc(i2,i0) * rhs.rc(i2,i1);
    return ret;
}

// Element-wise division:
template<typename T,uint nrows,uint ncols>
Mat<T,nrows,ncols>
mapDiv(
    const Mat<T,nrows,ncols> &    lhs,
    const Mat<T,nrows,ncols> &    rhs)
{
    Mat<T,nrows,ncols>            ret;
    for (uint ii=0; ii<nrows*ncols; ++ii)
        ret[ii] = lhs[ii] / rhs[ii];
    return ret;
}

double  randUniform(double lo,double hi);

double  randNormal();

template<typename T,uint nrows,uint ncols>
Mat<T,nrows,ncols> 
Mat<T,nrows,ncols>::randNormal(T stdev)
{
    Mat<T,nrows,ncols>        ret;
    for (size_t ii=0; ii<nrows*ncols; ++ii)
        ret[ii] = static_cast<T>(Fg::randNormal())*stdev;
    return ret;
}

template<typename T,uint nrows,uint ncols>
Mat<T,nrows,ncols>
Mat<T,nrows,ncols>::randUniform(T lo,T hi)
{
    Mat<T,nrows,ncols>    ret;
    for (size_t ii=0; ii<nrows*ncols; ++ii)
        ret[ii] = static_cast<T>(Fg::randUniform(double(lo),double(hi)));
    return ret;
}

template<typename T,uint nrows,uint ncols>
Mat<T,nrows,ncols>
cFloor(const Mat<T,nrows,ncols> & mat)
{
    Mat<T,nrows,ncols> ret;
    for (uint ii=0; ii<mat.numElems(); ++ii)
        ret[ii] = std::floor(mat[ii]);
    return ret;
}

template<typename Flt,typename Int,uint nrows,uint ncols>
void
round_(
    Mat<Flt,nrows,ncols> const &    lhs,
    Mat<Int,nrows,ncols> &          rhs)
{
    for (uint ii=0; ii<rhs.numElems(); ++ii)
        round_(lhs[ii],rhs[ii]);
}

template<typename To,typename From,uint nrows,uint ncols>
Mat<To,nrows,ncols>
round(Mat<From,nrows,ncols> const & m)
{
    Mat<To,nrows,ncols>     ret;
    for (uint ii=0; ii<m.numElems(); ++ii)
        ret[ii] = round<To>(m[ii]);
    return ret;
}

template<uint nrows,uint ncols>
Mat<uint,nrows,ncols>
pow2Ceil(Mat<uint,nrows,ncols> m)
{
    Mat<uint,nrows,ncols> ret;
    for (uint ii=0; ii<m.numElems(); ++ii)
        ret[ii] = pow2Ceil(m[ii]);
    return ret;
}

// Create a wider matrix by concatenating rows from 2 matrices:
template<class T,uint nrows,uint ncols1,uint ncols2>
Mat<T,nrows,ncols1+ncols2>
fgJoinHoriz(
    const Mat<T,nrows,ncols1> & lhs,
    const Mat<T,nrows,ncols2> & rhs)
{
    Mat<T,nrows,ncols1+ncols2>    ret;
    for (uint row=0; row<nrows; ++row)
    {
        uint    col=0;
        for (uint cc=0; cc<ncols1; ++cc)
            ret.rc(row,col++) = lhs.rc(row,cc);
        for (uint cc=0; cc<ncols2; ++cc)
            ret.rc(row,col++) = rhs.rc(row,cc);
    }
    return ret;
}

// Create a wider matrix by concatenating rows from 3 matrices:
template<class T,uint nrows,uint ncols1,uint ncols2,uint ncols3>
Mat<T,nrows,ncols1+ncols2+ncols3>
fgJoinHoriz(
    const Mat<T,nrows,ncols1> & m1,
    const Mat<T,nrows,ncols2> & m2,
    const Mat<T,nrows,ncols3> & m3)
{
    Mat<T,nrows,ncols1+ncols2+ncols3> ret;
    for (uint row=0; row<nrows; ++row)
    {
        uint    col=0;
        for (uint cc=0; cc<ncols1; ++cc)
            ret.rc(row,col++) = m1.rc(row,cc);
        for (uint cc=0; cc<ncols2; ++cc)
            ret.rc(row,col++) = m2.rc(row,cc);
        for (uint cc=0; cc<ncols3; ++cc)
            ret.rc(row,col++) = m3.rc(row,cc);
    }
    return ret;
}

// Create a taller matrix by concatenating cols from 2 matrices:
template<class T,uint nrows1,uint nrows2,uint ncols>
Mat<T,nrows1+nrows2,ncols>
fgJoinVert(
    const Mat<T,nrows1,ncols> & upper,
    const Mat<T,nrows2,ncols> & lower)
{
    Mat<T,nrows1+nrows2,ncols>    ret;
    for (uint rr=0; rr<nrows1; ++rr)
        for (uint cc=0; cc<ncols; ++cc)
            ret.rc(rr,cc) = upper.rc(rr,cc);
    for (uint rr=0; rr<nrows2; ++rr)
        for (uint cc=0; cc<ncols; ++cc)
            ret.rc(nrows1+rr,cc) = lower.rc(rr,cc);
    return ret;
}

// Create a taller matrix by concatenating a given value to all columns:
template<class T,uint nrows,uint ncols>
Mat<T,nrows+1,ncols>
fgJoinVert(const Mat<T,nrows,ncols> & mat,T val)
{
    Mat<T,nrows+1,ncols>  ret;
    uint    ii=0;
    for (; ii<nrows*ncols; ++ii)
        ret[ii] = mat[ii];
    ret[ii] = val;
    return ret;
}

// Parameterized unmirrored permutation of axes in 3D:
template<class T>
Mat<T,3,3>
fgPermuteAxes(uint axisToBecomeX)
{
    Mat<T,3,3>    ret;
    for (uint ii=0; ii<3; ++ii)
        ret.rc(ii,(ii+axisToBecomeX)%3) = T(1);
    return ret;
}

template<class T,uint nrows,uint ncols>
bool
isPow2(const Mat<T,nrows,ncols> & mat)
{
    for (uint ii=0; ii<nrows*ncols; ++ii)
        if (!isPow2(mat[ii]))
            return false;
    return true;
}

template<class T,uint nrows,uint ncols>
T
cMean(const Mat<T,nrows,ncols> & mat)
{
    typedef typename Traits<T>::Accumulator Acc;
    typedef typename Traits<T>::Scalar      Scal;
    Acc     acc = Acc(mat[0]);
    for (uint ii=1; ii<nrows*ncols; ++ii)
        acc += Acc(mat[ii]);
    return T(acc / Scal(nrows*ncols));
}

#define FG_MATRIXC_ELEMWISE(matFunc,elemFunc)               \
    template<class T,uint nrows,uint ncols>                 \
    Mat<T,nrows,ncols>                                \
    matFunc (const Mat<T,nrows,ncols> & mat)          \
    {                                                       \
        Mat<T,nrows,ncols>    ret;                    \
        for (uint ii=0; ii<nrows*ncols; ++ii)               \
            ret[ii] = elemFunc (mat[ii]);                   \
        return ret;                                         \
    }

FG_MATRIXC_ELEMWISE(pos2Floor,pos2Floor)
FG_MATRIXC_ELEMWISE(pow2Ceil,pow2Ceil)
FG_MATRIXC_ELEMWISE(mapAbs,std::abs)
FG_MATRIXC_ELEMWISE(mapLog,std::log)
FG_MATRIXC_ELEMWISE(mapExp,std::exp)

template<class T,uint nrows,uint ncols>
double
cDot(
    const Svec<Mat<T,nrows,ncols> > & v0,
    const Svec<Mat<T,nrows,ncols> > & v1)
{
    FGASSERT(v0.size() == v1.size());
    double  acc(0);
    for (size_t ii=0; ii<v0.size(); ++ii)
        acc += cDot(v0[ii],v1[ii]);
    return acc;
}

// Weighted dot product:
template<class T,uint nrows,uint ncols>
double
fgDotWgt(
    const Svec<Mat<T,nrows,ncols> > & v0,
    const Svec<Mat<T,nrows,ncols> > & v1,
    const Svec<T> &                         w)    // Weight to apply to each dot product
{
    FGASSERT(v0.size() == v1.size());
    FGASSERT(v0.size() == w.size());
    double  acc(0);
    for (size_t ii=0; ii<v0.size(); ++ii)
        acc += cDot(v0[ii],v1[ii]) * w[ii];
    return acc;
}

template<uint nrows,uint ncols>
Mat<bool,nrows,ncols>
fgOr(Mat<bool,nrows,ncols> v0,Mat<bool,nrows,ncols> v1)
{
    Mat<bool,nrows,ncols> ret;
    for (uint ii=0; ii<nrows*ncols; ++ii)
        ret[ii] = v0[ii] || v1[ii];
    return ret;
}

template<typename T,uint dim>
T
fgTrace(const Mat<T,dim,dim> & m)
{
    T   ret(0);
    for (uint ii=0; ii<m.numElems(); ii+=dim+1)
        ret += m[ii];
    return ret;
}

// For Mat/Vec use linear interpolation
template<uint nrows,uint ncols>
Mat<double,nrows,ncols>
interpolate(Mat<double,nrows,ncols> m0,Mat<double,nrows,ncols> m1,double val)   // val [0,1]
{return m0*(1.0-val) + m1*(val); }

template<typename Flt,typename Int,uint dim>
void
fgUninterpolate(
    const Mat<Flt,dim,1> &    coord,
    Mat<Int,dim,1> &          coordLo,    // Returned
    Mat<Flt,dim,2> &          weights)    // Returned
{
    Mat<Flt,dim,1>    coordL = cFloor(coord),
                            coordH = coordL + Mat<Flt,dim,1>(1);
    weights = fgJoinHoriz(coordH-coord,coord-coordL);
    coordLo = Mat<Int,dim,1>(coordL);
}

// Provide an ordering by axis order
// (without making it the default operator as this may not be desired):
template<typename T,uint nrows,uint ncols>
bool
fgLt(
    const Mat<T,nrows,ncols> & m0,
    const Mat<T,nrows,ncols> & m1)
{
    for (uint ii=0; ii<nrows*ncols; ++ii)
        if (!(m0[ii] < m1[ii]))
            return false;
    return true;
}

template<class T,uint nrows,uint ncols>
T
fgSumElems(const Mat<T,nrows,ncols> & m)
{
    T   ret(m[0]);
    for (uint ii=1; ii<nrows*ncols; ++ii)
        ret += m[ii];
    return ret;
}

template<class T,uint sz>
Mat<T,sz,sz>
fgDiagonal(Mat<T,sz,1> vec)
{
    Mat<T,sz,sz>      ret(static_cast<T>(0));
    for (uint ii=0; ii<sz; ++ii)
        ret.rc(ii,ii) = vec[ii];
    return ret;
}

template<class T>
Mat<T,2,2>
fgDiagonal(T v0,T v1)
{
    Mat<T,2,2>    ret(static_cast<T>(0));
    ret[0] = v0;
    ret[3] = v1;
    return ret;
}

template<class T>
Mat<T,3,3>
fgDiagonal(T v0,T v1,T v2)
{
    Mat<T,3,3>    ret(static_cast<T>(0));
    ret[0] = v0;
    ret[4] = v1;
    ret[8] = v2;
    return ret;
}

template<typename T,uint nrows,uint ncols>
Mat<T,nrows,ncols>
normalize(Mat<T,nrows,ncols> m)
{
    return m / m.len();
}

template<typename T,uint nrows,uint ncols>
Mat<T,nrows,ncols>
mapft(Mat<T,nrows,ncols> m,T(*func)(T))
{
    Mat<T,nrows,ncols>    ret;
    for (uint ii=0; ii<nrows*ncols; ++ii)
        ret[ii] = func(m[ii]);
    return ret;
}

template<class T,uint nrows,uint ncols>
Svec<T>
mapMag(const Svec<Mat<T,nrows,ncols> > & v)
{
    Svec<T>   ret(v.size());
    for (size_t ii=0; ii<v.size(); ++ii)
        ret[ii] = cMag(v[ii]);
    return ret;
}

// Find first index of an element in a Svec. Return 'size' if not found:
template<typename T,uint nrows>
uint
findFirstIdx(Mat<T,nrows,1> m,T v)
{
    for (uint ii=0; ii<nrows; ++ii)
        if (m[ii] == v)
            return ii;
    return nrows;
}

// Solve matrix equation of the form Ax = b. Returns x if solvable, invalid if degenerate:
Opt<Vec2F> fgSolve(Mat22F A,Vec2F b);
Opt<Vec3D> fgSolve(Mat33D A,Vec3D b);
inline
Opt<Vec3F> fgSolve(Mat33F A,Vec3F b) {return fgSolve(Mat33D(A),Vec3D(b)).cast<Vec3F>(); }
Opt<Vec4D> fgSolve(Mat44D A,Vec4D b);
inline
Opt<Vec4F> fgSolve(Mat44F A,Vec4F b) {return fgSolve(Mat44D(A),Vec4D(b)).cast<Vec4F>(); }

// Elements must all be non-zero:
template<uint nrows,uint ncols>
bool
fgIsIntegerSizeMultiple(Mat<uint,nrows,ncols> m0,Mat<uint,nrows,ncols> m1)
{
    if ((m0.m[0] == 0) || (m1.m[0] == 0))
        return false;
    if (m0.m[0] < m1.m[0])
        std::swap(m0,m1);
    if ((m0.m[0] % m1.m[0]) != 0)
        return false;
    uint    div = m0.m[0] / m1.m[0];
    for (uint dd=1; dd<nrows*ncols; ++dd) {
        if ((m0.m[dd] % m1.m[dd]) != 0)
            return false;
        if ((m0.m[dd] / m1.m[dd]) != div)
            return false;
    }
    return true;
}

template<typename T,uint nrows,uint ncols>
bool
fgNoZeros(Mat<T,nrows,ncols> m)
{
    T   acc = T(1);
    for (uint ii=0; ii<nrows*ncols; ++ii)
        acc *= m.m[ii];
    return (acc != T(0));
}

template<uint nrows,uint ncols>
Mat<double,nrows,ncols>
fgF2D(const Mat<float,nrows,ncols> & m)
{return Mat<double,nrows,ncols>(m); }

template<uint nrows,uint ncols>
Mat<float,nrows,ncols>
fgD2F(const Mat<double,nrows,ncols> & m)
{return Mat<float,nrows,ncols>(m); }

// Contract columns to row Svec:
template<class T,uint nrows,uint ncols>
Mat<T,1,ncols>
fgSumCols(const Mat<T,nrows,ncols> & m)
{
    Mat<T,1,ncols>    r(0);
    for (uint rr=0; rr<nrows; ++rr)
        for (uint cc=0; cc<ncols; ++cc)
            r[cc] += m.rc(rr,cc);
    return r;
}

// Contract rows to column Svec:
template<class T,uint nrows,uint ncols>
Mat<T,nrows,1>
fgSumRows(const Mat<T,nrows,ncols> & m)
{
    Mat<T,nrows,1>    r(0);
    for (uint rr=0; rr<nrows; ++rr)
        for (uint cc=0; cc<ncols; ++cc)
            r[rr] += m.rc(rr,cc);
    return r;
}

template<class T,uint nrows,uint ncols>
Mat<T,nrows,ncols>
mapSqr(Mat<T,nrows,ncols> m)
{
    Mat<T,nrows,ncols>    r;
    for (uint ii=0; ii<nrows*ncols; ++ii)
        r[ii] = sqr(m[ii]);
    return r;
}

template<class T,uint nrows,uint ncols>
double
cMag(Mat<T,nrows,ncols> m)
{return m.mag(); }

template<class T,uint nrows,uint ncols>
double
cRms(Mat<T,nrows,ncols> m)
{return std::sqrt(m.mag()/static_cast<double>(nrows*ncols)); }

template<typename T,uint nrows,uint ncols>
inline T
cLen(const Svec<Mat<T,nrows,ncols> > & v)
{return std::sqrt(cMag(v)); }

template<uint nrows,uint ncols>
Mat<double,nrows,ncols>
fgReal(const Mat<std::complex<double>,nrows,ncols> & m)   // Return real compoments
{
    Mat<double,nrows,ncols>   ret;
    for (uint ii=0; ii<nrows*ncols; ++ii)
        ret[ii] = m[ii].real();
    return ret;
}

template<uint nrows,uint ncols>
Mat<double,nrows,ncols>
fgImag(const Mat<std::complex<double>,nrows,ncols> & m)   // Return imaginary compoments
{
    Mat<double,nrows,ncols>   ret;
    for (uint ii=0; ii<nrows*ncols; ++ii)
        ret[ii] = m[ii].imag();
    return ret;
}

// Give a tangent coordinate system for a point on a sphere centred at origin:
Mat32D
fgTanSphere(Vec3D p);

template<class T,class U,uint nrows,uint ncols>
void
scast_(const Mat<T,nrows,ncols> & i,Mat<U,nrows,ncols> & o)
{
    for (size_t ii=0; ii<i.numElems(); ++ii)
        scast_(i[ii],o[ii]);
}

// Transpose a matrix stored as an array of arrays. All sub-arrays must have same size:
template<class T,uint nrows,uint ncols>
Mat<Svec<T>,nrows,ncols>
transpose(const Svec<Mat<T,nrows,ncols> > & v)
{
    Mat<Svec<T>,nrows,ncols>   ret;
    for (uint ii=0; ii<nrows*ncols; ++ii)
        ret[ii].resize(v.size());
    for (size_t jj=0; jj<v.size(); ++jj) {
        for (uint ii=0; ii<nrows*ncols; ++ii)
            ret[ii][jj] = v[jj][ii];
    }
    return ret;
}

// Return a vector normal to 'unitMag' (which must have .mag() == 1) by Gram-Schmidt of 'seed'.
// Resulting value is not normalized. 'seed' must not be parallel to 'unitMag'.
template<class T,uint dim>
Mat<T,dim,1>
orthogonalize(Mat<T,dim,1> seed,Mat<T,dim,1> unitMag)
{return seed - dotProd(seed,unitMag)*unitMag; }

template<uint dim>
bool
fgIsValidPermutation(Mat<uint,dim,1> perm)
{
    Mat<uint,dim,1>  chk(0);
    for (uint dd=0; dd<dim; ++dd) {
        if (perm[dd] >= dim)
            return false;
        chk[perm[dd]] = 1;
    }
    if (cMinElem(chk) == 1)
        return true;
    return false;
}

template<class T,uint dim>
Mat<T,dim,1>
fgPermute(const Mat<T,dim,1> & v,Mat<uint,dim,1> perm)  // Assumes a valid permutation, use above to check
{
    Mat<T,dim,1>      ret;
    for (uint dd=0; dd<dim; ++dd)
        ret[dd] = v[perm[dd]];
    return ret;
}

template<class T,uint nrows,uint ncols>
Mat<T,ncols,nrows>
fgHermitian(const Mat<T,nrows,ncols> & mat)
{
    Mat<T,ncols,nrows>        ret;
    for (uint rr=0; rr<nrows; ++rr)
        for (uint cc=0; cc<ncols; ++cc)
            ret.rc(cc,rr) = std::conj(mat.rc(rr,cc));
    return ret;
}

template<class Xform,class T,uint nrows,uint ncols>
Svec<Mat<T,nrows,ncols> >
operator*(
    const Xform &                                   xform,
    const Svec<Mat<T,nrows,ncols> > &  rhs)
{
    Svec<Mat<T,nrows,ncols> >  ret(rhs.size());
    mapXf_(rhs,xform,ret);
    return ret;
}

}

#endif
