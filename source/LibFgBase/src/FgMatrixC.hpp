//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Feb 22, 2005
//
// Functions operating on FgMatrixC<>

#ifndef FGMATRIXC_HPP
#define FGMATRIXC_HPP

#include "FgStdLibs.hpp"

#include "FgMatrixCBase.hpp"
#include "FgVariant.hpp"
#include "FgRandom.hpp"
#include "FgAlgs.hpp"
#include "FgOpt.hpp"

template<class T,uint nrows,uint ncols>
struct  FgTraits<FgMatrixC<T,nrows,ncols> >
{
    typedef FgMatrixC<typename FgTraits<T>::Accumulator,nrows,ncols>  Accumulator;
    typedef FgMatrixC<typename FgTraits<T>::Floating,nrows,ncols>     Floating;
    typedef T                                                               Scalar;
};

template<class T,uint nrows,uint ncols>
FgMatrixC<T,nrows,ncols>
operator*(T val,const FgMatrixC<T,nrows,ncols> & mat)
{return (mat * val); }

template <class T,uint nrows,uint ncols,uint ncols2>
FgMatrixC<T,nrows,ncols2>
operator*(
    const FgMatrixC<T,nrows,ncols> &    v1,
    const FgMatrixC<T,ncols,ncols2> &   v2)
{
    FgMatrixC<T,nrows,ncols2>      newMat;
    for (uint ii=0; ii<nrows; ii++) {
        for (uint jj=0; jj<ncols2; jj++) {
            newMat[ii*ncols2+jj] = T(0);
            for (uint kk=0; kk<ncols; kk++)
                newMat[ii*ncols2+jj] += v1[ii*ncols+kk] * v2[kk*ncols2+jj];
        }
    }
    return newMat;
}

template <class T>
FgMatrixC<T,2,2>
fgMatRotate(T radians)
{
    FgMatrixC<T,2,2> mat;
    FgTypeAttributeFloatingS<T>();
    T ct = T(cos(radians));
    T st = T(sin(radians));
    mat.elem(0,0)=ct;    mat.elem(0,1)=-st;
    mat.elem(1,0)=st;    mat.elem(1,1)=ct;
    return mat;
}

template <class T>
FgMatrixC<T,3,3>
fgMatRotateX(T radians)        // RHR rotation around X axis
{
    FgMatrixC<T,3,3> mat;
    FgTypeAttributeFloatingS<T>();
    T ct = (T)cos(radians);
    T st = (T)sin(radians);
    mat.elem(0,0)=1.0;   mat.elem(0,1)=0.0;   mat.elem(0,2)=0.0;
    mat.elem(1,0)=0.0;   mat.elem(1,1)=ct;    mat.elem(1,2)=-st;
    mat.elem(2,0)=0.0;   mat.elem(2,1)=st;    mat.elem(2,2)=ct;
    return mat;
}

template <class T>
FgMatrixC<T,3,3>
fgMatRotateY(T radians)        // RHR rotation around Y axis
{
    FgMatrixC<T,3,3> mat;
    FgTypeAttributeFloatingS<T>();
    T ct = (T)cos(radians);
    T st = (T)sin(radians);
    mat.elem(0,0)=ct;    mat.elem(0,1)=0.0;   mat.elem(0,2)=st;
    mat.elem(1,0)=0.0;   mat.elem(1,1)=1.0;   mat.elem(1,2)=0.0;
    mat.elem(2,0)=-st;   mat.elem(2,1)=0.0;   mat.elem(2,2)=ct;
    return mat;
}

template <class T>
FgMatrixC<T,3,3>
fgMatRotateZ(T radians)        // RHR rotation around Z axis
{
    FgMatrixC<T,3,3> mat;
    FgTypeAttributeFloatingS<T>();
    T ct = (T)cos(radians);
    T st = (T)sin(radians);
    mat.elem(0,0)=ct;    mat.elem(0,1)=-st;   mat.elem(0,2)=0.0;
    mat.elem(1,0)=st;    mat.elem(1,1)=ct;    mat.elem(1,2)=0.0;
    mat.elem(2,0)=0.0;   mat.elem(2,1)=0.0;   mat.elem(2,2)=1.0;
    return mat;
}

template <class T>
FgMatrixC<T,3,3>
fgMatRotateAxis(                   // RHR rotation around an arbitrary axis
    T           radians, 
    const       FgMatrixC<T,3,1> &axis)             // vector describing that axis
{
    FgMatrixC<T,3,3> mat;
    FgTypeAttributeFloatingS<T>();
    T           len = axis.length();
    FGASSERT(len > T(0));
    FgMatrixC<T,3,1>    ax = axis * (T(1) / len);   // Ensure the rotation axis is normalized
    T           ct = (T)cos(radians);
    T           st = (T)sin(radians);
    T           vt = T(1)-ct;
    mat.elem(0,0) = ax[0]*ax[0]*vt + ct;
    mat.elem(0,1) = ax[0]*ax[1]*vt - ax[2]*st;
    mat.elem(0,2) = ax[0]*ax[2]*vt + ax[1]*st;
    mat.elem(1,0) = ax[0]*ax[1]*vt + ax[2]*st;
    mat.elem(1,1) = ax[1]*ax[1]*vt + ct;
    mat.elem(1,2) = ax[1]*ax[2]*vt - ax[0]*st;
    mat.elem(2,0) = ax[0]*ax[2]*vt - ax[1]*st;
    mat.elem(2,1) = ax[1]*ax[2]*vt + ax[0]*st;
    mat.elem(2,2) = ax[2]*ax[2]*vt + ct;
    return mat;
}

template<class T>
T
fgDeterminant(const FgMatrixC<T,2,2> & mat)
{return (mat[0]*mat[3] - mat[1]*mat[2]); }

template<class T>
T
fgDeterminant(const FgMatrixC<T,3,3> & mat)
{
    return (
        mat[0]*mat[4]*mat[8] + mat[1]*mat[5]*mat[6] + mat[2]*mat[3]*mat[7]
        - mat[2]*mat[4]*mat[6] - mat[1]*mat[3]*mat[8] - mat[0]*mat[5]*mat[7]);
}

// Concatenate an element onto a column vector:
template<class T,uint dim>
FgMatrixC<T,dim+1,1>
fgConcat(FgMatrixC<T,dim,1> vec,T val)
{
    FgMatrixC<T,dim+1,1>    ret;
    for (uint ii=0; ii<dim; ++ii)
        ret[ii] = vec[ii];
    ret[dim] = val;
    return ret;
}
// Concatenate an element onto a row vector:
template<class T,uint dim>
FgMatrixC<T,1,dim+1>
fgConcat(FgMatrixC<T,1,dim> vec,T val)
{
    FgMatrixC<T,1,dim+1>    ret;
    for (uint ii=0; ii<dim; ++ii)
        ret[ii] = val[ii];
    ret[dim] = val;
    return ret;
}

template<class T,uint dim>
FgMatrixC<T,dim+1,1>
fgAsHomogVec(FgMatrixC<T,dim,1> v)
{
    FgMatrixC<T,dim+1,1>    ret;
    for (uint ii=0; ii<dim; ++ii)
        ret[ii] = v[ii];
    ret[dim] = T(1);
    return ret;
}

template<class T,uint dim>
FgMatrixC<T,dim-1,1>
fgFromHomogVec(FgMatrixC<T,dim,1> v)
{
    FgMatrixC<T,dim-1,1>    ret;
    T                       w = v[dim-1];
    for (uint ii=0; ii<dim-1; ++ii)
        ret[ii] = v[ii] / w;
    return ret;
}

// Return homogenous matrix representation of an affine transform:
template<class T, uint dims>
FgMatrixC<T,dims+1,dims+1>
fgAsHomogMat(
    const FgMatrixC<T,dims,dims>  & linTrans,
    const FgMatrixC<T,dims,1>     & translation)
{
    FgMatrixC<T,dims+1,dims+1>    ret;
    for (uint rr=0; rr<dims; rr++)
        for (uint cc=0; cc<dims; cc++)
            ret.elem(rr,cc) = linTrans.elem(rr,cc);
    for (uint rr=0; rr<dims; rr++)
        ret.elem(rr,dims) = translation[rr];
    ret.elem(dims,dims) = 1;
    return ret;
}

// Return homogenous matrix representation of a linear transform:
template<class T, uint dims>
FgMatrixC<T,dims+1,dims+1>
fgAsHomogMat(const FgMatrixC<T,dims,dims> & linear)
{
    FgMatrixC<T,dims+1,dims+1>    ret;
    for (uint rr=0; rr<dims; ++rr)
        for (uint cc=0; cc<dims; ++cc)
            ret.elem(rr,cc) = linear.elem(rr,cc);
    ret.elem(dims,dims) = T(1);
    return ret;
}

// Return homogenous matrix representation of a translation:
template<class T, uint dims>
FgMatrixC<T,dims+1,dims+1>
fgAsHomogMat(const FgMatrixC<T,dims,1> & translation)
{
    FgMatrixC<T,dims+1,dims+1>    ret;
    ret.setIdentity();
    for (uint rr=0; rr<dims; rr++)
        ret.elem(rr,dims) = translation[rr];
    return ret;
}

// RETURNS: The inverse of an invertible matrix. Throws an FGASSERT if not invertible.
template <class T>
FgMatrixC<T,2,2> fgMatInverse(
    const FgMatrixC<T,2,2>&      m)
{
    FgTypeAttributeFloatingS<T>();
    FgMatrixC<T,2,2>     retval;
    T   fac = (m.elem(0,0) * m.elem(1,1) - m.elem(0,1) * m.elem(1,0));
    FGASSERT(fac != T(0));
    fac = T(1) / fac;
    retval.elem(0,0) = m.elem(1,1) * fac;
    retval.elem(0,1) = - m.elem(0,1) * fac;
    retval.elem(1,0) = - m.elem(1,0) * fac;
    retval.elem(1,1) = m.elem(0,0) * fac;
    return retval;
}
template <class T>
FgMatrixC<T,3,3> fgMatInverse(
    const FgMatrixC<T,3,3>&      m)
{
    FgTypeAttributeFloatingS<T>();
    FgMatrixC<T,3,3>     r;
    T   fac = (m.elem(0,0)*m.elem(1,1)*m.elem(2,2) - m.elem(0,0)*m.elem(1,2)*m.elem(2,1) +
               m.elem(1,0)*m.elem(0,2)*m.elem(2,1) - m.elem(1,0)*m.elem(0,1)*m.elem(2,2) +
               m.elem(2,0)*m.elem(0,1)*m.elem(1,2) - m.elem(2,0)*m.elem(1,1)*m.elem(0,2));
    FGASSERT(fac != T(0));
    r.elem(0,0) = m.elem(1,1) * m.elem(2,2) - m.elem(1,2) * m.elem(2,1);
    r.elem(0,1) = m.elem(0,2) * m.elem(2,1) - m.elem(0,1) * m.elem(2,2);
    r.elem(0,2) = m.elem(0,1) * m.elem(1,2) - m.elem(0,2) * m.elem(1,1);
    r.elem(1,0) = m.elem(1,2) * m.elem(2,0) - m.elem(1,0) * m.elem(2,2);
    r.elem(1,1) = m.elem(0,0) * m.elem(2,2) - m.elem(0,2) * m.elem(2,0);
    r.elem(1,2) = m.elem(0,2) * m.elem(1,0) - m.elem(0,0) * m.elem(1,2);
    r.elem(2,0) = m.elem(1,0) * m.elem(2,1) - m.elem(1,1) * m.elem(2,0);
    r.elem(2,1) = m.elem(0,1) * m.elem(2,0) - m.elem(0,0) * m.elem(2,1);
    r.elem(2,2) = m.elem(0,0) * m.elem(1,1) - m.elem(1,0) * m.elem(0,1);
    r *= T(1) / fac;
    return r;
}

template <class T,uint nrows,uint ncols>
T
fgDot(
    const FgMatrixC<T,nrows,ncols> & lhs,
    const FgMatrixC<T,nrows,ncols> & rhs)
{
    T           acc(0);
    for (uint ii=0; ii<nrows*ncols; ++ii)
        acc += lhs[ii] * rhs[ii];
    return acc;
}

template<typename T>
FgMatrixC<T,3,1> fgCrossProduct(
    const FgMatrixC<T,3,1>& v1,
    const FgMatrixC<T,3,1>& v2)
{
    FgMatrixC<T,3,1>      r;
    r[0] = v1[1] * v2[2] - v1[2] * v2[1];
    r[1] = v1[2] * v2[0] - v1[0] * v2[2];
    r[2] = v1[0] * v2[1] - v1[1] * v2[0];
    return r;
}

// Component-wise multiply:
template<typename T,uint nrows,uint ncols>
FgMatrixC<T,nrows,ncols>
fgMultiply(
    const FgMatrixC<T,nrows,ncols> &    lhs,
    const FgMatrixC<T,nrows,ncols> &    rhs)
{
    FgMatrixC<T,nrows,ncols>    ret;
    for (uint ii=0; ii<nrows*ncols; ++ii)
        ret[ii] = lhs[ii] * rhs[ii];
    return ret;
}

// Component-wise divide:
template<typename T,uint nrows,uint ncols>
FgMatrixC<T,nrows,ncols>
fgDivide(
    const FgMatrixC<T,nrows,ncols> &    lhs,
    const FgMatrixC<T,nrows,ncols> &    rhs)
{
    FgMatrixC<T,nrows,ncols>            ret;
    for (uint ii=0; ii<nrows*ncols; ++ii)
        ret[ii] = lhs[ii] / rhs[ii];
    return ret;
}

template<uint nrows,uint ncols>
FgMatrixC<double,nrows,ncols> 
fgMatRandNormal()
{
    FgMatrixC<double,nrows,ncols>   vec;
    for (uint ii=0; ii<nrows*ncols; ++ii)
        vec[ii] = fgRandNormal();
    return vec;
}

template<uint nrows,uint ncols>
FgMatrixC<double,nrows,ncols> 
fgMatRandUniform(double lo,double hi)
{
    FgMatrixC<double,nrows,ncols>   vec;
    for (uint ii=0; ii<nrows*ncols; ++ii)
        vec[ii] = fgRandUniform(lo,hi);
    return vec;
}

template<typename T,uint nrows,uint ncols>
FgMatrixC<T,nrows,ncols>
fgFloor(const FgMatrixC<T,nrows,ncols> & mat)
{
    FgMatrixC<T,nrows,ncols> ret;
    for (uint ii=0; ii<mat.numElems(); ++ii)
        ret[ii] = std::floor(mat[ii]);
    return ret;
}

template<typename Flt,typename Int,uint nrows,uint ncols>
void
fgRound_(
    const FgMatrixC<Flt,nrows,ncols> &    lhs,
    FgMatrixC<Int,nrows,ncols> &          rhs)
{
    for (uint ii=0; ii<rhs.numElems(); ++ii)
        fgRound_(lhs[ii],rhs[ii]);
}

template<uint nrows,uint ncols>
FgMatrixC<int,nrows,ncols>
fgRound(FgMatrixC<double,nrows,ncols> m)
{
    FgMatrixC<int,nrows,ncols> ret;
    for (uint ii=0; ii<m.numElems(); ++ii)
        ret[ii] = fgRound(m[ii]);
    return ret;
}

template<uint nrows,uint ncols>
FgMatrixC<int,nrows,ncols>
fgRound(FgMatrixC<float,nrows,ncols> m)
{
    FgMatrixC<int,nrows,ncols> ret;
    for (uint ii=0; ii<m.numElems(); ++ii)
        ret[ii] = fgRound(m[ii]);
    return ret;
}

template<uint nrows,uint ncols>
FgMatrixC<uint,nrows,ncols>
fgRoundU(FgMatrixC<double,nrows,ncols> m)
{
    FgMatrixC<uint,nrows,ncols> ret;
    for (uint ii=0; ii<m.numElems(); ++ii)
        ret[ii] = fgRoundU(m[ii]);
    return ret;
}

template<uint nrows,uint ncols>
FgMatrixC<uint,nrows,ncols>
fgRoundU(FgMatrixC<float,nrows,ncols> m)
{
    FgMatrixC<uint,nrows,ncols> ret;
    for (uint ii=0; ii<m.numElems(); ++ii)
        ret[ii] = fgRoundU(m[ii]);
    return ret;
}

// Create a wider matrix by concatenating rows from 2 matrices:
template<class T,uint nrows,uint ncols1,uint ncols2>
FgMatrixC<T,nrows,ncols1+ncols2>
fgConcatHoriz(
    const FgMatrixC<T,nrows,ncols1> & lhs,
    const FgMatrixC<T,nrows,ncols2> & rhs)
{
    FgMatrixC<T,nrows,ncols1+ncols2>    ret;
    for (uint row=0; row<nrows; ++row)
    {
        uint    col=0;
        for (uint cc=0; cc<ncols1; ++cc)
            ret.elem(row,col++) = lhs.elem(row,cc);
        for (uint cc=0; cc<ncols2; ++cc)
            ret.elem(row,col++) = rhs.elem(row,cc);
    }
    return ret;
}

// Create a wider matrix by concatenating rows from 3 matrices:
template<class T,uint nrows,uint ncols1,uint ncols2,uint ncols3>
FgMatrixC<T,nrows,ncols1+ncols2+ncols3>
fgConcatHoriz(
    const FgMatrixC<T,nrows,ncols1> & m1,
    const FgMatrixC<T,nrows,ncols2> & m2,
    const FgMatrixC<T,nrows,ncols3> & m3)
{
    FgMatrixC<T,nrows,ncols1+ncols2+ncols3> ret;
    for (uint row=0; row<nrows; ++row)
    {
        uint    col=0;
        for (uint cc=0; cc<ncols1; ++cc)
            ret.elem(row,col++) = m1.elem(row,cc);
        for (uint cc=0; cc<ncols2; ++cc)
            ret.elem(row,col++) = m2.elem(row,cc);
        for (uint cc=0; cc<ncols3; ++cc)
            ret.elem(row,col++) = m3.elem(row,cc);
    }
    return ret;
}

// Create a taller matrix by concatenating cols from 2 matrices:
template<class T,uint nrows1,uint nrows2,uint ncols>
FgMatrixC<T,nrows1+nrows2,ncols>
fgConcatVert(
    const FgMatrixC<T,nrows1,ncols> & upper,
    const FgMatrixC<T,nrows2,ncols> & lower)
{
    FgMatrixC<T,nrows1+nrows2,ncols>    ret;
    for (uint rr=0; rr<nrows1; ++rr)
        for (uint cc=0; cc<ncols; ++cc)
            ret.elem(rr,cc) = upper.elem(rr,cc);
    for (uint rr=0; rr<nrows2; ++rr)
        for (uint cc=0; cc<ncols; ++cc)
            ret.elem(nrows1+rr,cc) = lower.elem(rr,cc);
    return ret;
}

// Create a taller matrix by concatenating a given value to all columns:
template<class T,uint nrows,uint ncols>
FgMatrixC<T,nrows+1,ncols>
fgConcatVert(const FgMatrixC<T,nrows,ncols> & mat,T val)
{
    FgMatrixC<T,nrows+1,ncols>  ret;
    uint    ii=0;
    for (; ii<nrows*ncols; ++ii)
        ret[ii] = mat[ii];
    ret[ii] = val;
    return ret;
}

// Parameterized unmirrored permutation of axes in 3D:
template<class T>
FgMatrixC<T,3,3>
fgPermuteAxes(uint axisToBecomeX)
{
    FgMatrixC<T,3,3>    ret;
    for (uint ii=0; ii<3; ++ii)
        ret.elem(ii,(ii+axisToBecomeX)%3) = T(1);
    return ret;
}

template<class T,uint nrows,uint ncols>
bool
fgIsPow2(const FgMatrixC<T,nrows,ncols> & mat)
{
    for (uint ii=0; ii<nrows*ncols; ++ii)
        if (!fgIsPow2(mat[ii]))
            return false;
    return true;
}

template<class T,uint nrows,uint ncols>
T
fgMean(const FgMatrixC<T,nrows,ncols> & mat)
{
    typedef typename FgTraits<T>::Accumulator Acc;
    typedef typename FgTraits<T>::Scalar      Scal;
    Acc     acc = Acc(mat[0]);
    for (uint ii=1; ii<nrows*ncols; ++ii)
        acc += Acc(mat[ii]);
    return T(acc / Scal(nrows*ncols));
}

#define FG_MATRIXC_ELEMWISE(matFunc,elemFunc)               \
    template<class T,uint nrows,uint ncols>                 \
    FgMatrixC<T,nrows,ncols>                                \
    matFunc (const FgMatrixC<T,nrows,ncols> & mat)          \
    {                                                       \
        FgMatrixC<T,nrows,ncols>    ret;                    \
        for (uint ii=0; ii<nrows*ncols; ++ii)               \
            ret[ii] = elemFunc (mat[ii]);                   \
        return ret;                                         \
    }

FG_MATRIXC_ELEMWISE(fgPower2Ceil,fgPower2Ceil)
FG_MATRIXC_ELEMWISE(fgAbs,std::abs)
FG_MATRIXC_ELEMWISE(fgSquare,fgSqr)
FG_MATRIXC_ELEMWISE(fgLog,std::log)
FG_MATRIXC_ELEMWISE(fgExp,std::exp)
FG_MATRIXC_ELEMWISE(fgSqrt,std::sqrt)

template<class T,uint nrows,uint ncols>
typename FgTraits<T>::Accumulator
fgDot(
    const vector<FgMatrixC<T,nrows,ncols> > & v0,
    const vector<FgMatrixC<T,nrows,ncols> > & v1)
{
    FGASSERT(v0.size() == v1.size());
    typename FgTraits<T>::Accumulator acc(0);
    for (size_t ii=0; ii<v0.size(); ++ii)
        acc += fgDot(v0[ii],v1[ii]);
    return acc;
}

template<uint nrows,uint ncols>
FgMatrixC<bool,nrows,ncols>
fgOr(FgMatrixC<bool,nrows,ncols> v0,FgMatrixC<bool,nrows,ncols> v1)
{
    FgMatrixC<bool,nrows,ncols> ret;
    for (uint ii=0; ii<nrows*ncols; ++ii)
        ret[ii] = v0[ii] || v1[ii];
    return ret;
}

template<typename T,uint dim>
T
fgTrace(const FgMatrixC<T,dim,dim> & m)
{
    T   ret(0);
    for (uint ii=0; ii<m.numElems(); ii+=dim+1)
        ret += m[ii];
    return ret;
}

template<typename Flt,typename Int,uint dim>
void
fgUninterpolate(
    const FgMatrixC<Flt,dim,1> &    coord,
    FgMatrixC<Int,dim,1> &          coordLo,    // Returned
    FgMatrixC<Flt,dim,2> &          weights)    // Returned
{
    FgMatrixC<Flt,dim,1>    coordL = fgFloor(coord),
                            coordH = coordL + FgMatrixC<Flt,dim,1>(1);
    weights = fgConcatHoriz(coordH-coord,coord-coordL);
    coordLo = FgMatrixC<Int,dim,1>(coordL);
}

template<typename T,uint nrows,uint ncols>
bool
fgLt(
    const FgMatrixC<T,nrows,ncols> & m0,
    const FgMatrixC<T,nrows,ncols> & m1)
{
    for (uint ii=0; ii<nrows*ncols; ++ii)
        if (!(m0[ii] < m1[ii]))
            return false;
    return true;
}

template<class T,uint nrows,uint ncols>
T
fgSumElems(const FgMatrixC<T,nrows,ncols> & m)
{
    T   ret(m[0]);
    for (uint ii=1; ii<nrows*ncols; ++ii)
        ret += m[ii];
    return ret;
}

template<class T,uint sz>
FgMatrixC<T,sz,sz>
fgDiagonal(FgMatrixC<T,sz,1> vec)
{
    FgMatrixC<T,sz,sz>      ret;
    for (uint ii=0; ii<sz; ++ii)
        ret.elem(ii,ii) = vec[ii];
    return ret;
}

template<typename T,uint nrows,uint ncols>
FgMatrixC<T,nrows,ncols>
fgNormalize(FgMatrixC<T,nrows,ncols> m)
{
    return m / m.length();
}

template<typename T,uint nrows,uint ncols>
FgMatrixC<T,nrows,ncols>
fgMap(FgMatrixC<T,nrows,ncols> m,T(*func)(T))
{
    FgMatrixC<T,nrows,ncols>    ret;
    for (uint ii=0; ii<nrows*ncols; ++ii)
        ret[ii] = func(m[ii]);
    return ret;
}

template<class T,uint dim>
FgMatrixC<T,dim,1>
fgClamp(FgMatrixC<T,dim,1> val,FgMatrixC<T,dim,2> bounds)
{
    for (uint ii=0; ii<dim; ++ii)
        val[ii] = fgClamp(val[ii],bounds.elem(ii,0),bounds.elem(ii,1));
    return val;
}

template<class T,uint nrows,uint ncols>
vector<T>
fgMapMag(const vector<FgMatrixC<T,nrows,ncols> > & v)
{
    vector<T>   ret(v.size());
    for (size_t ii=0; ii<v.size(); ++ii)
        ret[ii] = fgLengthSqr(v[ii]);
    return ret;
}

template<class T,uint nrows,uint ncols>
T
fgSsd(
    const vector<FgMatrixC<T,nrows,ncols> > &  v0,
    const vector<FgMatrixC<T,nrows,ncols> > &  v1)
{
    FGASSERT(v0.size() == v1.size());
    double      acc(0);
    for (size_t ii=0; ii<v0.size(); ++ii)
        acc += (v1[ii]-v0[ii]).lengthSqr();
    return T(acc);
}

template<typename T,uint nrows,uint ncols>
T
fgRms(const vector<FgMatrixC<T,nrows,ncols> > & v)
{
    T       acc(0);
    for (size_t ii=0; ii<v.size(); ++ii)
        acc += v[ii].lengthSqr();
    acc /= T(v.size());
    return acc;
}

// Find first index of an element in a vector. Return 'size' if not found:
template<typename T,uint nrows>
uint
fgFindFirstIdx(FgMatrixC<T,nrows,1> m,T v)
{
    for (uint ii=0; ii<nrows; ++ii)
        if (m[ii] == v)
            return ii;
    return nrows;
}

// Solve matrix equation of the form Ax = b. Returns x if solvable, invalid if degenerate:
FgOpt<FgVect2F>
fgSolve(FgMat22F A,FgVect2F b);
FgOpt<FgVect3F>
fgSolve(FgMat33F A,FgVect3F b);
FgOpt<FgVect4D>
fgSolve(FgMat44D A,FgVect4D b);
FgOpt<FgVect4F>
fgSolve(FgMat44F A,FgVect4F b);

// Elements must all be non-zero:
template<uint nrows,uint ncols>
bool
fgIsIntegerSizeMultiple(FgMatrixC<uint,nrows,ncols> m0,FgMatrixC<uint,nrows,ncols> m1)
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

template<uint nrows,uint ncols>
uint
fgL1Norm(FgMatrixC<int,nrows,ncols> m)
{
    uint        ret(0);
    for (uint ii=0; ii<nrows*ncols; ++ii)
        ret += uint(std::abs(m.m[ii]));
    return ret;
}

template<uint nrows,uint ncols>
double
fgL1Norm(FgMatrixC<double,nrows,ncols> m)
{
    double      ret(0);
    for (uint ii=0; ii<nrows*ncols; ++ii)
        ret += std::abs(m.m[ii]);
    return ret;
}

template<typename T,uint nrows,uint ncols>
bool
fgNoZeros(FgMatrixC<T,nrows,ncols> m)
{
    T   acc = T(1);
    for (uint ii=0; ii<nrows*ncols; ++ii)
        acc *= m.m[ii];
    return (acc != T(0));
}

template<uint nrows,uint ncols>
FgMatrixC<double,nrows,ncols>
fgF2D(const FgMatrixC<float,nrows,ncols> & m)
{return FgMatrixC<double,nrows,ncols>(m); }

template<uint nrows,uint ncols>
FgMatrixC<float,nrows,ncols>
fgD2F(const FgMatrixC<double,nrows,ncols> & m)
{return FgMatrixC<float,nrows,ncols>(m); }

#endif
