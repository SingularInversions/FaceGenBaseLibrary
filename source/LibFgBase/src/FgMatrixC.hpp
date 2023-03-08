//
// Copyright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Functions operating on Mat<>

#ifndef FGMATRIXC_HPP
#define FGMATRIXC_HPP

#include "FgMatrixCBase.hpp"
#include "FgMath.hpp"
#include "FgRandom.hpp"

namespace Fg {

template<class T,uint R,uint C>
T                   cSsd(Mat<T,R,C> const & l,Mat<T,R,C> const & r) {return cSsd(l.m,r.m); }
template<class T,uint R,uint C>
T                   cLen(Mat<T,R,C> const & mat) {return cLen(mat.m); }
template<class T,uint R,uint C>
Mat<T,R,C>          operator*(T val,Mat<T,R,C> const & mat) {return (mat * val); }
template <class T,uint R,uint C,uint C2>
Mat<T,R,C2>         operator*(Mat<T,R,C> const & v1,Mat<T,C,C2> const & v2)
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
Mat<T,2,2>          matRotate(T radians)
{
    T               c = std::cos(radians),
                    s = std::sin(radians);
    return Mat<T,2,2>{
        c, -s,
        s,  c,
    };
}
template <class T>
Mat<T,3,3>          matRotateX(T radians)        // RHR rotation around X axis
{
    T               c = std::cos(radians),
                    s = std::sin(radians);
    return Mat<T,3,3>{
        1,  0,  0,
        0,  c, -s,
        0,  s,  c,
    };
}
template <class T>
Mat<T,3,3>          matRotateY(T radians)        // RHR rotation around Y axis
{
    T               c = std::cos(radians),
                    s = std::sin(radians);
    return Mat<T,3,3>{
        c,  0,  s,
        0,  1,  0,
        -s, 0,  c,
    };
}
template <class T>
Mat<T,3,3>          matRotateZ(T radians)        // RHR rotation around Z axis
{
    T               c = std::cos(radians),
                    s = std::sin(radians);
    return Mat<T,3,3>{
        c, -s,  0,
        s,  c,  0,
        0,  0,  1,
    };
}
// Create matrix for RHR around an arbitrary axis:
Mat33D              matRotateAxis(double radians,Vec3D const & normalizedZxis);

template<class T>
Mat<T,3,1>          cMirrorX(Mat<T,3,1> const & m) {return {-m[0],m[1],m[2]}; }

template<class T>
T                   cDeterminant(const Mat<T,2,2> & mat) {return (mat[0]*mat[3] - mat[1]*mat[2]); }
// Useful for finding if aspect ratios match, or sine of angle between normalized vectors:
template<class T>
T                   cDeterminant(const Mat<T,2,1> & col0,const Mat<T,2,1> & col1)
{
    return col0[0]*col1[1] - col0[1]*col1[0];
}
template<class T>
T                   cDeterminant(Mat<T,3,3> const & M)
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
T                   cDeterminant(Mat<T,4,4> const & M)
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
// append an element onto a column vec:
template<class T,uint dim>
Mat<T,dim+1,1>      append(Mat<T,dim,1> const & vec,T val)
{
    Mat<T,dim+1,1>    ret;
    for (uint ii=0; ii<dim; ++ii)
        ret[ii] = vec[ii];
    ret[dim] = val;
    return ret;
}
// append an element onto a row vec:
template<class T,uint dim>
Mat<T,1,dim+1>      append(const Mat<T,1,dim> & vec,T val)
{
    Mat<T,1,dim+1>    ret;
    for (uint ii=0; ii<dim; ++ii)
        ret[ii] = vec[ii];
    ret[dim] = val;
    return ret;
}
// Concatenate 2 column vectors:
template<class T,uint D0,uint D1>
Mat<T,D0+D1,1>      cat(Mat<T,D0,1> const & l,Mat<T,D1,1> const & r)
{
    return Mat<T,D0+D1,1>{cat(l.m,r.m)};
}
// create block diagonal square matrix of 2 square matrices. Off diagonals set to 0.
template<class T,uint D0,uint D1>
Mat<T,D0+D1,D0+D1>  catDiagonal(Mat<T,D0,D0> const & l,Mat<T,D1,D1> const & r)
{
    Mat<T,D0+D1,D0+D1>      ret(0);
    for (uint rr=0; rr<D0; ++rr)
        for (uint cc=0; cc<D0; ++cc)
            ret.rc(rr,cc) = l.rc(rr,cc);
    for (uint rr=0; rr<D1; ++rr)
        for (uint cc=0; cc<D1; ++cc)
            ret.rc(D0+rr,D0+cc) = r.rc(rr,cc);
    return ret;
}
// Flatten a Svec of matrices into a Svec of scalars:
template<class T,uint R,uint C>
Svec<T>             flatten(Svec<Mat<T,R,C>> const & ms)
{
    Svec<T>       ret;
    ret.reserve(ms.size()*R*C);
    for (size_t ii=0; ii<ms.size(); ++ii)
        for (uint jj=0; jj<R*C; ++jj)
            ret.push_back(ms[ii].m[jj]);
    return ret;
}

Doubles             toDoubles(Floatss const & v);
template<class T,uint R,uint C>
Doubles             toDoubles(Mat<T,R,C> const & mat)
{
    Doubles         ret;
    ret.reserve(mat.numElems());
    for (T e : mat.m)
        ret.push_back(scast<double>(e));
    return ret;
}
template<class T,uint R,uint C>
Doubles             toDoubles(Svec<Mat<T,R,C>> const & ms)
{
    Doubles         ret;
    ret.reserve(ms.size()*R*C);
    for (Mat<T,R,C> const & mat : ms)
        for (T e : mat.m)
            ret.push_back(scast<double>(e));
    return ret;
}
template<class T,uint R,uint C>
Doubles             toDoubles(const Svec<Svec<Mat<T,R,C>>> & mss)
{
    size_t          sz = 0;
    for (Svec<Mat<T,R,C>> const & ms : mss)
        sz += ms.size() * R * C;
    Doubles         ret;
    ret.reserve(sz);
    for (Svec<Mat<T,R,C>> const & ms : mss)
        for (Mat<T,R,C> const & mat : ms)
            for (T e : mat.m)
                ret.push_back(scast<double>(e));
    return ret;
}
// project a homogenous position vector back to euclidean representation:
template<class T,uint dim>
Mat<T,dim-1,1>      projectHomog(Mat<T,dim,1> v)
{
    Mat<T,dim-1,1>    ret;
    T                 w = v[dim-1];
    FGASSERT(w != T(0));                // cannot be a direction vector
    for (uint ii=0; ii<dim-1; ++ii)
        ret[ii] = v[ii] / w;
    return ret;
}
// Return homogeneous matrix representation of an affine transform:
template<class T, uint dims>
Mat<T,dims+1,dims+1> asHomogMat(Mat<T,dims,dims> const & linear,Mat<T,dims,1> const & translation)
{
    Mat<T,dims+1,dims+1>    ret;
    for (uint rr=0; rr<dims; rr++)
        for (uint cc=0; cc<dims; cc++)
            ret.rc(rr,cc) = linear.rc(rr,cc);
    for (uint rr=0; rr<dims; rr++)
        ret.rc(rr,dims) = translation[rr];
    ret.rc(dims,dims) = 1;
    return ret;
}
// Return homogeneous matrix representation of a linear transform:
template<class T, uint dims>
Mat<T,dims+1,dims+1> asHomogMat(Mat<T,dims,dims> const & linear)
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
Mat<T,dims+1,dims+1> asHomogMat(Mat<T,dims,1> const & translation)
{
    auto            ret = Mat<T,dims+1,dims+1>::identity();
    for (uint rr=0; rr<dims; rr++)
        ret.rc(rr,dims) = translation[rr];
    return ret;
}
// returns the inverse of an invertible matrix, throws if not invertible:
template <class T>
Mat<T,2,2>          cInverse(Mat<T,2,2> const & mat)
{
    static_assert(std::is_floating_point<T>::value,"Mat inverse requires floating point type");
    Mat<T,2,2>     ret;
    T   fac = (mat.rc(0,0) * mat.rc(1,1) - mat.rc(0,1) * mat.rc(1,0));
    FGASSERT(fac != T(0));
    fac = T(1) / fac;
    ret.rc(0,0) = mat.rc(1,1) * fac;
    ret.rc(0,1) = - mat.rc(0,1) * fac;
    ret.rc(1,0) = - mat.rc(1,0) * fac;
    ret.rc(1,1) = mat.rc(0,0) * fac;
    return ret;
}
template <class T>
Mat<T,3,3>          cInverse(Mat<T,3,3> const & i)
{
    static_assert(std::is_floating_point<T>::value,"Mat inverse requires floating point type");
    Mat<T,3,3>     r;
    T   fac = (i.rc(0,0)*i.rc(1,1)*i.rc(2,2) - i.rc(0,0)*i.rc(1,2)*i.rc(2,1) +
               i.rc(1,0)*i.rc(0,2)*i.rc(2,1) - i.rc(1,0)*i.rc(0,1)*i.rc(2,2) +
               i.rc(2,0)*i.rc(0,1)*i.rc(1,2) - i.rc(2,0)*i.rc(1,1)*i.rc(0,2));
    FGASSERT(fac != T(0));
    r.rc(0,0) = i.rc(1,1) * i.rc(2,2) - i.rc(1,2) * i.rc(2,1);
    r.rc(0,1) = i.rc(0,2) * i.rc(2,1) - i.rc(0,1) * i.rc(2,2);
    r.rc(0,2) = i.rc(0,1) * i.rc(1,2) - i.rc(0,2) * i.rc(1,1);
    r.rc(1,0) = i.rc(1,2) * i.rc(2,0) - i.rc(1,0) * i.rc(2,2);
    r.rc(1,1) = i.rc(0,0) * i.rc(2,2) - i.rc(0,2) * i.rc(2,0);
    r.rc(1,2) = i.rc(0,2) * i.rc(1,0) - i.rc(0,0) * i.rc(1,2);
    r.rc(2,0) = i.rc(1,0) * i.rc(2,1) - i.rc(1,1) * i.rc(2,0);
    r.rc(2,1) = i.rc(0,1) * i.rc(2,0) - i.rc(0,0) * i.rc(2,1);
    r.rc(2,2) = i.rc(0,0) * i.rc(1,1) - i.rc(1,0) * i.rc(0,1);
    r *= T(1) / fac;
    return r;
}
template <class T,uint R,uint C>
T                   cDot(Mat<T,R,C> const & lhs,Mat<T,R,C> const & rhs) {return cDot(lhs.m,rhs.m); }
template <class T,uint R,uint C>
double              cCos(Mat<T,R,C> const & lhs,Mat<T,R,C> const & rhs)
{
    double      mag = cMag(lhs) * cMag(rhs);
    FGASSERT(mag > 0.0);
    return cDot(lhs,rhs)/sqrt(mag);
}
template<typename T>
Mat<T,3,1>          crossProduct(Mat<T,3,1> const & v1,Mat<T,3,1> const & v2)
{
    Mat<T,3,1>      r;
    r[0] = v1[1] * v2[2] - v1[2] * v2[1];
    r[1] = v1[2] * v2[0] - v1[0] * v2[2];
    r[2] = v1[0] * v2[1] - v1[1] * v2[0];
    return r;
}
// Equivalent to V * Y.transpose() (but more efficient and succinct):
template<typename T,uint R,uint C>
Mat<T,R,C>          outerProduct(Mat<T,R,1> const & lhs,Mat<T,C,1> const & rhs)
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
Mat<T,R,C>          mapCall(Mat<T,R,C> const & lhs,Mat<T,R,C> const & rhs,F func)
{
    Mat<T,R,C>          ret;
    for (size_t ii=0; ii<R*C; ++ii)
        ret[ii] = func(lhs,rhs);
    return ret;
}
// Subtract same value from each element:
template<typename T,uint R,uint C>
Mat<T,R,C>          mapSub(Mat<T,R,C> const & lhs,T const & rhs) {return Mat<T,R,C>{mapSub(lhs.m,rhs)}; }

// Element-wise multiplication (aka Hadamard product):
template<typename T,uint R,uint C>
Mat<T,R,C>          mapMul(Mat<T,R,C> const & lhs,Mat<T,R,C> const & rhs)
{
    Mat<T,R,C>    ret;
    for (uint ii=0; ii<R*C; ++ii)
        ret[ii] = lhs[ii] * rhs[ii];
    return ret;
}
template<typename T,uint R,uint C>
Mat<T,R,C>          mapMul(Mat<T,R,C> const & m0,Mat<T,R,C> const & m1,Mat<T,R,C> const & m2)
{
    Mat<T,R,C>    ret;
    for (uint ii=0; ii<R*C; ++ii)
        ret[ii] = m0[ii] * m1[ii] * m2[ii];
    return ret;
}
// Element-wise division:
template<typename T,uint R,uint C>
Mat<T,R,C>          mapDiv(Mat<T,R,C> const & lhs,Mat<T,R,C> const & rhs)
{
    Mat<T,R,C>            ret;
    for (uint ii=0; ii<R*C; ++ii)
        ret[ii] = lhs[ii] / rhs[ii];
    return ret;
}
template<typename T,uint R,uint C,class F>
Mat<T,R,C>          mapCall(Mat<T,R,C> const & mat,F const & func)
{
    Mat<T,R,C>          ret;
    for (uint ii=0; ii<R*C; ++ii)
        ret[ii] = func(mat[ii]);
    return ret;
}
template<class T,uint R,uint C>
Svec<T>             mapMag(Svec<Mat<T,R,C>> const & v)
{
    Svec<T>   ret(v.size());
    for (size_t ii=0; ii<v.size(); ++ii)
        ret[ii] = cMag(v[ii]);
    return ret;
}
template<class T,uint R,uint C>
Mat<T,R,C>          mapSqr(Mat<T,R,C> mat)
{
    Mat<T,R,C>    r;
    for (uint ii=0; ii<R*C; ++ii)
        r[ii] = sqr(mat[ii]);
    return r;
}
template<class T,uint R,uint C>
Mat<T,R,C>          mapSqrt(Mat<T,R,C> mat)
{
    Mat<T,R,C>    r;
    for (uint ii=0; ii<R*C; ++ii)
        r[ii] = sqrt(mat[ii]);
    return r;
}
template<typename T,uint R,uint C>
Mat<T,R,C>          mapFloor(Mat<T,R,C> const & mat)
{
    Mat<T,R,C> ret;
    for (uint ii=0; ii<mat.numElems(); ++ii)
        ret[ii] = std::floor(mat[ii]);
    return ret;
}
template<typename Flt,typename Int,uint R,uint C>
void                round_(Mat<Flt,R,C> const & lhs,Mat<Int,R,C> & rhs)
{
    for (uint ii=0; ii<rhs.numElems(); ++ii)
        round_(lhs[ii],rhs[ii]);
}
template<typename To,typename From,uint R,uint C>
Mat<To,R,C>         mapRound(Mat<From,R,C> const & mat)
{
    Mat<To,R,C>     ret;
    for (uint ii=0; ii<mat.numElems(); ++ii)
        ret[ii] = round<To>(mat[ii]);
    return ret;
}
template<uint R,uint C>
Mat<uint,R,C>       mapPow2Ceil(Mat<uint,R,C> const & mat)
{
    Mat<uint,R,C>       ret;
    for (uint ii=0; ii<mat.numElems(); ++ii)
        ret[ii] = pow2Ceil(mat[ii]);
    return ret;
}

#define FG_MATRIXC_MAP_BINARY(mapName)                                      \
template<class T,uint R,uint C>                                             \
Mat<T,R,C>          mapName(Mat<T,R,C> const & l,Mat<T,R,C> const & r)      \
{return Mat<T,R,C>{mapName(l.m,r.m)}; }

FG_MATRIXC_MAP_BINARY(mapMin)
FG_MATRIXC_MAP_BINARY(mapMax)


#define FG_MATRIXC_ELEMWISE(matFunc,elemFunc)               \
    template<class T,uint R,uint C>                 \
    Mat<T,R,C>      matFunc (Mat<T,R,C> const & mat)          \
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
Mat<bool,R,C>       mapOr(Mat<bool,R,C> v0,Mat<bool,R,C> v1)
{
    Mat<bool,R,C> ret;
    for (uint ii=0; ii<R*C; ++ii)
        ret[ii] = v0[ii] || v1[ii];
    return ret;
}

// Faster equivalent to lhs^T * rhs:
template<typename T,uint n0,uint n1,uint n2>
Mat<T,n0,n1>        transposeMul(Mat<T,n2,n0> const & lhs,Mat<T,n2,n1> const & rhs)
{
    Mat<T,n0,n1>      ret(T(0));
    for (uint i0=0; i0<n0; ++i0)
        for (uint i1=0; i1<n1; ++i1)
            for (uint i2=0; i2<n2; ++i2)
                ret.rc(i0,i1) += lhs.rc(i2,i0) * rhs.rc(i2,i1);
    return ret;
}
template<typename T,uint R,uint C>
Mat<T,R,C>          Mat<T,R,C>::randNormal(T stdev)
{
    Mat<T,R,C>        ret;
    for (size_t ii=0; ii<R*C; ++ii)
        ret[ii] = static_cast<T>(Fg::randNormal())*stdev;
    return ret;
}
template<typename T,uint R,uint C>
Mat<T,R,C>          Mat<T,R,C>::randUniform(T lo,T hi)
{
    Mat<T,R,C>    ret;
    for (size_t ii=0; ii<R*C; ++ii)
        ret[ii] = static_cast<T>(Fg::randUniform(double(lo),double(hi)));
    return ret;
}
template<typename T,uint dim,FG_ENABLE_IF(T,is_floating_point)>
Mat<T,dim,1>        randVecNormal()
{
    Mat<T,dim,1>    ret;
    for (uint ii=0; ii<dim; ++ii)
        ret[ii] = scast<T>(randNormal());
    return ret;
}
template<typename T,uint dim,FG_ENABLE_IF(T,is_floating_point)>
Svec<Mat<T,dim,1>>  randVecNormals(size_t sz,double stdev)
{
    Svec<Mat<T,dim,1>>     ret;
    ret.reserve(sz);
    for (size_t ii=0; ii<sz; ++ii)
        ret.push_back(randVecNormal<T,dim>()*stdev);
    return ret;
}
// Create a wider matrix by concatenating rows from 2 matrices:
template<class T,uint R,uint ncols1,uint C2>
Mat<T,R,ncols1+C2>  catHoriz(Mat<T,R,ncols1> const & lhs,Mat<T,R,C2> const & rhs)
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
// Create a taller matrix by concatenating cols from 2 matrices:
template<class T,uint nrows1,uint nrows2,uint C>
Mat<T,nrows1+nrows2,C>  catVertical(
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
Mat<T,R+1,C>        catVertical(Mat<T,R,C> const & mat,T val)
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
Mat<T,3,3>          permuteAxes(uint axisToBecomeX)
{
    Mat<T,3,3>    ret;
    for (uint ii=0; ii<3; ++ii)
        ret.rc(ii,(ii+axisToBecomeX)%3) = T(1);
    return ret;
}

template<class T,uint R,uint C>
bool                isPow2(Mat<T,R,C> const & mat)
{
    for (uint ii=0; ii<R*C; ++ii)
        if (!isPow2(mat[ii]))
            return false;
    return true;
}
template<class T,uint R,uint C>
T                   cMean(Mat<T,R,C> const & mat)
{
    typedef typename Traits<T>::Accumulator Acc;
    typedef typename Traits<T>::Scalar      Scal;
    Acc     acc = Acc(mat[0]);
    for (uint ii=1; ii<R*C; ++ii)
        acc += Acc(mat[ii]);
    return T(acc / Scal(R*C));
}
template<class T,uint R,uint C>
double              cDot(Svec<Mat<T,R,C>> const & v0,Svec<Mat<T,R,C>> const & v1)
{
    FGASSERT(v0.size() == v1.size());
    double  acc(0);
    for (size_t ii=0; ii<v0.size(); ++ii)
        acc += cDot(v0[ii],v1[ii]);
    return acc;
}
// Weighted dot product:
template<class T,uint R,uint C>
double              fgDotWgt(
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
T                   cTrace(const Mat<T,dim,dim> & mat)
{
    T                   ret(0);
    uint constexpr      inc = dim+1;    // Increment by a row's worth plus 1
    for (uint ii=0; ii<mat.numElems(); ii+=inc)
        ret += mat[ii];
    return ret;
}
// For Mat/Vec use linear interpolation
template<uint R,uint C>
Mat<double,R,C>     interpolate(Mat<double,R,C> m0,Mat<double,R,C> m1,double val)   // val [0,1]
{
    return m0*(1.0-val) + m1*(val);
}

template<class T,uint R,uint C>
inline T            cSumElems(Mat<T,R,C> const & mat) {return cSum(mat.m); }

template<class T,uint sz>
Mat<T,sz,sz>        cDiagMat(Mat<T,sz,1> vec)
{
    Mat<T,sz,sz>      ret(static_cast<T>(0));
    for (uint ii=0; ii<sz; ++ii)
        ret.rc(ii,ii) = vec[ii];
    return ret;
}
template<class T>
Mat<T,2,2>          cDiagMat(T v0,T v1)
{
    Mat<T,2,2>    ret(static_cast<T>(0));
    ret[0] = v0;
    ret[3] = v1;
    return ret;
}
template<class T>
Mat<T,3,3>          cDiagMat(T v0,T v1,T v2)
{
    Mat<T,3,3>    ret(static_cast<T>(0));
    ret[0] = v0;
    ret[4] = v1;
    ret[8] = v2;
    return ret;
}
template<typename T,uint R,uint C>
void                normalize_(Mat<T,R,C> & mat)
{
    T       len = mat.len();
    FGASSERT(len > 0);
    mat.m /= len;
}
template<typename T,uint R,uint C>
Mat<T,R,C>          normalize(Mat<T,R,C> mat)
{
    T       len = mat.len();
    FGASSERT(len > 0);
    return mat / len;
}
// Find first index of an element in a Svec. Return 'size' if not found:
template<typename T,uint R>
uint                findFirstIdx(Mat<T,R,1> mat,T v)
{
    for (uint ii=0; ii<R; ++ii)
        if (mat[ii] == v)
            return ii;
    return R;
}

// Solve linear system of equations of the form Mx = b. Assumes matrix is well-conditioned:
Vec2D               solveLinear(Mat22D const & M,Vec2D const & b);
Vec3D               solveLinear(Mat33D const & M,Vec3D const & b);
Vec4D               solveLinear(Mat44D const & M,Vec4D const & b);

template<typename T,uint R,uint C>
bool                noZeroElems(Mat<T,R,C> mat)
{
    T                   acc = T(1);
    for (T v : mat.m)
        acc *= v;
    return (acc != T(0));
}

template<class T,uint R,uint C>
double              cMag(Mat<T,R,C> mat) {return cMag(mat.m); }

template<class T,uint R,uint C>
double              cRms(Mat<T,R,C> mat) {return cRms(mat.m); }

template<typename T,uint R,uint C>
inline T            cLen(Svec<Mat<T,R,C>> const & v) {return std::sqrt(cMag(v)); }

template<uint R,uint C>
Mat<double,R,C>     cReal(const Mat<std::complex<double>,R,C> & mat)   // Return real compoments
{
    Mat<double,R,C>   ret;
    for (uint ii=0; ii<R*C; ++ii)
        ret[ii] = mat[ii].real();
    return ret;
}

template<class T,class U,uint R,uint C>
void                scast_(Mat<T,R,C> const & i,Mat<U,R,C> & o)
{
    for (size_t ii=0; ii<i.numElems(); ++ii)
        scast_(i[ii],o[ii]);
}

// Transpose a matrix stored as an array of arrays. All sub-arrays must have same size:
template<class T,uint R,uint C>
Mat<Svec<T>,R,C>    transpose(Svec<Mat<T,R,C>> const & v)
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

// project a vector onto an unnormalized arbitrary axis
// (ie. find the vector component parallel to that axis):
template<class T,uint dim>
Mat<T,dim,1>        projectVec(Mat<T,dim,1> vec,Mat<T,dim,1> axis)
{
    return axis * (cDot(vec,axis) / cMag(axis));
}

// remap vector indices from the given lookup list:
template<uint N>
Mat<uint,N,1>       remapInds(Mat<uint,N,1> inds,Uints const & inToOut)
{
    Mat<uint,N,1>           r;
    for (uint ii=0; ii<N; ++ii)
        r[ii] = inToOut[inds[ii]];
    return r;
}
template<uint N>
Svec<Mat<uint,N,1>> remapInds(Svec<Mat<uint,N,1>> const & inds,Uints const & inToOut)
{
    Svec<Mat<uint,N,1>>     ret; ret.reserve(inds.size());
    for (Mat<uint,N,1> const & idx : inds)
        ret.push_back(remapInds(idx,inToOut));
    return ret;
}

// Change a square matrix representation under a permutation of 2 of the axes:
template<class T,uint dim>
Mat<T,dim,dim>      permuteAxes(
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
Mat<T,C,R>          cHermitian(Mat<T,R,C> const & mat)
{
    Mat<T,C,R>        ret;
    for (uint rr=0; rr<R; ++rr)
        for (uint cc=0; cc<C; ++cc)
            ret.rc(cc,rr) = std::conj(mat.rc(rr,cc));
    return ret;
}

struct      MatS2D              // Symmetric 2x2 double matrix
{
    double          m00,m11,m01;
    FG_SER3(m00,m11,m01)

    inline Vec2D    operator*(Vec2D const & v) const {return {m00*v[0]+m01*v[1], m01*v[0]+m11*v[1] }; }
    inline double   determinant() const {return m00 * m11 - m01 * m01; }

    // isotropic random symmetric positive definite matrix with ln eigenvalues ~ N(0,lnEigStdev):
    static MatS2D   randSpd(double lnEigStdev);
};

inline MatS2D       outerProductSelf(Vec2D v) {return {sqr(v[0]), sqr(v[1]), v[0]*v[1]}; }

struct      MatUT2D             // upper triangular 2x2 double matrix
{
    double          m00,m11,m01;
    FG_SER3(m00,m11,m01)

    inline Vec2D    operator*(Vec2D v) const {return {m00*v[0]+m01*v[1],m11*v[1]}; }    // M * vec
    inline MatUT2D  operator*(double s) const {return {m00*s,m11*s,m01*s}; }            // scale elements of M
    inline MatS2D   luProduct() const {return {sqr(m00),sqr(m01)+sqr(m11),m00*m01}; }   // M^T * M
    inline double   determinant() const {return m00*m11; }
};

// Symmetric 3x3 double matrix:
struct      MatS3D
{
    Arr<double,3>       diag;
    Arr<double,3>       offd;       // In order 01, 02, 12
    FG_SER2(diag,offd);

    MatS3D() {}
    explicit MatS3D(double fillVal) {diag.fill(fillVal); offd.fill(fillVal); }
    explicit MatS3D(Mat33D const &);   // Checks for symmetry
    MatS3D(Arr<double,3> const & d,Arr<double,3> const & o) : diag(d), offd(o)  {}

    double              rc(size_t row,size_t col) const
    {
        FGASSERT((row<3)&&(col<3));
        if (row==col)
            return diag[row];
        else
            return offd[row+col-1];
    }
    Mat33D              asMatC() const
    {
        return Mat33D {diag[0],offd[0],offd[1], offd[0],diag[1],offd[2], offd[1],offd[2],diag[2]};
    }
    void                operator+=(MatS3D const & rhs) {diag += rhs.diag; offd += rhs.offd; }
    MatS3D              operator+(MatS3D const & rhs) const {return {diag+rhs.diag,offd+rhs.offd}; }
    Mat33D              operator+(Mat33D const & rhs) const;
    MatS3D              operator-(MatS3D const & rhs) const {return {diag-rhs.diag,offd-rhs.offd}; }
    MatS3D              operator*(double rhs) const {return {diag*rhs,offd*rhs}; }
    Vec3D               operator*(Vec3D rhs) const;
    // product of symmetric matrices is not in general symmetric:
    // (however powers of a symmetric matrix are symmetric)
    Mat33D              operator*(MatS3D const & rhs) const;
    Mat33D              operator*(Mat33D const & rhs) const;
    Vec3D               diagonal() const {return Vec3D{diag}; }
    double              sumElems() const {return cSum(diag) + 2.0 * cSum(offd); }
    // Sum of squares of elements (== square of frobenius norm):
    double              mag() const {return cMag(diag) + 2.0 * cMag(offd); }
    double              dot(MatS3D const & rhs) const {return cDot(diag,rhs.diag) + 2.0 * cDot(offd,rhs.offd); }
    double              determinant() const;
    MatS3D              inverse() const;
    MatS3D              square() const;     // the square of a symmetric matrix is also symmetric

    static MatS3D       identity() {return { {{1,1,1}} , {{0,0,0}} }; }
    // isotropic random symmetric positive definite matrix with ln eigenvalues ~ N(0,lnEigStdev):
    static MatS3D       randSpd(double lnEigStdev);
};
std::ostream &      operator<<(std::ostream & os,MatS3D const & mat);
inline double       cMag(MatS3D const & mat) {return mat.mag(); }
inline double       cSum(MatS3D const & mat) {return mat.sumElems(); }
inline double       cDot(MatS3D const & l,MatS3D const & r) {return l.dot(r); }
inline double       cDeterminant(MatS3D const & mat) {return mat.determinant(); }
inline MatS3D       cInverse(MatS3D const & mat) {return mat.inverse(); }
// apply *only* a quadratic form (no factor of 1/2, no constant):
inline double       cQuadForm(MatS3D const & Q,Vec3D const & x_m) {return cDot(x_m,Q*x_m); }
inline MatS3D       mapSqr(MatS3D const & mat) {return MatS3D {mapSqr(mat.diag),mapSqr(mat.offd)}; }
inline MatS3D       mapMul(MatS3D const & mat,MatS3D const & n)
{
    return {mapMul(mat.diag,n.diag),mapMul(mat.offd,n.offd)};
}
inline double       sumElems(MatS3D const & mat) {return cSum(mat.diag) + cSum(mat.offd) * 2; }
inline MatS3D       outerProductSelf(Vec3D v)
{
    return          {{mapSqr(v.m)},{{v[0]*v[1],v[0]*v[2],v[1]*v[2]}}};
}

struct      MeanCov3
{
    Vec3D           mean;       // mean from a set of samples
    MatS3D          cov;        // covariance of samples after zero-mean correction
};
MeanCov3            cMeanCov(Vec3Ds const & samps);

// Upper triangular 3x3 matrix
struct      MatUT3D
{
    Arr<double,6>       m;              // non-zero elements in row-major order
    FG_SER1(m)

    MatUT3D() {}
    explicit MatUT3D(Arr<double,6> const & data) : m{data} {}
    MatUT3D(double s0,double s1,double s2) : m{{s0,0,0,s1,0,s2}} {}         // diagonal matrix
    MatUT3D(double m0,double m1,double m2,double m3,double m4,double m5) : m{{m0,m1,m2,m3,m4,m5}} {}
    explicit MatUT3D(Arr3D const & s) : m{{s[0],0,0,s[1],0,s[2]}} {}        // diagonal matrix
    MatUT3D(Arr3D const & diag,Arr3D const & offd) : m{{diag[0],offd[0],offd[1],diag[1],offd[2],diag[2]}} {}

    static MatUT3D  identity()  {return MatUT3D{1,1,1}; }
    static MatUT3D  diagonal(double v) {return MatUT3D{v,v,v}; }
    static MatUT3D  zeros() {return {0,0,0,0,0,0}; }

    MatUT3D         operator-(MatUT3D const & rhs) const {return MatUT3D(m-rhs.m); }
    MatUT3D         operator*(double s) const {return MatUT3D {m*s}; }
    MatUT3D         operator/(double s) const {return  MatUT3D {m/s}; }
    Vec3D           operator*(Vec3D v) const
    {
        return Vec3D {m[0]*v[0]+m[1]*v[1]+m[2]*v[2], m[3]*v[1]+m[4]*v[2], m[5]*v[2]};
    }
    Vec3D           tranposeMul(Vec3D r)    // U^T * v
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
std::ostream &      operator<<(std::ostream &,MatUT3D const &);
inline double       cDeterminant(MatUT3D const & mat) {return mat.determinant(); }
inline double       cMag(MatUT3D const & mat) {return mat.mag(); }

template<class T,uint D>
struct      NameVec
{
    String          name;
    Mat<T,D,1>      vec;
    FG_SER2(name,vec)

    NameVec() {}
    NameVec(String const & n,Mat<T,D,1> const & v) : name{n}, vec{v} {}

    bool            operator==(NameVec const & r) const {return ((name==r.name)&&(vec==r.vec)); }
    bool            operator==(String const & n) const {return (name==n); }     // easy lookup
};
typedef NameVec<float,2>    NameVec2F;
typedef Svec<NameVec2F>     NameVec2Fs;
typedef Svec<NameVec2Fs>    NameVec2Fss;
typedef NameVec<float,3>    NameVec3F;
typedef Svec<NameVec3F>     NameVec3Fs;
template<class T,uint D>
std::ostream &      operator<<(std::ostream & os,NameVec<T,D> const & nv)
{
    return os << nv.name << ": " << nv.vec;
}
// select nameVecs in order of names and return the vecs. Throws if any of names is not found in nameVecs:
template<class T,uint D>
Svec<Mat<T,D,1>>    selectVecsByName(Svec<NameVec<T,D>> const & nameVecs,Strings const & names)
{
    Svec<Mat<T,D,1>>    ret; ret.reserve(names.size());
    for (String const & name : names)
        ret.push_back(findFirst(nameVecs,name).vec);
    return ret;
}

}

#endif
