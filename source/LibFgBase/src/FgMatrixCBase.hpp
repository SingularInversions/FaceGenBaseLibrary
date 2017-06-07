//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Feb 22, 2005
//
// Constant size (stack based) matrix / vector
//
//      USE:
//
// A vector is just a matrix with one dimension set to size 1 (thus row and column 
// vectors are different)
//
// rc(row,col) allows access to matrix elements.
//
// operator[] allows access to vector elements (or to unrolled matrix elements).
//
// Templatable by any copyable type, although some member functions require 
// mathematical operators.
//
//      DESIGN:
//
// Due to compiler optimizations such as loop unrolling the code should run pretty much
// as fast as if we'd written the various cases out separately.
//
// Accumulator types were deemed unnecessary for small, constant size matrices.
//

#ifndef FGMATRIXCBASE_HPP
#define FGMATRIXCBASE_HPP

#include "FgStdLibs.hpp"
#include "FgTypes.hpp"
#include "FgStdVector.hpp"
#include "FgDiagnostics.hpp"
#include "FgSerialize.hpp"
#include "FgOut.hpp"

// Declare only due to mutual dependence; FgMatrixC and FgMatrixV can be constructed
// from each other:
template<class T>
struct  FgMatrixV;

template <typename T,uint nrows,uint ncols>
struct  FgMatrixC
{
    T   m[nrows*ncols];
    FG_SERIALIZE1(m)

    typedef T ValType;

    // Auto-initialization of builtins to zero is deprecated, avoid depending on it by using
    // constant-value initialization constructor below:
    FgMatrixC()
    {
        for (uint ii=0; ii<nrows*ncols; ++ii)
            fgInitializeBuiltinsToZero(m[ii]);
    }

    // Constant-value initialization constructor:
    // (unfortunately a pointer-based constructor cannot be added without being ambiguous
    // to the below which is widely used. Use 'fgMatrixC' below instead)
    explicit
    FgMatrixC(T x)
    {setConstant(x); }

    // Value-set constructors; # args must agree with # elements:
    FgMatrixC(T x,T y) {
        FG_STATIC_ASSERT(nrows*ncols == 2);
        m[0] = x; m[1] = y; }
    FgMatrixC(T x,T y,T z) {
        FG_STATIC_ASSERT(nrows*ncols == 3);
        m[0] = x; m[1] = y; m[2] = z; }
    FgMatrixC(T a,T b,T c,T d) {
        FG_STATIC_ASSERT(nrows*ncols == 4);
        m[0]=a; m[1]=b; m[2]=c; m[3]=d; }
    FgMatrixC(T a,T b,T c,T d,T e) {
        FG_STATIC_ASSERT(nrows*ncols == 5);
        m[0]=a; m[1]=b; m[2]=c; m[3]=d; m[4]=e; }
    FgMatrixC(T a,T b,T c,T d,T e,T f) {
        FG_STATIC_ASSERT(nrows*ncols == 6);
        m[0]=a; m[1]=b; m[2]=c; m[3]=d; m[4]=e; m[5]=f; }
    FgMatrixC(T a,T b,T c,T d,T e,T f,T g,T h,T i) {
        FG_STATIC_ASSERT(nrows*ncols == 9);
        m[0]=a; m[1]=b; m[2]=c; m[3]=d; m[4]=e; m[5]=f; m[6]=g; m[7]=h; m[8]=i; }

    // Initializer lists not supported by VS2012:
    //explicit
    //FgMatrixC(std::initializer_list<T> il)
    //{
    //    FGASSERT(nrows*ncols == il.size());
    //    T * dst = &m[0];
    //    for (auto it=il.begin(); it!=il.end(); ++it)
    //        *dst++ = *it;
    //}

    // CC requires explicit definition due to type conversion constructor below:
    FgMatrixC(const FgMatrixC & mat) {
        for (uint ii=0; ii<nrows*ncols; ++ii)
            m[ii] = mat.m[ii];
    }

    // Type conversion constructor. Use 'fgRound' if float->fixed rounding desired:
    template<class U>
    explicit
    FgMatrixC(const FgMatrixC<U,nrows,ncols> & mat) {
        for (uint ii=0; ii<nrows*ncols; ++ii)
            m[ii] = static_cast<T>(mat[ii]);
    }

    explicit
    FgMatrixC(const FgMatrixV<T>& mm) {
        FGASSERT((nrows == mm.numRows()) && (ncols == mm.numCols()));
        for (uint ii=0; ii<nrows*ncols; ++ii)
            m[ii] = mm[ii];
    }

    uint
    numRows() const
    {return nrows; }

    uint
    numCols() const
    {return ncols; }

    uint
    numElems() const
    {return nrows*ncols; }

    size_t
    size() const    // For template interface compatibility with std::vector
    {return nrows*ncols; }

    // Element access by (row,column):
    T &
    rc(uint row,uint col)
    {
        FGASSERT_FAST((row < nrows) && (col < ncols));
        return m[row*ncols+col];
    }
    const T &
    rc(uint row,uint col) const
    {
        FGASSERT_FAST((row < nrows) && (col < ncols));
        return m[row*ncols+col];
    }
    // Element access by (column,row):
    T &
    cr(uint col,uint row)
    {
        FGASSERT_FAST((row < nrows) && (col < ncols));
        return m[row*ncols+col];
    }
    const T &
    cr(uint col,uint row) const
    {
        FGASSERT_FAST((row < nrows) && (col < ncols));
        return m[row*ncols+col];
    }
    T &
    operator[](size_t xx)
    {
        FGASSERT_FAST(xx < nrows*ncols);
        return m[xx];
    }
    const T &
    operator[](size_t xx) const
    {
        FGASSERT_FAST(xx < nrows*ncols);
        return m[xx];
    }
    T &
    operator[](FgMatrixC<uint,2,1> crd)
    {
        FGASSERT_FAST((crd.m[0]<ncols) && (crd.m[1]<nrows));
        return m[crd.m[1]*ncols+crd.m[0]];
    }
    const T &
    operator[](FgMatrixC<uint,2,1> crd) const
    {
        FGASSERT_FAST((crd.m[0]<ncols) && (crd.m[1]<nrows));
        return m[crd.m[1]*ncols+crd.m[0]];
    }
    const T *
    dataPtr() const
    {return m; }

    FgMatrixC
    operator+(const FgMatrixC & rhs) const
    {
        FgMatrixC           ret;
        for (uint ii=0; ii<nrows*ncols; ++ii)
            ret.m[ii] = m[ii] + rhs.m[ii];
        return ret;
    }

    FgMatrixC
    operator-(const FgMatrixC & rhs) const
    {
        FgMatrixC           ret;
        for (uint ii=0; ii<nrows*ncols; ++ii)
            ret.m[ii] = m[ii] - rhs.m[ii];
        return ret;
    }

    FgMatrixC
    operator-() const
    {
        FgMatrixC           ret;
        for (uint ii=0; ii<nrows*ncols; ++ii)
            ret.m[ii] = -m[ii];
        return ret;
    }

    FgMatrixC
    operator*(T val) const
    {
        FgMatrixC           ret;
        for (uint ii=0; ii<nrows*ncols; ++ii)
            ret.m[ii] = m[ii] * val;
        return ret;
    }

    FgMatrixC
    operator/(T val) const
    {
        FgMatrixC           ret;
        for (uint ii=0; ii<nrows*ncols; ++ii)
            ret.m[ii] = m[ii] / val;
        return ret;
    }

    void
    operator*=(T val)
    {for (uint ii=0; ii<nrows*ncols; ++ii) m[ii] *= val; }

    void
    operator/=(T val)
    {for (uint ii=0; ii<nrows*ncols; ++ii) m[ii] /= val; }

    void
    operator+=(const FgMatrixC & rhs)
    {for (uint ii=0; ii<nrows*ncols; ++ii) m[ii] += rhs.m[ii]; }

    void
    operator-=(const FgMatrixC & rhs)
    {for (uint ii=0; ii<nrows*ncols; ++ii) m[ii] -= rhs.m[ii]; }

    bool
    operator==(const FgMatrixC & rhs) const
    {
        for (uint ii=0; ii<nrows*ncols; ++ii)
            if (!(m[ii] == rhs.m[ii]))
                return false;
        return true;
    }

    bool
    operator!=(const FgMatrixC & rhs) const
    {
        for (uint ii=0; ii<nrows*ncols; ++ii)
            if (m[ii] != rhs.m[ii])
                return true;
        return false;
    }

    // Ternary compare to match STL containers. Ordering by axis:
    int
    compare(const FgMatrixC & rhs) const
    {
        for (uint ii=0; ii<nrows*ncols; ++ii) {
            if (m[ii] < rhs.m[ii])
                return -1;
            else if (rhs.m[ii] < m[ii])
                return 1;
        }
        return 0;
    }

    void
    setConstant(T v)
    {
        for (uint ii=0; ii<nrows*ncols; ii++)
            m[ii] = v;
    }

    void
    setZero()
    {setConstant(T(0)); }

    void
    setIdentity()
    {
        setConstant(T(0));
        uint nn = std::min(nrows,ncols);
        for (uint ii=0; ii<nn; ++ii)
            m[ii*ncols+ii] = T(1);
    }

    template<uint srows,uint scols>
    FgMatrixC<T,srows,scols>
    subMatrix(uint firstRow,uint firstCol) const
    {
        FGASSERT_FAST((firstRow+srows <= nrows) && (firstCol+scols <= ncols));
        FgMatrixC<T,srows,scols>    ret;
        uint                        cnt = 0;
        for (uint rr=firstRow; rr<firstRow+srows; ++rr)
            for (uint cc=firstCol; cc<firstCol+scols; ++cc)
                ret[cnt++] = rc(rr,cc);
        return ret;
    }

    template <uint srows,uint scols>
    void
    setSubMat(const FgMatrixC<T,srows,scols> & sub,uint row,uint col)
    {
        FGASSERT((srows+row <= nrows) && (scols+col <= ncols));
        for (uint rr=0; rr<srows; rr++)
            for (uint cc=0; cc<scols; cc++)
                rc(rr+row,cc+col) = sub.rc(rr,cc);
    }

    T
    mag() const         // Squared magnitude
    {
        T   ret = m[0]*m[0];
        for (uint ii=1; ii<nrows*ncols; ++ii)
            ret += m[ii]*m[ii];
        return ret;
    }

    T
    length() const
    {return sqrt(mag()); }

    FgMatrixC<T,ncols,nrows>
    transpose() const
    {
        FgMatrixC<T,ncols,nrows> tMat;
        for (uint ii=0; ii<nrows; ii++)
            for (uint jj=0; jj<ncols; jj++)
                tMat.rc(jj,ii) = rc(ii,jj);
        return tMat;
    }

    FgMatrixC<T,nrows,1>
    colVec(uint col) const
    {
        FgMatrixC<T,nrows,1>    ret;
        FGASSERT_FAST(col < nrows);
        for (uint rr=0; rr<nrows; rr++)
            ret[rr] = rc(rr,col);
        return ret;
    }

    FgMatrixC<T,1,ncols>
    rowVec(uint row) const
    {
        FgMatrixC<T,1,ncols>    ret;
        FGASSERT_FAST(row < nrows);
        for (uint cc=0; cc<ncols; ++cc)
            ret[cc] = rc(row,cc);
        return ret;
    }

    static
    FgMatrixC
    identity()
    {
        FG_STATIC_ASSERT(nrows == ncols);
        FgMatrixC               ret(T(0));
        for (uint ii=0; ii<nrows; ++ii)
            ret.rc(ii,ii) = T(1);
        return ret;
    }

    // Initialize from array data. This is done via a proxy type (accessed via a convenient
    // static member) since compilers interpret '0' as either 'int' or pointer, potentially
    // resulting in either:
    // 1. Ambiguity with single-value constructor (compile-time error)
    // 2. Accidental interpretation of '0' value as a pointer (run-time error)
    struct  FromPtr
    {
        FromPtr(const T * p) : _p(p) {}
        const T * _p;
    };
    explicit
    FgMatrixC(FromPtr p)
    {
        for (uint ii=0; ii<nrows*ncols; ++ii)
            m[ii] = *(p._p++);
    }
    static
    FgMatrixC
    fromPtr(const T * p)
    {return FgMatrixC(FromPtr(p)); }

    // Product of all components:
    T
    volume() const
    {
        T   acc = m[0];
        for (uint ii=1; ii<ncols*nrows; ++ii)
            acc *= m[ii];
        return acc;
    }
};

template<class T,uint nrows,uint ncols>
struct  FgTraits<FgMatrixC<T,nrows,ncols> >
{
    typedef typename FgTraits<T>::Scalar                                Scalar;
    typedef FgMatrixC<typename FgTraits<T>::Accumulator,nrows,ncols>    Accumulator;
    typedef FgMatrixC<typename FgTraits<T>::Floating,nrows,ncols>       Floating;
};

typedef FgMatrixC<float,2,2>        FgMat22F;
typedef FgMatrixC<double,2,2>       FgMat22D;
typedef FgMatrixC<int,2,2>          FgMat22I;
typedef FgMatrixC<uint,2,2>         FgMat22UI;
typedef FgMatrixC<float,3,3>        FgMat33F;
typedef FgMatrixC<double,3,3>       FgMat33D;
typedef FgMatrixC<float,4,4>        FgMat44F;
typedef FgMatrixC<double,4,4>       FgMat44D;

typedef FgMatrixC<float,2,3>        FgMat23F;
typedef FgMatrixC<double,2,3>       FgMat23D;
typedef FgMatrixC<float,3,2>        FgMat32F;
typedef FgMatrixC<double,3,2>       FgMat32D;
typedef FgMatrixC<int,3,2>          FgMat32I;
typedef FgMatrixC<uint,3,2>         FgMat32UI;
typedef FgMatrixC<double,3,4>       FgMat34D;
typedef FgMatrixC<float,4,3>        FgMat43F;

typedef FgMatrixC<float,2,1>        FgVect2F;
typedef FgMatrixC<double,2,1>       FgVect2D;
typedef FgMatrixC<short,2,1>        FgVect2S;
typedef FgMatrixC<ushort,2,1>       FgVect2US;
typedef FgMatrixC<int,2,1>          FgVect2I;
typedef FgMatrixC<uint,2,1>         FgVect2UI;
typedef FgMatrixC<uchar,2,1>        FgVect2UC;
typedef FgMatrixC<bool,2,1>         FgVect2B;
typedef FgMatrixC<float,3,1>        FgVect3F;
typedef FgMatrixC<double,3,1>       FgVect3D;
typedef FgMatrixC<short,3,1>        FgVect3S;
typedef FgMatrixC<int,3,1>          FgVect3I;
typedef FgMatrixC<uint,3,1>         FgVect3UI;
typedef FgMatrixC<int16,3,1>        FgVect3I16;
typedef FgMatrixC<schar,3,1>        FgVect3SC;
typedef FgMatrixC<uchar,3,1>        FgVect3UC;
typedef FgMatrixC<float,4,1>        FgVect4F;
typedef FgMatrixC<double,4,1>       FgVect4D;
typedef FgMatrixC<short,4,1>        FgVect4S;
typedef FgMatrixC<int,4,1>          FgVect4I;
typedef FgMatrixC<uint,4,1>         FgVect4UI;
typedef FgMatrixC<schar,4,1>        FgVect4SC;
typedef FgMatrixC<uchar,4,1>        FgVect4UC;
typedef FgMatrixC<float,5,1>        FgVect5F;
typedef FgMatrixC<double,5,1>       FgVect5D;
typedef FgMatrixC<short,5,1>        FgVect5S;
typedef FgMatrixC<int,5,1>          FgVect5I;
typedef FgMatrixC<uint,5,1>         FgVect5UI;
typedef FgMatrixC<float,6,1>        FgVect6F;
typedef FgMatrixC<double,6,1>       FgVect6D;
typedef FgMatrixC<short,6,1>        FgVect6S;
typedef FgMatrixC<int,6,1>          FgVect6I;
typedef FgMatrixC<uint,6,1>         FgVect6UI;
typedef FgMatrixC<double,9,1>       FgVect9D;

typedef FgMatrixC<float,1,2>        FgVectF2;
typedef FgMatrixC<float,1,3>        FgVectF3;
typedef FgMatrixC<float,1,4>        FgVectF4;
typedef FgMatrixC<double,1,2>       FgVectD2;
typedef FgMatrixC<double,1,3>       FgVectD3;
typedef FgMatrixC<uint,1,2>         FgVectU2;

typedef vector<FgVect2F>            FgVect2Fs;
typedef vector<FgVect2D>            FgVect2Ds;
typedef vector<FgVect3F>            FgVerts;
typedef vector<FgVect3F>            FgVect3Fs;
typedef vector<FgVect3UI>           FgVect3UIs;
typedef vector<FgVect3D>            FgVect3Ds;
typedef vector<FgVect4F>            FgVect4Fs;
typedef vector<FgVect4UI>           FgVect4UIs;
typedef vector<FgVect4D>            FgVect4Ds;

typedef vector<FgVerts>             FgVertss;
typedef vector<FgVect3Ds>           FgVect3Dss;
typedef vector<FgVect3Dss>          FgVect3Dsss;

template<class T,uint nrows,uint ncols>
void
fgReadp(std::istream & is,FgMatrixC<T,nrows,ncols> & m)
{
    for (uint ii=0; ii<nrows*ncols; ++ii)
        fgReadp(is,m[ii]);
}

template<class T,uint nrows,uint ncols>
void
fgWritep(std::ostream & os,const FgMatrixC<T,nrows,ncols> & m)
{
    for (uint ii=0; ii<nrows*ncols; ++ii)
        fgWritep(os,m[ii]);
}

// function 'constructors':

template<typename T,uint nrows,uint ncols>
FgMatrixC<T,nrows,ncols>
fgMatrixC(T * const ptr)
{
    FgMatrixC<T,nrows,ncols>    ret;
    for (size_t ii=0; ii<nrows*ncols; ++ii)
        ret.m[ii] = *ptr++;
    return ret;
}

template<typename T,uint nrows,uint ncols>
FgMatrixC<T,nrows,ncols>
fgMatrixC(const vector<T> & v)
{
    FgMatrixC<T,nrows,ncols>    ret;
    FGASSERT(v.size() == nrows*ncols);
    for (size_t ii=0; ii<v.size(); ++ii)
        ret.m[ii] = v[ii];
    return ret;
}

// Number type conversions are easier with a function for 2 reasons:
// 1. No repetition of nrows and ncols vals in templates.
// 2. Can be used in certain instances where constructor conversions would be interpreted as
//    function delarations.
template<class T,uint nrows,uint ncols>
FgMatrixC<double,nrows,ncols>
fgToDouble(const FgMatrixC<T,nrows,ncols> & m)
{return FgMatrixC<double,nrows,ncols>(m); }

template<class T,uint nrows,uint ncols>
FgMatrixC<float,nrows,ncols>
fgToFloat(const FgMatrixC<T,nrows,ncols> & m)
{return FgMatrixC<float,nrows,ncols>(m); }

template<class T,uint nrows,uint ncols>
FgMatrixC<int,nrows,ncols>
fgToInt(const FgMatrixC<T,nrows,ncols> & m)
{return FgMatrixC<int,nrows,ncols>(m); }

template<class T,uint nrows,uint ncols>
vector<FgMatrixC<double,nrows,ncols> >
fgToDouble(const vector<FgMatrixC<T,nrows,ncols> > & v)
{
    vector<FgMatrixC<double,nrows,ncols> >  ret;
    ret.reserve(v.size());
    for (size_t ii=0; ii<v.size(); ++ii)
        ret.push_back(FgMatrixC<double,nrows,ncols>(v[ii]));
    return ret;
}

template<class T,uint nrows,uint ncols>
vector<FgMatrixC<float,nrows,ncols> >
fgToFloat(const vector<FgMatrixC<T,nrows,ncols> > & v)
{
    vector<FgMatrixC<float,nrows,ncols> >  ret;
    ret.reserve(v.size());
    for (size_t ii=0; ii<v.size(); ++ii)
        ret.push_back(FgMatrixC<float,nrows,ncols>(v[ii]));
    return ret;
}

#endif
