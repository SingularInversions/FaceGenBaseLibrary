//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
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
#include "FgRandom.hpp"

namespace Fg {

template<class T,uint R,uint C>
T
cSsd(Mat<T,R,C> const & l,Mat<T,R,C> const & r)
{return cSsd(l.m,r.m); }

template<class T,uint R,uint C>
T
cLen(Mat<T,R,C> const & m)
{return cLen(m.m); }

template<class T,uint R,uint C>
Mat<T,R,C>
operator*(T val,Mat<T,R,C> const & mat)
{return (mat * val); }

template <class T,uint R,uint C,uint C2>
Mat<T,R,C2>
operator*(
    Mat<T,R,C> const &    v1,
    const Mat<T,C,C2> &   v2)
{
    Mat<T,R,C2>      newMat;
    for (uint ii=0; ii<R; ii++) {
        for (uint jj=0; jj<C2; jj++) {
            newMat[ii*C2+jj] = T(0);
            for (uint kk=0; kk<C; kk++)
                newMat[ii*C2+jj] += v1[ii*C+kk] * v2[kk*C2+jj];
        }
    }
    return newMat;
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

// Create matrix for RHR around an arbitrary axis:
Mat33D          matRotateAxis(double radians,Vec3D const & normalizedZxis);

template<class T>
T
cDeterminant(const Mat<T,2,2> & mat)
{return (mat[0]*mat[3] - mat[1]*mat[2]); }

// Useful for finding if aspect ratios match, or sine of angle between normalized vectors:
template<class T>
T
cDeterminant(const Mat<T,2,1> & col0,const Mat<T,2,1> & col1)
{return col0[0]*col1[1] - col0[1]*col1[0]; }

template<class T>
T
cDeterminant(Mat<T,3,3> const & M)
{
    return
        M[0]*M[4]*M[8] +
        M[1]*M[5]*M[6] +
        M[2]*M[3]*M[7] -
        M[2]*M[4]*M[6] -
        M[1]*M[3]*M[8] -
        M[0]*M[5]*M[7];
}

template<class T>
T
cDeterminant(Mat<T,4,4> const & M)
{
    // This may not be the fastest method:
    double          d0 = cDeterminant(Mat<T,3,3>{
        M.rc(1,1),  M.rc(1,2),  M.rc(1,3),
        M.rc(2,1),  M.rc(2,2),  M.rc(2,3),
        M.rc(3,1),  M.rc(3,2),  M.rc(3,3),
    }),
                    d1 = cDeterminant(Mat<T,3,3>{
        M.rc(1,0),  M.rc(1,2),  M.rc(1,3),
        M.rc(2,0),  M.rc(2,2),  M.rc(2,3),
        M.rc(3,0),  M.rc(3,2),  M.rc(3,3),
    }),
                    d2 = cDeterminant(Mat<T,3,3>{
        M.rc(1,0),  M.rc(1,1),  M.rc(1,3),
        M.rc(2,0),  M.rc(2,1),  M.rc(2,3),
        M.rc(3,0),  M.rc(3,1),  M.rc(3,3),
    }),
                    d3 = cDeterminant(Mat<T,3,3>{
        M.rc(1,0),  M.rc(1,1),  M.rc(1,2),
        M.rc(2,0),  M.rc(2,1),  M.rc(2,2),
        M.rc(3,0),  M.rc(3,1),  M.rc(3,2),
    });
    return M[0]*d0 - M[1]*d1 + M[2]*d2 - M[3]*d3;
}

// Concatenate an element onto a column Svec:
template<class T,uint dim>
Mat<T,dim+1,1>
cat(Mat<T,dim,1> const & vec,T val)
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

template<class T,uint R,uint C>
void
cat_(Svec<T> & l,Mat<T,R,C> const & r)
{
    for (uint ii=0; ii<R*C; ++ii)
        l.push_back(r[ii]);
}

// Flatten a Svec of matrices into a Svec of scalars:
template<class T,uint R,uint C>
Svec<T>
flatten(Svec<Mat<T,R,C>> const & ms)
{
    Svec<T>       ret;
    ret.reserve(ms.size()*R*C);
    for (size_t ii=0; ii<ms.size(); ++ii)
        for (uint jj=0; jj<R*C; ++jj)
            ret.push_back(ms[ii].m[jj]);
    return ret;
}

inline Doubles
toDoubles(Doubless const & v)
{return flatten(v); }

Doubles
toDoubles(Floatss const & v);

template<class T,uint R,uint C>
Doubles
toDoubles(Mat<T,R,C> const & mat)
{
    Doubles         ret;
    ret.reserve(mat.numElems());
    for (T e : mat.m)
        ret.push_back(scast<double>(e));
    return ret;
}

template<class T,uint R,uint C>
Doubles
toDoubles(Svec<Mat<T,R,C>> const & ms)
{
    Doubles         ret;
    ret.reserve(ms.size()*R*C);
    for (Mat<T,R,C> const & m : ms)
        for (T e : m.m)
            ret.push_back(scast<double>(e));
    return ret;
}

template<class T,uint R,uint C>
Doubles
toDoubles(const Svec<Svec<Mat<T,R,C>>> & mss)
{
    size_t          sz = 0;
    for (Svec<Mat<T,R,C>> const & ms : mss)
        sz += ms.size() * R * C;
    Doubles         ret;
    ret.reserve(sz);
    for (Svec<Mat<T,R,C>> const & ms : mss)
        for (Mat<T,R,C> const & m : ms)
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
    auto            ret = Mat<T,dims+1,dims+1>::identity();
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

template <class T,uint R,uint C>
T
cDot(Mat<T,R,C> const & lhs,Mat<T,R,C> const & rhs)
{
    return cDot(lhs.m,rhs.m);
}

template <class T,uint R,uint C>
double
cCos(Mat<T,R,C> const & lhs,Mat<T,R,C> const & rhs)
{
    double      mag = cMag(lhs) * cMag(rhs);
    FGASSERT(mag > 0.0);
    return cDot(lhs,rhs)/sqrt(mag);
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

// Equivalent to V * Y.transpose() (but more efficient and succinct):
template<typename T,uint R,uint C>
Mat<T,R,C>
outerProduct(Mat<T,R,1> const & lhs,Mat<T,C,1> const & rhs)
{
    Mat<T,R,C>      ret;
    for (uint rr=0; rr<R; ++rr)
        for (uint cc=0; cc<C; ++cc)
            ret.rc(rr,cc) = lhs[rr] * rhs[cc];
    return ret;
}

// UNARY & BINARY MAP OPERATIONS:

// Type-preserving binary callable:
template<class T,uint R,uint C,class F>
Mat<T,R,C>
mapCall(Mat<T,R,C> const & lhs,Mat<T,R,C> const & rhs,F func)
{
    Mat<T,R,C>          ret;
    for (size_t ii=0; ii<R*C; ++ii)
        ret[ii] = func(lhs,rhs);
    return ret;
}

// Subtract same value from each element:
template<typename T,uint R,uint C>
Mat<T,R,C>
mapSub(Mat<T,R,C> const & lhs,T const & rhs)
{return Mat<T,R,C>{mapSub(lhs.m,rhs)}; }

// Element-wise multiplication (aka Hadamard product):
template<typename T,uint R,uint C>
Mat<T,R,C>
mapMul(
    Mat<T,R,C> const &    lhs,
    Mat<T,R,C> const &    rhs)
{
    Mat<T,R,C>    ret;
    for (uint ii=0; ii<R*C; ++ii)
        ret[ii] = lhs[ii] * rhs[ii];
    return ret;
}
template<typename T,uint R,uint C>
Mat<T,R,C>
mapMul(Mat<T,R,C> const & m0,Mat<T,R,C> const & m1,Mat<T,R,C> const & m2)
{
    Mat<T,R,C>    ret;
    for (uint ii=0; ii<R*C; ++ii)
        ret[ii] = m0[ii] * m1[ii] * m2[ii];
    return ret;
}

// Element-wise division:
template<typename T,uint R,uint C>
Mat<T,R,C>
mapDiv(
    Mat<T,R,C> const &    lhs,
    Mat<T,R,C> const &    rhs)
{
    Mat<T,R,C>            ret;
    for (uint ii=0; ii<R*C; ++ii)
        ret[ii] = lhs[ii] / rhs[ii];
    return ret;
}

template<typename T,uint R,uint C>
Mat<T,R,C>
mapCall(Mat<T,R,C> m,T(*func)(T))
{
    Mat<T,R,C>    ret;
    for (uint ii=0; ii<R*C; ++ii)
        ret[ii] = func(m[ii]);
    return ret;
}

template<class T,uint R,uint C>
Svec<T>
mapMag(Svec<Mat<T,R,C>> const & v)
{
    Svec<T>   ret(v.size());
    for (size_t ii=0; ii<v.size(); ++ii)
        ret[ii] = cMag(v[ii]);
    return ret;
}

template<class T,uint R,uint C>
Mat<T,R,C>
mapSqr(Mat<T,R,C> m)
{
    Mat<T,R,C>    r;
    for (uint ii=0; ii<R*C; ++ii)
        r[ii] = sqr(m[ii]);
    return r;
}

template<typename T,uint R,uint C>
Mat<T,R,C>
mapFloor(Mat<T,R,C> const & mat)
{
    Mat<T,R,C> ret;
    for (uint ii=0; ii<mat.numElems(); ++ii)
        ret[ii] = std::floor(mat[ii]);
    return ret;
}

template<typename Flt,typename Int,uint R,uint C>
void
round_(
    Mat<Flt,R,C> const &    lhs,
    Mat<Int,R,C> &          rhs)
{
    for (uint ii=0; ii<rhs.numElems(); ++ii)
        round_(lhs[ii],rhs[ii]);
}

template<typename To,typename From,uint R,uint C>
Mat<To,R,C>
mapRound(Mat<From,R,C> const & m)
{
    Mat<To,R,C>     ret;
    for (uint ii=0; ii<m.numElems(); ++ii)
        ret[ii] = round<To>(m[ii]);
    return ret;
}

template<uint R,uint C>
Mat<uint,R,C>
mapPow2Ceil(Mat<uint,R,C> const & mat)
{
    Mat<uint,R,C>       ret;
    for (uint ii=0; ii<mat.numElems(); ++ii)
        ret[ii] = pow2Ceil(mat[ii]);
    return ret;
}

template<class T,uint R,uint C>
Mat<T,R,C>
mapMax(Mat<T,R,C> const & lhs,Mat<T,R,C> const & rhs)
{
    struct Max { T operator()(T l,T r) {return std::max(l,r); } };
    return Mat<T,R,C>{mapCall(lhs.m,rhs.m,Max{})};
}

#define FG_MATRIXC_ELEMWISE(matFunc,elemFunc)               \
    template<class T,uint R,uint C>                 \
    Mat<T,R,C>                                \
    matFunc (Mat<T,R,C> const & mat)          \
    {                                                       \
        Mat<T,R,C>    ret;                    \
        for (uint ii=0; ii<R*C; ++ii)               \
            ret[ii] = elemFunc (mat[ii]);                   \
        return ret;                                         \
    }

FG_MATRIXC_ELEMWISE(mapAbs,std::abs)
FG_MATRIXC_ELEMWISE(mapLog,std::log)
FG_MATRIXC_ELEMWISE(mapExp,std::exp)

template<uint R,uint C>
Mat<bool,R,C>
mapOr(Mat<bool,R,C> v0,Mat<bool,R,C> v1)
{
    Mat<bool,R,C> ret;
    for (uint ii=0; ii<R*C; ++ii)
        ret[ii] = v0[ii] || v1[ii];
    return ret;
}

// Faster equivalent to lhs^T * rhs:
template<typename T,uint n0,uint n1,uint n2>
Mat<T,n0,n1>
transposeMul(
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

template<typename T,uint R,uint C>
Mat<T,R,C> 
Mat<T,R,C>::randNormal(T stdev)
{
    Mat<T,R,C>        ret;
    for (size_t ii=0; ii<R*C; ++ii)
        ret[ii] = static_cast<T>(Fg::randNormal())*stdev;
    return ret;
}

template<typename T,uint R,uint C>
Mat<T,R,C>
Mat<T,R,C>::randUniform(T lo,T hi)
{
    Mat<T,R,C>    ret;
    for (size_t ii=0; ii<R*C; ++ii)
        ret[ii] = static_cast<T>(Fg::randUniform(double(lo),double(hi)));
    return ret;
}

template<typename T,uint dim,
    FG_ENABLE_IF(T,is_floating_point)>
Mat<T,dim,1>
randVecNormal()
{
    Mat<T,dim,1>    ret;
    for (uint ii=0; ii<dim; ++ii)
        ret[ii] = scast<T>(randNormal());
    return ret;
}

template<typename T,uint dim,
    FG_ENABLE_IF(T,is_floating_point)>
Svec<Mat<T,dim,1>>
randVecNormals(size_t sz,double stdev)
{
    Svec<Mat<T,dim,1>>     ret;
    ret.reserve(sz);
    for (size_t ii=0; ii<sz; ++ii)
        ret.push_back(randVecNormal<T,dim>()*stdev);
    return ret;
}

// Create a wider matrix by concatenating rows from 2 matrices:
template<class T,uint R,uint ncols1,uint C2>
Mat<T,R,ncols1+C2>
catHoriz(
    const Mat<T,R,ncols1> & lhs,
    const Mat<T,R,C2> & rhs)
{
    Mat<T,R,ncols1+C2>    ret;
    for (uint row=0; row<R; ++row)
    {
        uint    col=0;
        for (uint cc=0; cc<ncols1; ++cc)
            ret.rc(row,col++) = lhs.rc(row,cc);
        for (uint cc=0; cc<C2; ++cc)
            ret.rc(row,col++) = rhs.rc(row,cc);
    }
    return ret;
}

// Create a wider matrix by concatenating rows from 3 matrices:
template<class T,uint R,uint ncols1,uint C2,uint ncols3>
Mat<T,R,ncols1+C2+ncols3>
catHoriz(
    const Mat<T,R,ncols1> & m1,
    const Mat<T,R,C2> & m2,
    const Mat<T,R,ncols3> & m3)
{
    Mat<T,R,ncols1+C2+ncols3> ret;
    for (uint row=0; row<R; ++row)
    {
        uint    col=0;
        for (uint cc=0; cc<ncols1; ++cc)
            ret.rc(row,col++) = m1.rc(row,cc);
        for (uint cc=0; cc<C2; ++cc)
            ret.rc(row,col++) = m2.rc(row,cc);
        for (uint cc=0; cc<ncols3; ++cc)
            ret.rc(row,col++) = m3.rc(row,cc);
    }
    return ret;
}

// Create a taller matrix by concatenating cols from 2 matrices:
template<class T,uint nrows1,uint nrows2,uint C>
Mat<T,nrows1+nrows2,C>
catVertical(
    const Mat<T,nrows1,C> & upper,
    const Mat<T,nrows2,C> & lower)
{
    Mat<T,nrows1+nrows2,C>    ret;
    for (uint rr=0; rr<nrows1; ++rr)
        for (uint cc=0; cc<C; ++cc)
            ret.rc(rr,cc) = upper.rc(rr,cc);
    for (uint rr=0; rr<nrows2; ++rr)
        for (uint cc=0; cc<C; ++cc)
            ret.rc(nrows1+rr,cc) = lower.rc(rr,cc);
    return ret;
}

// Create a taller matrix by concatenating a given value to all columns:
template<class T,uint R,uint C>
Mat<T,R+1,C>
catVertical(Mat<T,R,C> const & mat,T val)
{
    Mat<T,R+1,C>  ret;
    uint    ii=0;
    for (; ii<R*C; ++ii)
        ret[ii] = mat[ii];
    ret[ii] = val;
    return ret;
}

// Parameterized unmirrored permutation of axes in 3D:
template<class T>
Mat<T,3,3>
permuteAxes(uint axisToBecomeX)
{
    Mat<T,3,3>    ret;
    for (uint ii=0; ii<3; ++ii)
        ret.rc(ii,(ii+axisToBecomeX)%3) = T(1);
    return ret;
}

template<class T,uint R,uint C>
bool
isPow2(Mat<T,R,C> const & mat)
{
    for (uint ii=0; ii<R*C; ++ii)
        if (!isPow2(mat[ii]))
            return false;
    return true;
}

template<class T,uint R,uint C>
T
cMean(Mat<T,R,C> const & mat)
{
    typedef typename Traits<T>::Accumulator Acc;
    typedef typename Traits<T>::Scalar      Scal;
    Acc     acc = Acc(mat[0]);
    for (uint ii=1; ii<R*C; ++ii)
        acc += Acc(mat[ii]);
    return T(acc / Scal(R*C));
}

template<class T,uint R,uint C>
double
cDot(
    Svec<Mat<T,R,C>> const & v0,
    Svec<Mat<T,R,C>> const & v1)
{
    FGASSERT(v0.size() == v1.size());
    double  acc(0);
    for (size_t ii=0; ii<v0.size(); ++ii)
        acc += cDot(v0[ii],v1[ii]);
    return acc;
}

// Weighted dot product:
template<class T,uint R,uint C>
double
fgDotWgt(
    Svec<Mat<T,R,C>> const & v0,
    Svec<Mat<T,R,C>> const & v1,
    Svec<T> const &                         w)    // Weight to apply to each dot product
{
    FGASSERT(v0.size() == v1.size());
    FGASSERT(v0.size() == w.size());
    double  acc(0);
    for (size_t ii=0; ii<v0.size(); ++ii)
        acc += cDot(v0[ii],v1[ii]) * w[ii];
    return acc;
}

template<typename T,uint dim>
T
cTrace(const Mat<T,dim,dim> & m)
{
    T                   ret(0);
    uint constexpr      inc = dim+1;    // Increment by a row's worth plus 1
    for (uint ii=0; ii<m.numElems(); ii+=inc)
        ret += m[ii];
    return ret;
}

// For Mat/Vec use linear interpolation
template<uint R,uint C>
Mat<double,R,C>
interpolate(Mat<double,R,C> m0,Mat<double,R,C> m1,double val)   // val [0,1]
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
    weights = catHoriz(coordH-coord,coord-coordL);
    coordLo = Mat<Int,dim,1>(coordL);
}

// Provide an ordering by axis order
// (without making it the default operator as this may not be desired):
template<typename T,uint R,uint C>
bool
fgLt(
    Mat<T,R,C> const & m0,
    Mat<T,R,C> const & m1)
{
    for (uint ii=0; ii<R*C; ++ii)
        if (!(m0[ii] < m1[ii]))
            return false;
    return true;
}

template<class T,uint R,uint C>
T
fgSumElems(Mat<T,R,C> const & m)
{
    T   ret(m[0]);
    for (uint ii=1; ii<R*C; ++ii)
        ret += m[ii];
    return ret;
}

template<class T,uint sz>
Mat<T,sz,sz>
cDiagMat(Mat<T,sz,1> vec)
{
    Mat<T,sz,sz>      ret(static_cast<T>(0));
    for (uint ii=0; ii<sz; ++ii)
        ret.rc(ii,ii) = vec[ii];
    return ret;
}

template<class T>
Mat<T,2,2>
cDiagMat(T v0,T v1)
{
    Mat<T,2,2>    ret(static_cast<T>(0));
    ret[0] = v0;
    ret[3] = v1;
    return ret;
}

template<class T>
Mat<T,3,3>
cDiagMat(T v0,T v1,T v2)
{
    Mat<T,3,3>    ret(static_cast<T>(0));
    ret[0] = v0;
    ret[4] = v1;
    ret[8] = v2;
    return ret;
}

template<typename T,uint R,uint C>
void
normalize_(Mat<T,R,C> & m)
{
    T       len = m.len();
    FGASSERT(len > 0);
    m.m /= len;
}

template<typename T,uint R,uint C>
Mat<T,R,C>
normalize(Mat<T,R,C> m)
{
    T       len = m.len();
    FGASSERT(len > 0);
    return m / len;
}

// Find first index of an element in a Svec. Return 'size' if not found:
template<typename T,uint R>
uint
findFirstIdx(Mat<T,R,1> m,T v)
{
    for (uint ii=0; ii<R; ++ii)
        if (m[ii] == v)
            return ii;
    return R;
}

// Solve linear system of equations of the form Mx = b. Assumes matrix is well-conditioned:
Vec2D           solveLinear(Mat22D const & M,Vec2D const & b);
Vec3D           solveLinear(Mat33D const & M,Vec3D const & b);
Vec4D           solveLinear(Mat44D const & M,Vec4D const & b);

template<typename T,uint R,uint C>
bool
noZeroElems(Mat<T,R,C> m)
{
    T                   acc = T(1);
    for (T v : m.m)
        acc *= v;
    return (acc != T(0));
}

template<uint R,uint C>
Mat<double,R,C>
fgF2D(const Mat<float,R,C> & m)
{return Mat<double,R,C>(m); }

template<uint R,uint C>
Mat<float,R,C>
fgD2F(const Mat<double,R,C> & m)
{return Mat<float,R,C>(m); }

template<class T,uint R,uint C>
double
cMag(Mat<T,R,C> m)
{return m.mag(); }

template<class T,uint R,uint C>
double
cRms(Mat<T,R,C> m)
{return std::sqrt(m.mag()/static_cast<double>(R*C)); }

template<typename T,uint R,uint C>
inline T
cLen(Svec<Mat<T,R,C>> const & v)
{return std::sqrt(cMag(v)); }

template<uint R,uint C>
Mat<double,R,C>
cReal(const Mat<std::complex<double>,R,C> & m)   // Return real compoments
{
    Mat<double,R,C>   ret;
    for (uint ii=0; ii<R*C; ++ii)
        ret[ii] = m[ii].real();
    return ret;
}

template<class T,class U,uint R,uint C>
void
deepCast_(Mat<T,R,C> const & i,Mat<U,R,C> & o)
{
    for (size_t ii=0; ii<i.numElems(); ++ii)
        deepCast_(i[ii],o[ii]);
}

// Transpose a matrix stored as an array of arrays. All sub-arrays must have same size:
template<class T,uint R,uint C>
Mat<Svec<T>,R,C>
transpose(Svec<Mat<T,R,C>> const & v)
{
    Mat<Svec<T>,R,C>   ret;
    for (uint ii=0; ii<R*C; ++ii)
        ret[ii].resize(v.size());
    for (size_t jj=0; jj<v.size(); ++jj) {
        for (uint ii=0; ii<R*C; ++ii)
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
fgPermute(Mat<T,dim,1> const & v,Mat<uint,dim,1> perm)  // Assumes a valid permutation, use above to check
{
    Mat<T,dim,1>      ret;
    for (uint dd=0; dd<dim; ++dd)
        ret[dd] = v[perm[dd]];
    return ret;
}

// Change a square matrix representation under a permutation of 2 of the axes:
template<class T,uint dim>
Mat<T,dim,dim>
permuteAxes(
    Mat<T,dim,dim> const &      M,
    Mat<uint,dim,1>             p)  // Permuation map from input index to output index. NOT checked for validity.
{
    Mat<T,dim,dim>      ret;
    for (uint rr=0; rr<dim; ++rr)
        for (uint cc=0; cc<dim; ++cc)
            ret.rc(p[rr],p[cc]) = M.rc(rr,cc);
    return ret;
}

template<class T,uint R,uint C>
Mat<T,C,R>
cHermitian(Mat<T,R,C> const & mat)
{
    Mat<T,C,R>        ret;
    for (uint rr=0; rr<R; ++rr)
        for (uint cc=0; cc<C; ++cc)
            ret.rc(cc,rr) = std::conj(mat.rc(rr,cc));
    return ret;
}

// Symmetric 3x3 double matrix:
struct  MatS3D
{
    Arr<double,3>       diag;
    Arr<double,3>       offd;       // In order 01, 02, 12
    FG_SER2(diag,offd);

    MatS3D() {}
    explicit MatS3D(double fillVal) {diag.fill(fillVal); offd.fill(fillVal); }
    explicit MatS3D(Mat33D const &);   // Checks for symmetry
    MatS3D(Arr<double,3> const & d,Arr<double,3> const & o) : diag(d), offd(o)  {}

    static MatS3D identity() {return { {{1,1,1}} , {{0,0,0}} }; }

    double
    rc(size_t row,size_t col) const
    {
        FGASSERT((row<3)&&(col<3));
        if (row==col)
            return diag[row];
        else
            return offd[row+col-1];
    }
    Mat33D          asMatC() const
    {
        return Mat33D {diag[0],offd[0],offd[1], offd[0],diag[1],offd[2], offd[1],offd[2],diag[2]};
    }
    void            operator+=(MatS3D const & rhs) {diag += rhs.diag; offd += rhs.offd; }
    MatS3D          operator+(MatS3D const & rhs) const {return {diag+rhs.diag,offd+rhs.offd}; }
    MatS3D          operator-(MatS3D const & rhs) const {return {diag-rhs.diag,offd-rhs.offd}; }
    MatS3D          operator*(double rhs) const {return {diag*rhs,offd*rhs}; }
    Vec3D
    operator*(Vec3D rhs) const
    {
        return {
            diag[0]*rhs[0] + offd[0]*rhs[1] + offd[1]*rhs[2],
            offd[0]*rhs[0] + diag[1]*rhs[1] + offd[2]*rhs[2],
            offd[1]*rhs[0] + offd[2]*rhs[1] + diag[2]*rhs[2],
        };
    }
    Vec3D           diagonal() const {return Vec3D{diag}; }
    double          sumElems() const {return cSum(diag) + 2.0 * cSum(offd); }
    // Sum of squares of elements (== square of frobenius norm):
    double          mag() const {return cMag(diag) + 2.0 * cMag(offd); }
    double          dot(MatS3D const & rhs) const {return cDot(diag,rhs.diag) + 2.0 * cDot(offd,rhs.offd); }
    double
    determinant() const
    {
        return
            diag[0]*diag[1]*diag[2] +
            offd[0]*offd[1]*offd[2] * 2 -
            diag[0]*offd[2]*offd[2] -
            diag[1]*offd[1]*offd[1] -
            diag[2]*offd[0]*offd[0];
    }
    MatS3D          inverse() const;
};

std::ostream &      operator<<(std::ostream & os,MatS3D const & m);
inline double       cMag(MatS3D const & m) {return m.mag(); }
inline double       cSum(MatS3D const & m) {return m.sumElems(); }
inline double       cDot(MatS3D const & l,MatS3D const & r) {return l.dot(r); }
inline double       cDeterminant(MatS3D const & m) {return m.determinant(); }
inline MatS3D       cInverse(MatS3D const & m) {return m.inverse(); }
inline double       cQuadForm(MatS3D const & Q,Vec3D const & x_m) {return cDot(x_m,Q*x_m); }

inline
MatS3D
mapSqr(MatS3D const & m)
{
    return MatS3D {mapSqr(m.diag),mapSqr(m.offd)};
}

inline
MatS3D
outerProductSelf(Vec3D v)
{
    return          MatS3D {
        {{sqr(v[0]),sqr(v[1]),sqr(v[2])}},
        {{v[0]*v[1],v[0]*v[2],v[1]*v[2]}},
    };
}

// Isotropic random symmetric positive definite matrix with given standard deviation of log eigenvalues:
MatS3D          randMatSpd3D(double lnEigStdev);

// Symmetric 2x2 double matrix:
struct  MatS2D
{
    double          m00,m11,m01;
    FG_SER3(m00,m11,m01);

    Vec2D
    operator*(Vec2D const & rhs) const
    {return {m00*rhs[0] + m01*rhs[1], m01*rhs[0] + m11*rhs[1]}; }

    double
    determinant() const
    {return m00 * m11 - m01 * m01; }
};

// Upper triangular 3x3 matrix non-zero entries in row-major order
struct  MatUT3D
{
    Arr<double,6>     m;
    FG_SER1(m)

    MatUT3D() {}
    explicit MatUT3D(Arr<double,6> const & data) : m(data) {}
    MatUT3D(double m0,double m1,double m2,double m3,double m4,double m5)
    {m[0]=m0; m[1]=m1; m[2]=m2; m[3]=m3; m[4]=m4; m[5]=m5; }
    explicit MatUT3D(Arr3D const & scales)          // Diagonal version
    {
        m[0]=scales[0]; m[1]=0.0;       m[2]=0.0;
                        m[3]=scales[1]; m[4]=0.0;
                                        m[5]=scales[2];
    }
    MatUT3D(Arr3D scales,Arr3D shears)              // Scales are the diag elems, shears are off-diag UT
    {
        m[0]=scales[0]; m[1]=shears[0]; m[2]=shears[1];
                        m[3]=scales[1]; m[4]=shears[2];
                                        m[5]=scales[2];
    }

    static MatUT3D  identity()  {return MatUT3D(1,0,0,1,0,1); }
    static MatUT3D  randNormal(double lnEigsStdev,double shearsStdev);          // Isotropic positive definite

    MatUT3D         operator-(MatUT3D const & rhs) const {return MatUT3D(m-rhs.m); }
    MatUT3D         operator*(double s) const {return MatUT3D {m*s}; }
    MatUT3D         operator/(double s) const {return  MatUT3D {m/s}; }
    Vec3D           operator*(Vec3D v) const
    {
        return Vec3D {m[0]*v[0]+m[1]*v[1]+m[2]*v[2], m[3]*v[1]+m[4]*v[2], m[5]*v[2]};
    }
    // U^T * v
    Vec3D
    tranposeMul(Vec3D r)
    {
        return Vec3D {
            m[0]*r[0],
            m[1]*r[0] + m[3]*r[1],
            m[2]*r[0] + m[4]*r[1] + m[5]*r[2] };
    }
    MatUT3D         operator*(MatUT3D r) const;
    double          determinant() const {return m[0]*m[3]*m[5]; }
    Mat33D          asMatrix() const {return Mat33D {m[0],m[1],m[2],0,m[3],m[4],0,0,m[5]}; }
    // Sum of squares of elements (== square of frobenius norm):
    double          mag() const {return cMag(m); }
    MatS3D          luProduct() const;
    MatUT3D         inverse() const;
};
std::ostream &  operator<<(std::ostream &,MatUT3D const &);
inline double   cDeterminant(MatUT3D const & m) {return m.determinant(); }
inline double   cMag(MatUT3D const & m) {return m.mag(); }

}

#endif
