//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Feb 21, 2005
//
// Variable-size (heap based) matrix / vector

#ifndef FGMATRIXV_HPP
#define FGMATRIXV_HPP

#include "FgStdLibs.hpp"
#include "FgStdVector.hpp"
#include "FgTypes.hpp"
#include "FgDiagnostics.hpp"
#include "FgMatrixCBase.hpp"    // Used to represent dimensions 
#include "FgAlgs.hpp"
#include "FgRandom.hpp"

template <class T>
struct  FgMatrixV
{
    uint            nrows;
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
    uint            numElems() const {return (nrows*ncols); }
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

    // Static creation functions:

    static FgMatrixV identity(size_t dim)
    {
        FgMatrixV       ret(dim,dim,T(0));
        for (size_t ii=0; ii<dim; ++ii)
            ret.rc(ii,ii) = T(1);
        return ret;
    }

    static FgMatrixV randNormal(size_t nrows,size_t ncols,T mean=T(0),T stdev=T(1))
    {return FgMatrixV<T>(nrows,ncols,fgRandNormals(nrows*ncols,mean,stdev)); }

    static FgMatrixV randOrthogonal(size_t dim)
    {
        FGASSERT(dim > 1);
        FgMatrixV        ret(dim,dim);
        for (uint row=0; row<dim; ++row) {
            FgMatrixV    vec = FgMatrixV::randNormal(1,dim);
            for (uint rr=0; rr<row; ++rr) {
                FgMatrixV    axis = ret.rowVec(rr);
                vec -=  axis * fgDot(vec,axis);
            }
            ret.setSubMat(row,0,fgNormalize(vec));
        }
        return ret;
    }
};

typedef FgMatrixV<short>        FgMatrixS;
typedef FgMatrixV<int>          FgMatrixI;
typedef FgMatrixV<uint>         FgMatrixUI;
typedef FgMatrixV<float>        FgMatrixF;
typedef FgMatrixV<double>       FgMatrixD;
typedef vector<FgMatrixD>       FgMatrixDs;

template <class T>
std::ostream& operator<<(std::ostream& ss,const FgMatrixV<T> & mm)
{
    FGASSERT(mm.numRows()*mm.numCols()>0);
    bool        vector((mm.numRows() == 1) || mm.numCols() == 1);
    std::ios::fmtflags
        oldFlag = ss.setf(
            std::ios::fixed |
            std::ios::showpos |
            std::ios::right);
    std::streamsize oldPrec = ss.precision(6);
    if (vector) {
        ss << "[" << mm[0];
        for (uint ii=1; ii<mm.numElems(); ii++)
            ss << "," << mm[ii];
        ss << "]";
        if (mm.numRows() > 1) ss << "^T";       // Indicate transpose of column vector
    }
    else {
        ss << fgpush;
        for (uint row=0; row<mm.numRows(); row++) {
            ss << fgnl;
            ss << "[ ";
            for (uint col=0; col<mm.numCols(); col++)
                ss << mm.rc(row,col) << "  ";
            ss << "]";
        }
        ss << fgpop;
    }
    ss.flags(oldFlag);
    ss.precision(oldPrec);
    return ss;
}

template<class T>
FgMatrixV<T>
operator*(const FgMatrixV<T> & lhs,const FgMatrixV<T> & rhs)
{
    // Block sub-loop cache optimization - no multithreading or explicit SIMD.
    // Use eigen instead if speed is important:
    const size_t        CN = 64 / sizeof(T);    // Number of elements that fit in L1 Cache (est)
    FgMatrixV<T>        mat(lhs.nrows,rhs.ncols,T(0));
    FGASSERT(lhs.ncols == rhs.nrows);
    for (size_t rr=0; rr<mat.nrows; rr+=CN) {
        size_t          R2 = fgMin(CN,mat.nrows-rr);
        for (size_t cc=0; cc<mat.ncols; cc+=CN) {
            size_t          C2 = fgMin(CN,mat.ncols-cc);
            if (C2 < CN) {                          // Keep paths separate so inner loop can be unrolled below
                for (size_t kk=0; kk<lhs.ncols; kk+=CN) {
                    size_t          K2 = fgMin(CN,lhs.ncols-kk);
                    for (size_t rr2=0; rr2<R2; ++rr2) {
                        size_t          mIdx = (rr+rr2)*mat.ncols + cc;
                        for (size_t kk2=0; kk2<K2; ++kk2) {
                            T               lv = lhs.rc(rr+rr2,kk+kk2);
                            size_t          rIdx = (kk+kk2)*rhs.ncols + cc;
                            for (size_t cc2=0; cc2<C2; ++cc2)
                                mat.m_data[mIdx+cc2] += lv * rhs.m_data[rIdx+cc2];
                        }
                    }
                }
            }
            else {
                for (size_t kk=0; kk<lhs.ncols; kk+=CN) {
                    size_t          K2 = fgMin(CN,lhs.ncols-kk);
                    for (size_t rr2=0; rr2<R2; ++rr2) {
                        size_t          mIdx = (rr+rr2)*mat.ncols + cc;
                        for (size_t kk2=0; kk2<K2; ++kk2) {
                            T               lv = lhs.rc(rr+rr2,kk+kk2);
                            size_t          rIdx = (kk+kk2)*rhs.ncols + cc;
                            for (size_t cc2=0; cc2<CN; ++cc2)   // CN is compile-time const so this loop is unrolled
                                mat.m_data[mIdx+cc2] += lv * rhs.m_data[rIdx+cc2];
                        }
                    }
                }
            }
        }
    }
    return mat;
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

FgMatrixD &
operator/=(FgMatrixD & mat,double div);

// FgMatrixV<> * vector<> treats rhs side as a column vector and returns same:
template<class T>
vector<T>
operator*(const FgMatrixV<T> & lhs,const vector<T> & rhs)
{
    vector<T>       ret(lhs.nrows,T(0));
    FGASSERT(lhs.ncols == rhs.size());
    for (uint rr=0; rr<lhs.nrows; ++rr)
        for (uint cc=0; cc<lhs.ncols; ++cc)
            ret[rr] += lhs.rc(rr,cc) * rhs[cc];
    return ret;
}

// vector<> * FgMatrixV<> treats lhs side as a row vector and returns same:
template<class T>
vector<T>
operator*(const vector<T> & lhs,const FgMatrixV<T> & rhs)
{
    vector<T>       ret(rhs.ncols,T(0));
    FGASSERT(lhs.size() == rhs.nrows);
    for (uint rr=0; rr<rhs.nrows; ++rr)
        for (uint cc=0; cc<rhs.ncols; ++cc)
            ret[cc] += lhs[rr] * rhs.rc(rr,cc);
    return ret;
}

double
fgMatSumElems(const FgMatrixD & mat);

template<class T>
T
fgDot(const FgMatrixV<T> & lhs,const FgMatrixV<T> & rhs)
{return fgDot(lhs.m_data,rhs.m_data); }

// Map 'abs':
template<class T>
FgMatrixV<T>
fgAbs(const FgMatrixV<T> & mat)
{return FgMatrixV<T>(mat.nrows,mat.ncols,fgAbs(mat.m_data)); }

// Matrix join is the inverse of partition:
// 2x2 block matrix join:
template<class T>
FgMatrixV<T>
fgJoin(const FgMatrixV<T> & ul,const FgMatrixV<T> & ur,const FgMatrixV<T> & ll,const FgMatrixV<T> & lr)
{
    FgMatrixV<T>        ret(ul.nrows+ll.nrows,ul.ncols+ur.ncols);
    FGASSERT((ul.ncols == ll.ncols) && (ur.ncols == lr.ncols));
    FGASSERT((ul.nrows == ur.nrows) && (ll.nrows == lr.nrows));
    ret.setSubMat(0,0,ul);
    ret.setSubMat(0,ul.ncols,ur);
    ret.setSubMat(ul.nrows,0,ll);
    ret.setSubMat(ul.nrows,ul.ncols,lr);
    return ret;
}
template <class T>
FgMatrixV<T>
fgJoinHoriz(const std::vector<FgMatrixV<T> > & ms)
{
    FgMatrixV<T>    ret;
    FGASSERT(!ms.empty());
    uint            rows = ms[0].numRows(),
                    cols = 0;
    for (size_t ii=0; ii<ms.size(); ++ii) {
        FGASSERT(ms[ii].nrows == rows);
        cols += ms[ii].numCols();
    }
    ret.resize(rows,cols);
    uint            col = 0;
    for (size_t ii=0; ii<ms.size(); ++ii) {
        ret.setSubMat(0,col,ms[ii]);
        col += ms[ii].numCols();
    }
    return ret;
}
template <class T>
FgMatrixV<T>
fgJoinHoriz(const FgMatrixV<T> & left,const FgMatrixV<T> & right)
{
    FgMatrixV<T>        retval;
    if (left.empty())
        retval = right;
    else if (right.empty())
        retval = left;
    else {
        FGASSERT(left.numRows() == right.numRows());
        uint            numRows = left.numRows(),
                        numCols = left.numCols() + right.numCols();
        retval.resize(numRows,numCols);
        for (uint rr=0; rr<numRows; rr++) {
            uint    col=0;
            for (uint cc=0; cc<left.numCols(); ++cc)
                retval.rc(rr,col++) = left.rc(rr,cc);
            for (uint cc=0; cc<right.numCols(); ++cc)
                retval.rc(rr,col++) = right.rc(rr,cc);
        }
    }
    return retval;
}
template <class T>
FgMatrixV<T>
fgJoinVert(const std::vector<FgMatrixV<T> > & ms)
{
    FgMatrixV<T>    ret;
    FGASSERT(!ms.empty());
    uint            rows = 0,
                    cols = ms[0].ncols;
    for (size_t ii=0; ii<ms.size(); ++ii) {
        FGASSERT(ms[ii].ncols == cols);
        rows += ms[ii].nrows;
    }
    ret.resize(rows,cols);
    uint            row = 0;
    for (size_t ii=0; ii<ms.size(); ++ii) {
        ret.setSubMat(row,0,ms[ii]);
        row += ms[ii].nrows;
    }
    return ret;
}
template <class T>
FgMatrixV<T>
fgJoinVert(const FgMatrixV<T> & upper,const FgMatrixV<T> & lower)
{
    FgMatrixV<T>      ret;
    if (upper.empty())
        ret = lower;
    else if (lower.empty())
        ret = upper;
    else {
        FGASSERT(upper.numCols() == lower.numCols());
        ret.resize(upper.numRows()+lower.numRows(),upper.numCols());
        for (uint ii=0; ii<upper.numElems(); ++ii)
            ret[ii] = upper[ii];
        uint    off = upper.numElems();
        for (uint ii=0; ii<lower.numElems(); ++ii)
            ret[off+ii] = lower[ii];
    }
    return ret;
}
template <class T>
FgMatrixV<T>
fgJoinVert(const FgMatrixV<T> & upper,const FgMatrixV<T> & middle,const FgMatrixV<T> & lower)
{
    FGASSERT(upper.numCols() == middle.numCols());
    FGASSERT(middle.numCols() == lower.numCols());
    FgMatrixV<T>      retval(upper.numRows()+middle.numRows()+lower.numRows(),upper.numCols());
    uint    row=0;
    for (uint rr=0; rr<upper.numRows(); rr++)
        for (uint col=0; col<upper.numCols(); col++)
            retval.rc(row++,col) = upper.rc(rr,col);
    for (uint rr=0; rr<middle.numRows(); rr++)
        for (uint col=0; col<middle.numCols(); col++)
            retval.rc(row++,col) = middle.rc(rr,col);
    for (uint rr=0; rr<lower.numRows(); rr++)
        for (uint col=0; col<lower.numCols(); col++)
            retval.rc(row++,col) = lower.rc(rr,col);
    return retval;
}
// Diagonal block matrix join. Off-diagonal blocks are all set to zero.
// Blocks are not required to be square.
template<class T>
FgMatrixV<T>
fgJoinDiagonal(const std::vector<FgMatrixV<T> > & blocks)
{
    FgMatrixV<T>        ret;
    FgSizes             rows(blocks.size(),0),
                        cols(blocks.size(),0);
    for (size_t ii=0; ii<blocks.size(); ++ii) {
        rows[ii] = blocks[ii].numRows();
        cols[ii] = blocks[ii].numCols();
    }
    size_t              nrows = fgSum(rows),
                        ncols = fgSum(cols),
                        rr = 0,
                        cc = 0;
    ret.resize(nrows,ncols,T(0));
    for (size_t ii=0; ii<blocks.size(); ++ii) {
        ret.setSubMat(rr,cc,blocks[ii]);
        rr += rows[ii];
        cc += cols[ii];
    }
    return ret;
}
template<class T>
FgMatrixV<T>
fgJoinDiagonal(const FgMatrixV<T> & b0,const FgMatrixV<T> & b1)
{
    FgMatrixV<T>        ret;
    size_t              nrows = b0.nrows + b1.nrows,
                        ncols = b0.ncols + b1.ncols;
    ret.resize(nrows,ncols,T(0));
    ret.setSubMat(0,0,b0);
    ret.setSubMat(b0.nrows,b0.ncols,b1);
    return ret;
}
template<class T>
FgMatrixV<T>
fgJoinDiagonal(const FgMatrixV<T> & b0,const FgMatrixV<T> & b1,const FgMatrixV<T> & b2)
{
    FgMatrixV<T>        ret;
    size_t              nrows = b0.nrows + b1.nrows + b2.nrows,
                        ncols = b0.ncols + b1.ncols + b2.ncols;
    ret.resize(nrows,ncols,T(0));
    ret.setSubMat(0,0,b0);
    ret.setSubMat(b0.nrows,b0.ncols,b1);
    ret.setSubMat(b0.nrows+b1.nrows,b0.ncols+b1.ncols,b2);
    return ret;
}

// Partition a square matrix into 4 matrices symmetrically:
FgMatrixC<FgMatrixD,2,2>
fgPartition(const FgMatrixD & m,size_t loSize);

template<class T>
FgMatrixV<T>
fgModulateCols(
    const FgMatrixV<T> &    matrix,
    const FgMatrixV<T> &    modVector)
{
    FGASSERT(matrix.numCols() == modVector.numElems());
    FgMatrixD       ret = matrix;
    for (uint rr=0; rr<matrix.numRows(); ++rr)
        for (uint cc=0; cc<matrix.numCols(); ++cc)
            ret.rc(rr,cc) *= modVector[cc];
    return ret;
}

template<class T>
FgMatrixV<T>
fgDiagonal(size_t dim,const T & val)
{
    FgMatrixV<T>    ret(dim,dim,T(0));
    for (uint ii=0; ii<dim; ++ii)
        ret.rc(ii,ii) = val;
    return ret;
}

template<class T>
FgMatrixV<T>
fgDiagonal(const vector<T> & vec)
{
    uint            dim = uint(vec.size());
    FgMatrixV<T>    ret(dim,dim,T(0));
    for (uint ii=0; ii<dim; ++ii)
        ret.rc(ii,ii) = vec[ii];
    return ret;
}

template<class T>
FgMatrixV<T>
fgDiagonal(const FgMatrixV<T> & vec)
{
    FGASSERT((vec.numRows() == 1) || (vec.numCols() == 1));
    uint            dim = vec.numRows() * vec.numCols();
    FgMatrixV<T>    ret(dim,dim,T(0));
    for (uint ii=0; ii<dim; ++ii)
        ret.rc(ii,ii) = vec[ii];
    return ret;
}

template<class T>
FgMatrixV<T>
fgNormalize(const FgMatrixV<T> & m)
{return m * (1/std::sqrt(m.mag())); }

template<class T>
FgMatrixV<T>
fgOuterProduct(const vector<T> & rowFacs,const vector<T> & colFacs)
{
    FgMatrixV<T>        ret(rowFacs.size(),colFacs.size());
    size_t              cnt = 0;
    for (size_t rr=0; rr<rowFacs.size(); ++rr)
        for (size_t cc=0; cc<colFacs.size(); ++cc)
            ret[cnt++] = rowFacs[rr] * colFacs[cc];
    return ret;
}

FgMatrixD
fgRelDiff(const FgMatrixD & a,const FgMatrixD & b,double minAbs=0.0);

// Dimensional extrapolation of 'fgJaggedVec', returns a square matrix of 
// matrices of outer product dimensions:
template<class T>
FgMatrixV<FgMatrixV<T> >
fgJaggedMat(const FgSizes & dims,const T & initVal)
{
    FgMatrixV<FgMatrixV<T> >    ret(dims.size(),dims.size());
    for (size_t rr=0; rr<ret.nrows; ++rr)
        for (size_t cc=0; cc<ret.ncols; ++cc)
            ret.rc(rr,cc).resize(dims[rr],dims[cc],initVal);
    return ret;
}

// For use with above:
typedef FgMatrixV<FgVect3D>     FgVect3Dz;
typedef FgMatrixV<FgVect3Dz>    FgVect3Dzz;
typedef FgMatrixV<FgMat33D>     FgMat33Dz;
typedef FgMatrixV<FgMat33Dz>    FgMat33Dzz;

// Form a matrix from a vector of vectors representing each row:
template<class T>
FgMatrixV<T>
fgVecVecToMatrix(const vector<vector<T> > & vss)    // vss must be non-empty with all sub-vects of same size
{
    FGASSERT(!vss.empty());
    size_t          numRows = vss.size(),
                    numCols = vss[0].size();
    vector<T>       data = fgFlat(vss);
    FGASSERT(data.size() == numRows*numCols);
    return FgMatrixV<T>(numRows,numCols,data);
}

// Return a vector of vectors for each row in a matrix:
template<class T>
vector<vector<T> >
fgMatrixToVecVec(const FgMatrixV<T> & v)
{
    vector<vector<T> >      ret(v.numRows());
    for (size_t rr=0; rr<ret.size(); ++rr)
        ret[rr] = v.rowData(uint(rr));
    return ret;
}

#endif
