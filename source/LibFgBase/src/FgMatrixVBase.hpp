//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Feb 21, 2005
//
// Variable-size (heap based) matrix / vector
//

#ifndef FGMATRIXVBASE_HPP
#define FGMATRIXVBASE_HPP

#include "FgStdLibs.hpp"
#include "FgStdVector.hpp"
#include "FgTypes.hpp"
#include "FgDiagnostics.hpp"
#include "FgMatrixCBase.hpp"    // Necessary for representing dimensions 

template <class T>
struct  FgMatrixV
{
    uint            nrows;  // We skip the 'm_' to be compatible with FgMatrixC
    uint            ncols;
    vector<T>       m_data;

    FG_SERIALIZE3(nrows,ncols,m_data)

    FgMatrixV()
    : nrows(0),ncols(0)
    {}

    FgMatrixV(size_t numRows,size_t numCols)
    : nrows(uint(numRows)), ncols(uint(numCols)), m_data(numRows*numCols)
    {}

    FgMatrixV(size_t numRows,size_t numCols,T val)
    : nrows(uint(numRows)), ncols(uint(numCols)), m_data(numRows*numCols,val)
    {}

    FgMatrixV(size_t numRows,size_t numCols,const T *ptr)
    : nrows(uint(numRows)), ncols(uint(numCols)), m_data(ptr,ptr+numRows*numCols)
    {}

    FgMatrixV(size_t nr,size_t nc,const vector<T> & v)
        : nrows(uint(nr)), ncols(uint(nc)), m_data(v)
    {FGASSERT(nr*nc == v.size()); }

    explicit
    FgMatrixV(FgVect2UI dims)
    : ncols(dims[0]), nrows(dims[1])
    {m_data.resize(ncols*nrows); }

    template<uint nr,uint nc>
    explicit
    FgMatrixV(const FgMatrixC<T,nr,nc>& mm)
    {
        resize(nr,nc);
        for (uint ii=0; ii<nr*nc; ii++)
            (*this)[ii] = mm[ii];
    }

    void
    clear()
    {m_data.clear(); nrows = ncols = 0; }

    void
    resize(size_t nr,size_t nc)
    {nrows = uint(nr); ncols = uint(nc); m_data.resize(nr*nc); }

    void
    resize(size_t nr,size_t nc,T v)
    {nrows = uint(nr);ncols = uint(nc); m_data.resize(nr*nc,v); }

    void
    resize(FgVect2UI dims)
    {resize(dims[1],dims[0]); }     // NB Order

    FgVect2UI       dims() const {return FgVect2UI(ncols,nrows); }  // NB Order
    uint            numRows() const { return nrows; }
    uint            numCols() const { return ncols; }
    uint            numElems() const {return (nrows*ncols); };
    bool            empty() const {return (numElems() == 0); }

    // elem is deprecated:
    T &
    elem(size_t row,size_t col)
    {
        FGASSERT_FAST((row < nrows) && (col < ncols));
        return m_data[row*ncols+col];
    }
    const T &
    elem(size_t row,size_t col) const
    {
        FGASSERT_FAST((row < nrows) && (col < ncols));
        return m_data[row*ncols+col];
    }

    T &
    elm(size_t col,size_t row)
    {
        FGASSERT_FAST((row < nrows) && (col < ncols));
        return m_data[row*ncols+col];
    }
    const T &
    elm(size_t col,size_t row) const
    {
        FGASSERT_FAST((row < nrows) && (col < ncols));
        return m_data[row*ncols+col];
    }

    T &
    operator[](size_t ii)
    {FGASSERT_FAST(ii<m_data.size()); return m_data[ii]; }

    const T &
    operator[](size_t ii) const
    {FGASSERT_FAST(ii<m_data.size()); return m_data[ii]; }

    T &
    operator[](FgVect2UI coord)
    {return elm(coord[0],coord[1]); }

    const T &
    operator[](FgVect2UI coord) const
    {return elm(coord[0],coord[1]); }

    const T *
    dataPtr() const
    {FGASSERT_FAST(!m_data.empty()); return &m_data[0]; }

    const vector<T> &
    dataVec() const
    {return m_data; }

    // Operators

    bool
    operator==(const FgMatrixV & rhs) const
    {return ((nrows==rhs.nrows) && (ncols==rhs.ncols) && (m_data==rhs.m_data)); }

    bool
    operator!=(const FgMatrixV & rhs) const
    {return !(operator==(rhs)); }

    FgMatrixV
    operator*(const FgMatrixV & m) const
    {
        FgMatrixV<T> newMat(nrows,m.ncols);
        FGASSERT(ncols == m.nrows);
        for (uint ii=0; ii<nrows; ii++) {
            for (uint jj=0; jj<m.ncols; jj++) {
                newMat.elem(ii,jj) = 0.0;
                for (uint kk=0; kk<ncols; kk++)
                    newMat.elem(ii,jj) += this->elem(ii,kk) * m.elem(kk,jj);
            }
        }
        return newMat;
    }

    FgMatrixV
    operator*(T v) const
    {
        FgMatrixV<T> newMat(nrows,ncols);
        for (uint ii=0; ii<m_data.size(); ii++)
            newMat.m_data[ii] = m_data[ii] * v;
        return newMat;
    }

    FgMatrixV
    operator+(const FgMatrixV & rhs) const
    {
        FgMatrixV<T>    ret(nrows,ncols);
        FGASSERT((nrows == rhs.nrows) && (ncols == rhs.ncols));
        for (uint ii=0; ii<m_data.size(); ii++)
            ret.m_data[ii] = m_data[ii] + rhs.m_data[ii];
        return ret;
    }

    FgMatrixV
    operator-(const FgMatrixV & rhs) const
    {
        FgMatrixV<T>    ret(nrows,ncols);
        FGASSERT((nrows == rhs.nrows) && (ncols == rhs.ncols));
        for (uint ii=0; ii<m_data.size(); ii++)
            ret.m_data[ii] = m_data[ii] - rhs.m_data[ii];
        return ret;
    }

    const FgMatrixV &
    operator*=(T v)
    {
        for (uint ii=0; ii<m_data.size(); ii++)
            m_data[ii] *= v;
        return *this;
    }

    const FgMatrixV &
    operator+=(const FgMatrixV & rhs)
    {
        FGASSERT((nrows == rhs.nrows) && (ncols == rhs.ncols));
        for (uint ii=0; ii<m_data.size(); ii++)
            m_data[ii] += rhs.m_data[ii];
        return *this;
    }

    const FgMatrixV &
    operator-=(const FgMatrixV & rhs)
    {
        FGASSERT((nrows == rhs.nrows) && (ncols == rhs.ncols));
        for (uint ii=0; ii<m_data.size(); ii++)
            m_data[ii] -= rhs.m_data[ii];
        return *this;
    }

    void
    setSubMat(size_t row,size_t col,const FgMatrixV & m)
    {
        FGASSERT((m.nrows+row <= nrows) && (m.ncols+col <= ncols));
        for (uint rr=0; rr<m.nrows; ++rr)
            for (uint cc=0; cc<m.ncols; ++cc)
                elem(row+rr,col+cc) = m.elem(rr,cc);
    }

    // Set submatrix from the transpose of the given matrix (saves an allocation and a copy):
    void
    setSubMatTr(size_t row,size_t col,const FgMatrixV & m)
    {
        FGASSERT((m.ncols+row <= nrows) && (m.nrows+col <= ncols));
        for (uint rr=0; rr<m.ncols; ++rr)
            for (uint cc=0; cc<m.nrows; ++cc)
                elem(row+rr,col+cc) = m.elem(cc,rr);
    }

    // Set submatrix and it's tranpose mirror submatrix from the given:
    void
    setSubMatMirror(size_t row,size_t col,const FgMatrixV & m)
    {
        // Ensure the submatrix doesn't intersect its mirror:
        FGASSERT((col >= (row + m.nrows)) || (row >= (col + m.ncols)));
        setSubMat(row,col,m);
        setSubMatTr(col,row,m);
    }

    // Accumulate in sub-matrix:
    void
    accSubMat(size_t row,size_t col,const FgMatrixV & m)
    {
        FGASSERT((m.nrows+row <= nrows) && (m.ncols+col <= ncols));
        for (uint rr=0; rr<m.nrows; ++rr)
            for (uint cc=0; cc<m.ncols; ++cc)
                elem(row+rr,col+cc) += m.elem(rr,cc);
    }
    template<uint mrows,uint mcols>
    void
    accSubMat(size_t row,size_t col,FgMatrixC<T,mrows,mcols> m)
    {
        FGASSERT((mrows+row <= nrows) && (mcols+col <= ncols));
        for (uint rr=0; rr<mrows; ++rr)
            for (uint cc=0; cc<mcols; ++cc)
                elem(row+rr,col+cc) += m.elem(rr,cc);
    }

    // Other
    void            setConst(T val);
    void            setZero() {setConst(T(0)); };
    void            setIdentity();
    FgMatrixV      transpose() const;
    T               length() const {return sqrt(mag());}
    T               mag() const;
    T               mean() const {return fgMean(m_data); };
    bool            isVector() const {return ((nrows*ncols>0) && ((nrows==1)||(ncols==1))); }
    FgMatrixV      subMatrix(uint firstRow,uint firstCol,uint numRows,uint numCols) const;
    FgMatrixV      colVec(uint n) const {return subMatrix(0,n,nrows,1); };
    FgMatrixV      rowVector(uint n) const {return subMatrix(n,0,1,ncols); };
    FgMatrixV      diagMatrix() const;
    FgMatrixV      colAdd(const FgMatrixV &colVec) const;
    FgMatrixV      colSubtract(const FgMatrixV &colVec) const;

    typedef T ValType;
};

typedef FgMatrixV<short>    FgMatrixS;
typedef FgMatrixV<int>      FgMatrixI;
typedef FgMatrixV<uint>     FgMatrixUI;
typedef FgMatrixV<float>    FgMatrixF;
typedef FgMatrixV<double>   FgMatrixD;

template <class T>
void    FgMatrixV<T>::setConst(T v)
{
    for (uint ii=0; ii<m_data.size(); ii++)
        m_data[ii] = v;
}

template <class T>
void FgMatrixV<T>::setIdentity()
{
    setZero();
    uint nn = (nrows < ncols) ? nrows : ncols;
    for (uint ii=0; ii<nn; ii++)
        this->elem(ii,ii) = T(1);
}

template <class T>
FgMatrixV<T> FgMatrixV<T>::transpose() const
{
    FgMatrixV<T> tMat(ncols,nrows);
    for (uint ii=0; ii<nrows; ii++)
        for (uint jj=0; jj<ncols; jj++)
            tMat.elem(jj,ii) = elem(ii,jj);
    return tMat;
}

template <class T>
T   FgMatrixV<T>::mag() const
{
    typename FgTraits<T>::Accumulator    tot = 0;
    for (uint ii=0; ii<m_data.size(); ii++)
        tot += m_data[ii] * m_data[ii];
    return T(tot);
}

template <class T>
FgMatrixV<T> FgMatrixV<T>::subMatrix(uint firstRow,uint firstCol,
                                         uint numRows_,uint numCols_) const
{
    FGASSERT(((firstCol + numCols_) <= ncols) && 
             ((firstRow + numRows_) <= nrows));
    FgMatrixV<T> retval(numRows_,numCols_);
    for (uint ii=0; ii<numRows_; ii++)
        for (uint jj=0; jj<numCols_; jj++)
            retval.elem(ii,jj) = this->elem(ii+firstRow,jj+firstCol);
    return retval;
}

    // Creates a diagonal matrix of the same dimension as a vector, using its
    // values as the diagonal elements.
template<class T>
FgMatrixV<T>  FgMatrixV<T>::diagMatrix() const
{
    FGASSERT(nrows*ncols > 0);
    FGASSERT((nrows == 1) || (ncols == 1));
    FgMatrixV<T>  retval(nrows*ncols,nrows*ncols,0);
    for (uint ii=0; ii<nrows*ncols; ii++)
        retval.elem(ii,ii) = m_data[ii];
    return retval;
}

template<class T>
FgMatrixV<T>   FgMatrixV<T>::colAdd(const FgMatrixV &colVec) const
{
    FGASSERT(colVec.numCols() == 1);
    FGASSERT(colVec.numRows() == numRows());
    FgMatrixV<T>   ret(numRows(),numCols());
    for (uint row=0; row<numRows(); row++)
        for (uint col=0; col<numCols(); col++)
            ret.elem(row,col) = elem(row,col) + colVec[row];
    return ret;
}

template<class T>
FgMatrixV<T>   FgMatrixV<T>::colSubtract(const FgMatrixV &colVec) const
{
    FGASSERT(colVec.numCols() == 1);
    FGASSERT(colVec.numRows() == numRows());
    FgMatrixV<T>   ret(numRows(),numCols());
    for (uint row=0; row<numRows(); row++)
        for (uint col=0; col<numCols(); col++)
            ret.elem(row,col) = elem(row,col) - colVec[row];
    return ret;
}

template<class T>
FgMatrixV<T>
operator*(const T & lhs,const FgMatrixV<T> & rhs)
{return (rhs*lhs); }

template<typename T>
FgMatrixV<T>
fgColVec(const vector<T> & v)
{return FgMatrixV<T>(uint(v.size()),1,v.data()); }

template<typename T>
FgMatrixV<T>
fgRowVec(const vector<T> & v)
{
    FGASSERT(!v.empty());
    return FgMatrixV<T>(1,uint(v.size()),&v[0]);
}

#endif
