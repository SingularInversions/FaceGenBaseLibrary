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
#include "FgMatrixCBase.hpp"    // Used to represent dimensions 

template <class T>
struct  FgMatrixV
{
    uint            nrows;  // We skip the 'm_' to be compatible with FgMatrixC
    uint            ncols;
    vector<T>       m_data;

    FG_SERIALIZE3(nrows,ncols,m_data)

    FgMatrixV() : nrows(0),ncols(0) {}

    FgMatrixV(size_t numRows,size_t numCols)
    : nrows(uint(numRows)), ncols(uint(numCols)), m_data(numRows*numCols)
    {}

    FgMatrixV(size_t numRows,size_t numCols,T val)
    : nrows(uint(numRows)), ncols(uint(numCols)), m_data(numRows*numCols,val)
    {}

    FgMatrixV(size_t numRows,size_t numCols,const T *ptr)
    : nrows(uint(numRows)), ncols(uint(numCols)), m_data(ptr,ptr+numRows*numCols)
    {}

    FgMatrixV(size_t nr,size_t nc,const vector<T> & v) : nrows(uint(nr)), ncols(uint(nc)), m_data(v)
    {FGASSERT(nr*nc == v.size()); }

    explicit
    FgMatrixV(FgVect2UI cols_rows) : nrows(cols_rows[1]), ncols(cols_rows[0])
    {m_data.resize(ncols*nrows); }

    template<uint nr,uint nc>
    explicit
    FgMatrixV(const FgMatrixC<T,nr,nc>& mm)
    {
        resize(nr,nc);
        for (uint ii=0; ii<nr*nc; ii++)
            (*this)[ii] = mm[ii];
    }

    // initializer_list is just a couple of pointers so pass-by-value is faster:
    FgMatrixV(uint rows,uint cols,std::initializer_list<T> l) : nrows(rows), ncols(cols), m_data(l)
    {FGASSERT(nrows*ncols == l.size()); }

    FgMatrixV(FgVect2UI dims,std::initializer_list<T> l) : nrows(dims[1]), ncols(dims[0]), m_data(l)
    {FGASSERT(nrows*ncols == l.size()); }

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
    resize(FgVect2UI cols_rows)
    {resize(cols_rows[1],cols_rows[0]); }

    FgVect2UI       dims() const {return FgVect2UI(ncols,nrows); }  // NB Order
    uint            numRows() const { return nrows; }
    uint            numCols() const { return ncols; }
    uint            numElems() const {return (nrows*ncols); };
    bool            empty() const {return (numElems() == 0); }

    // Element access by (row,column):
    T &
    rc(size_t row,size_t col)
    {
        FGASSERT_FAST((row < nrows) && (col < ncols));
        return m_data[row*ncols+col];
    }
    const T &
    rc(size_t row,size_t col) const
    {
        FGASSERT_FAST((row < nrows) && (col < ncols));
        return m_data[row*ncols+col];
    }
    // Element access by (column,row):
    T &
    cr(size_t col,size_t row)
    {
        FGASSERT_FAST((row < nrows) && (col < ncols));
        return m_data[row*ncols+col];
    }
    const T &
    cr(size_t col,size_t row) const
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
    {return cr(coord[0],coord[1]); }

    const T &
    operator[](FgVect2UI coord) const
    {return cr(coord[0],coord[1]); }

    const T *
    dataPtr() const
    {FGASSERT(!m_data.empty()); return &m_data[0]; }

    const T *
    rowPtr(size_t row) const
    {
        FGASSERT(!m_data.empty());
        FGASSERT(row < nrows);
        return &m_data[row*ncols];
    }

    const vector<T> &
    dataVec() const
    {return m_data; }

    vector<T>
    rowData(uint row) const
    {
        const T *       rPtr = rowPtr(row);
        return vector<T>(rPtr,rPtr+ncols);
    }

    vector<T>
    colData(uint row) const
    {
        FGASSERT(row < nrows);
        vector<T>       ret;
        ret.reserve(nrows);
        for (uint cc=0; cc<ncols; ++cc)
            ret.push_back(rc(row,cc));
        return ret;
    }

    void
    addRow(const vector<T> & data)
    {
        FGASSERT(data.size() == ncols);
        fgCat_(m_data,data);
        ++nrows;
    }

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
                newMat.rc(ii,jj) = 0.0;
                for (uint kk=0; kk<ncols; kk++)
                    newMat.rc(ii,jj) += this->rc(ii,kk) * m.rc(kk,jj);
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
                rc(row+rr,col+cc) = m.rc(rr,cc);
    }

    // Set submatrix from the transpose of the given matrix (saves an allocation and a copy):
    void
    setSubMatTr(size_t row,size_t col,const FgMatrixV & m)
    {
        FGASSERT((m.ncols+row <= nrows) && (m.nrows+col <= ncols));
        for (uint rr=0; rr<m.ncols; ++rr)
            for (uint cc=0; cc<m.nrows; ++cc)
                rc(row+rr,col+cc) = m.rc(cc,rr);
    }

    // Accumulate in sub-matrix:
    void
    accSubMat(size_t row,size_t col,const FgMatrixV & m)
    {
        FGASSERT((m.nrows+row <= nrows) && (m.ncols+col <= ncols));
        for (uint rr=0; rr<m.nrows; ++rr)
            for (uint cc=0; cc<m.ncols; ++cc)
                rc(row+rr,col+cc) += m.rc(rr,cc);
    }
    template<uint mrows,uint mcols>
    void
    accSubMat(size_t row,size_t col,FgMatrixC<T,mrows,mcols> m)
    {
        FGASSERT((mrows+row <= nrows) && (mcols+col <= ncols));
        for (uint rr=0; rr<mrows; ++rr)
            for (uint cc=0; cc<mcols; ++cc)
                rc(row+rr,col+cc) += m.rc(rr,cc);
    }

    void            setConst(T v)
    {
        for (uint ii=0; ii<m_data.size(); ii++)
            m_data[ii] = v;
    }

    void
    setZero()
    {setConst(T(0)); }

    void
    setIdentity()
    {
        setZero();
        uint nn = (nrows < ncols) ? nrows : ncols;
        for (uint ii=0; ii<nn; ii++)
            this->rc(ii,ii) = T(1);
    }

    FgMatrixV
    transpose() const
    {
        FgMatrixV<T> tMat(ncols,nrows);
        for (uint ii=0; ii<nrows; ii++)
            for (uint jj=0; jj<ncols; jj++)
                tMat.rc(jj,ii) = rc(ii,jj);
        return tMat;
    }

    double
    mag() const                     // Squared magnitude
    {return fgMag(m_data); }

    double
    length() const                  // Euclidean length
    {return fgLength(m_data); }

    T
    mean() const
    {return fgMean(m_data); }

    bool
    isVector() const
    {return ((nrows*ncols>0) && ((nrows==1)||(ncols==1))); }

    FgMatrixV
    subMatrix(size_t firstRow,size_t firstCol,size_t numRows_,size_t numCols_) const
    {
        FGASSERT(((firstCol + numCols_) <= ncols) && 
                 ((firstRow + numRows_) <= nrows));
        FgMatrixV<T> retval(numRows_,numCols_);
        for (size_t ii=0; ii<numRows_; ii++)
            for (size_t jj=0; jj<numCols_; jj++)
                retval.rc(ii,jj) = this->rc(ii+firstRow,jj+firstCol);
        return retval;
    }

    FgMatrixV
    colVec(uint n) const
    {return subMatrix(0,n,nrows,1); }

    FgMatrixV
    rowVec(uint n) const
    {return subMatrix(n,0,1,ncols); }

    typedef T ValType;
};

typedef FgMatrixV<short>        FgMatrixS;
typedef FgMatrixV<int>          FgMatrixI;
typedef FgMatrixV<uint>         FgMatrixUI;
typedef FgMatrixV<float>        FgMatrixF;
typedef FgMatrixV<double>       FgMatrixD;
typedef vector<FgMatrixD>       FgMatrixDs;

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
