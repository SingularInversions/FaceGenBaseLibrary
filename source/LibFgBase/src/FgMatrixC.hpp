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
#include "FgRandom.hpp"
#include "FgMath.hpp"
#include "FgOpt.hpp"

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
    mat.rc(0,0)=ct;    mat.rc(0,1)=-st;
    mat.rc(1,0)=st;    mat.rc(1,1)=ct;
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
    mat.rc(0,0)=1.0;   mat.rc(0,1)=0.0;   mat.rc(0,2)=0.0;
    mat.rc(1,0)=0.0;   mat.rc(1,1)=ct;    mat.rc(1,2)=-st;
    mat.rc(2,0)=0.0;   mat.rc(2,1)=st;    mat.rc(2,2)=ct;
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
    mat.rc(0,0)=ct;    mat.rc(0,1)=0.0;   mat.rc(0,2)=st;
    mat.rc(1,0)=0.0;   mat.rc(1,1)=1.0;   mat.rc(1,2)=0.0;
    mat.rc(2,0)=-st;   mat.rc(2,1)=0.0;   mat.rc(2,2)=ct;
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
    mat.rc(0,0)=ct;    mat.rc(0,1)=-st;   mat.rc(0,2)=0.0;
    mat.rc(1,0)=st;    mat.rc(1,1)=ct;    mat.rc(1,2)=0.0;
    mat.rc(2,0)=0.0;   mat.rc(2,1)=0.0;   mat.rc(2,2)=1.0;
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
fgDeterminant(const FgMatrixC<T,2,2> & mat)
{return (mat[0]*mat[3] - mat[1]*mat[2]); }

// Useful for finding the discriminant, for instance of aspect ratios:
template<class T>
T
fgDeterminant(const FgMatrixC<T,2,1> & col0,const FgMatrixC<T,2,1> & col1)
{return col0[0]*col1[1] - col0[1]*col1[0]; }

template<class T>
T
fgDeterminant(const FgMatrixC<T,3,3> & mat)
{
    return (
        mat[0]*mat[4]*mat[8] +
        mat[1]*mat[5]*mat[6] +
        mat[2]*mat[3]*mat[7] -
        mat[2]*mat[4]*mat[6] -
        mat[1]*mat[3]*mat[8] -
        mat[0]*mat[5]*mat[7]);
}

// Concatenate an element onto a column vector:
template<class T,uint dim>
FgMatrixC<T,dim+1,1>
fgCat(const FgMatrixC<T,dim,1> & vec,T val)
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
fgCat(const FgMatrixC<T,1,dim> & vec,T val)
{
    FgMatrixC<T,1,dim+1>    ret;
    for (uint ii=0; ii<dim; ++ii)
        ret[ii] = vec[ii];
    ret[dim] = val;
    return ret;
}

// Flatten a vector of matrices into a vector of scalars:
template<class T,uint nrows,uint ncols>
vector<T>
fgFlat(const vector<FgMatrixC<T,nrows,ncols> > & ms)
{
    vector<T>       ret;
    ret.reserve(ms.size()*nrows*ncols);
    for (size_t ii=0; ii<ms.size(); ++ii)
        for (uint jj=0; jj<nrows*ncols; ++jj)
            ret.push_back(ms[ii].m[jj]);
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
            ret.rc(rr,cc) = linTrans.rc(rr,cc);
    for (uint rr=0; rr<dims; rr++)
        ret.rc(rr,dims) = translation[rr];
    ret.rc(dims,dims) = 1;
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
            ret.rc(rr,cc) = linear.rc(rr,cc);
    ret.rc(dims,dims) = T(1);
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
        ret.rc(rr,dims) = translation[rr];
    return ret;
}

// RETURNS: The inverse of an invertible matrix. Throws an FGASSERT if not invertible.
template <class T>
FgMatrixC<T,2,2>
fgMatInverse(const FgMatrixC<T,2,2> & m)
{
    FgTypeAttributeFloatingS<T>();
    FgMatrixC<T,2,2>     ret;
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
FgMatrixC<T,3,3> fgMatInverse(
    const FgMatrixC<T,3,3>&      m)
{
    FgTypeAttributeFloatingS<T>();
    FgMatrixC<T,3,3>     r;
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
FgMatrixC<T,3,1>
fgCrossProduct(
    const FgMatrixC<T,3,1> & v1,
    const FgMatrixC<T,3,1> & v2)
{
    FgMatrixC<T,3,1>      r;
    r[0] = v1[1] * v2[2] - v1[2] * v2[1];
    r[1] = v1[2] * v2[0] - v1[0] * v2[2];
    r[2] = v1[0] * v2[1] - v1[1] * v2[0];
    return r;
}

// Element-wise multiplication (aka Hadamard product):
template<typename T,uint nrows,uint ncols>
FgMatrixC<T,nrows,ncols>
fgMapMul(
    const FgMatrixC<T,nrows,ncols> &    lhs,
    const FgMatrixC<T,nrows,ncols> &    rhs)
{
    FgMatrixC<T,nrows,ncols>    ret;
    for (uint ii=0; ii<nrows*ncols; ++ii)
        ret[ii] = lhs[ii] * rhs[ii];
    return ret;
}
template<typename T,uint nrows,uint ncols>
FgMatrixC<T,nrows,ncols>
fgMapMul(const FgMatrixC<T,nrows,ncols> & m0,const FgMatrixC<T,nrows,ncols> & m1,const FgMatrixC<T,nrows,ncols> & m2)
{
    FgMatrixC<T,nrows,ncols>    ret;
    for (uint ii=0; ii<nrows*ncols; ++ii)
        ret[ii] = m0[ii] * m1[ii] * m2[ii];
    return ret;
}

// Faster equivalent to lhs^T * rhs:
template<typename T,uint n0,uint n1,uint n2>
FgMatrixC<T,n0,n1>
fgTransposeMul(
    const FgMatrixC<T,n2,n0> &    lhs,
    const FgMatrixC<T,n2,n1> &    rhs)
{
    FgMatrixC<T,n0,n1>      ret(T(0));
    for (uint i0=0; i0<n0; ++i0)
        for (uint i1=0; i1<n1; ++i1)
            for (uint i2=0; i2<n2; ++i2)
                ret.rc(i0,i1) += lhs.rc(i2,i0) * rhs.rc(i2,i1);
    return ret;
}

// Element-wise division:
template<typename T,uint nrows,uint ncols>
FgMatrixC<T,nrows,ncols>
fgMapDiv(
    const FgMatrixC<T,nrows,ncols> &    lhs,
    const FgMatrixC<T,nrows,ncols> &    rhs)
{
    FgMatrixC<T,nrows,ncols>            ret;
    for (uint ii=0; ii<nrows*ncols; ++ii)
        ret[ii] = lhs[ii] / rhs[ii];
    return ret;
}

template<typename T,uint nrows,uint ncols>
FgMatrixC<T,nrows,ncols> 
fgMatRandNrm(T scale=1)
{
    FgMatrixC<T,nrows,ncols>    ret;
    for (uint ii=0; ii<nrows*ncols; ++ii)
        ret[ii] = static_cast<T>(fgRandNormal())*scale;
    return ret;
}

// Handy shortcut for type double vectors:
template<uint dim>
FgMatrixC<double,dim,1> 
fgVecRandNrm(double scale=1)
{
    FgMatrixC<double,dim,1>      vec;
    for (uint ii=0; ii<dim; ++ii)
        vec[ii] = fgRandNormal()*scale;
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

// Exponentiated normal distributed elements:
template<uint nrows,uint ncols>
FgMatrixC<double,nrows,ncols>
fgMatRandExp(double mean,double stdev)
{
    FgMatrixC<double,nrows,ncols>   ret;
    for (uint ii=0; ii<ret.numElems(); ++ii)
        ret[ii] = std::exp(fgRandNormal()*stdev+mean);
    return ret;
}

template<uint dim>
FgMatrixC<double,dim,dim>
fgMatRandOrtho()
{
    FgMatrixC<double,dim,dim>       ret;
    for (uint row=0; row<dim; ++row) {
        FgMatrixC<double,dim,1>     vec = fgMatRandNrm<double,dim,1>();
        for (uint rr=0; rr<row; ++rr) {
            FgMatrixC<double,dim,1> axis = ret.rowVec(rr);
            vec -=  axis * fgDot(vec,axis);
        }
        ret.setSubMat(row,0,fgNormalize(vec));
    }
    return ret;
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
            ret.rc(row,col++) = lhs.rc(row,cc);
        for (uint cc=0; cc<ncols2; ++cc)
            ret.rc(row,col++) = rhs.rc(row,cc);
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
FgMatrixC<T,nrows1+nrows2,ncols>
fgConcatVert(
    const FgMatrixC<T,nrows1,ncols> & upper,
    const FgMatrixC<T,nrows2,ncols> & lower)
{
    FgMatrixC<T,nrows1+nrows2,ncols>    ret;
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
        ret.rc(ii,(ii+axisToBecomeX)%3) = T(1);
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

FG_MATRIXC_ELEMWISE(fgPow2Floor,fgPow2Floor)
FG_MATRIXC_ELEMWISE(fgPow2Ceil,fgPow2Ceil)
FG_MATRIXC_ELEMWISE(fgAbs,std::abs)
FG_MATRIXC_ELEMWISE(fgSquare,fgSqr)
FG_MATRIXC_ELEMWISE(fgLog,std::log)
FG_MATRIXC_ELEMWISE(fgExp,std::exp)
FG_MATRIXC_ELEMWISE(fgSqrt,std::sqrt)

template<class T,uint nrows,uint ncols>
double
fgDot(
    const vector<FgMatrixC<T,nrows,ncols> > & v0,
    const vector<FgMatrixC<T,nrows,ncols> > & v1)
{
    FGASSERT(v0.size() == v1.size());
    double  acc(0);
    for (size_t ii=0; ii<v0.size(); ++ii)
        acc += fgDot(v0[ii],v1[ii]);
    return acc;
}

// Weighted dot product:
template<class T,uint nrows,uint ncols>
double
fgDotWgt(
    const vector<FgMatrixC<T,nrows,ncols> > & v0,
    const vector<FgMatrixC<T,nrows,ncols> > & v1,
    const vector<T> &                         w)    // Weight to apply to each dot product
{
    FGASSERT(v0.size() == v1.size());
    FGASSERT(v0.size() == w.size());
    double  acc(0);
    for (size_t ii=0; ii<v0.size(); ++ii)
        acc += fgDot(v0[ii],v1[ii]) * w[ii];
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

// Provide an ordering by axis order
// (without making it the default operator as this may not be desired):
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
    FgMatrixC<T,sz,sz>      ret(0);
    for (uint ii=0; ii<sz; ++ii)
        ret.rc(ii,ii) = vec[ii];
    return ret;
}

template<class T>
FgMatrixC<T,2,2>
fgDiagonal(T v0,T v1)
{
    FgMatrixC<T,2,2>    ret(0);
    ret[0] = v0;
    ret[3] = v1;
    return ret;
}

template<class T>
FgMatrixC<T,3,3>
fgDiagonal(T v0,T v1,T v2)
{
    FgMatrixC<T,3,3>    ret(0);
    ret[0] = v0;
    ret[4] = v1;
    ret[8] = v2;
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

template<class T,uint nrows,uint ncols>
vector<T>
fgMapMag(const vector<FgMatrixC<T,nrows,ncols> > & v)
{
    vector<T>   ret(v.size());
    for (size_t ii=0; ii<v.size(); ++ii)
        ret[ii] = fgMag(v[ii]);
    return ret;
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

// Contract columns to row vector:
template<class T,uint nrows,uint ncols>
FgMatrixC<T,1,ncols>
fgSumCols(const FgMatrixC<T,nrows,ncols> & m)
{
    FgMatrixC<T,1,ncols>    r(0);
    for (uint rr=0; rr<nrows; ++rr)
        for (uint cc=0; cc<ncols; ++cc)
            r[cc] += m.rc(rr,cc);
    return r;
}

// Contract rows to column vector:
template<class T,uint nrows,uint ncols>
FgMatrixC<T,nrows,1>
fgSumRows(const FgMatrixC<T,nrows,ncols> & m)
{
    FgMatrixC<T,nrows,1>    r(0);
    for (uint rr=0; rr<nrows; ++rr)
        for (uint cc=0; cc<ncols; ++cc)
            r[rr] += m.rc(rr,cc);
    return r;
}

template<class T,uint nrows,uint ncols>
FgMatrixC<T,nrows,ncols>
fgMapSqr(FgMatrixC<T,nrows,ncols> m)
{
    FgMatrixC<T,nrows,ncols>    r;
    for (uint ii=0; ii<nrows*ncols; ++ii)
        r[ii] = fgSqr(m[ii]);
    return r;
}

template<class T,uint nrows,uint ncols>
double
fgMag(FgMatrixC<T,nrows,ncols> m)
{return m.mag(); }

// Give a tangent coordinate system for a point on a sphere centred at origin:
FgMat32D
fgTanSphere(FgVect3D p);

template<class T,class U,uint nrows,uint ncols>
void
fgCast_(const FgMatrixC<T,nrows,ncols> & i,FgMatrixC<U,nrows,ncols> & o)
{
    for (size_t ii=0; ii<i.numElems(); ++ii)
        fgCast_(i[ii],o[ii]);
}

// Transpose a matrix stored as an array of arrays. All sub-arrays must have same size:
template<class T,uint nrows,uint ncols>
FgMatrixC<std::vector<T>,nrows,ncols>
fgZip(const std::vector<FgMatrixC<T,nrows,ncols> > & v)
{
    FgMatrixC<std::vector<T>,nrows,ncols>   ret;
    for (uint ii=0; ii<nrows*ncols; ++ii)
        ret[ii].resize(v.size());
    for (size_t jj=0; jj<v.size(); ++jj) {
        for (uint ii=0; ii<nrows*ncols; ++ii)
            ret[ii][jj] = v[jj][ii];
    }
    return ret;
}

template<uint dim>
bool
fgIsValidPermutation(FgMatrixC<uint,dim,1> perm)
{
    FgMatrixC<uint,dim,1>  chk(0);
    for (uint dd=0; dd<dim; ++dd) {
        if (perm[dd] >= dim)
            return false;
        chk[perm[dd]] = 1;
    }
    if (fgMinElem(chk) == 1)
        return true;
    return false;
}

template<class T,uint dim>
FgMatrixC<T,dim,1>
fgPermute(const FgMatrixC<T,dim,1> & v,FgMatrixC<uint,dim,1> perm)  // Assumes a valid permutation, use above to check
{
    FgMatrixC<T,dim,1>      ret;
    for (uint dd=0; dd<dim; ++dd)
        ret[dd] = v[perm[dd]];
    return ret;
}

#endif
