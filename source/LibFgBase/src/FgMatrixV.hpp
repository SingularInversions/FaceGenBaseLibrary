//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Variable-size (heap based) matrix / vector

#ifndef FGMATRIXV_HPP
#define FGMATRIXV_HPP

#include "FgStdLibs.hpp"
#include "FgStdVector.hpp"
#include "FgTypes.hpp"
#include "FgDiagnostics.hpp"
#include "FgMatrixCBase.hpp"    // Used to represent dimensions 
#include "FgRandom.hpp"

namespace Fg {

template <class T>
struct  MatV
{
    uint            nrows;
    uint            ncols;
    Svec<T>         m_data;

    FG_SERIALIZE3(nrows,ncols,m_data)

    MatV() : nrows(0),ncols(0) {}

    MatV(size_t numRows,size_t numCols)
    : nrows(uint(numRows)), ncols(uint(numCols)), m_data(numRows*numCols)
    {}

    MatV(size_t numRows,size_t numCols,T val)
    : nrows(uint(numRows)), ncols(uint(numCols)), m_data(numRows*numCols,val)
    {}

    MatV(size_t numRows,size_t numCols,const T *ptr)
    : nrows(uint(numRows)), ncols(uint(numCols)), m_data(ptr,ptr+numRows*numCols)
    {}

    MatV(size_t nr,size_t nc,const Svec<T> & v) : nrows(uint(nr)), ncols(uint(nc)), m_data(v)
    {FGASSERT(nr*nc == v.size()); }

    explicit
    MatV(Vec2UI cols_rows) : nrows(cols_rows[1]), ncols(cols_rows[0])
    {m_data.resize(ncols*nrows); }

    template<uint nr,uint nc>
    explicit
    MatV(const Mat<T,nr,nc>& mm)
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
    resize(Vec2UI cols_rows)
    {resize(cols_rows[1],cols_rows[0]); }

    Vec2UI       dims() const {return Vec2UI(ncols,nrows); }  // NB Order
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
    operator[](Vec2UI coord)
    {return cr(coord[0],coord[1]); }

    const T &
    operator[](Vec2UI coord) const
    {return cr(coord[0],coord[1]); }

    const T *
    data() const
    {FGASSERT(!m_data.empty()); return &m_data[0]; }

    const T *
    rowPtr(size_t row) const
    {
        FGASSERT(!m_data.empty());
        FGASSERT(row < nrows);
        return &m_data[row*ncols];
    }

    const Svec<T> &
    dataVec() const
    {return m_data; }

    Svec<T>
    rowData(uint row) const
    {
        const T *       rPtr = rowPtr(row);
        return Svec<T>(rPtr,rPtr+ncols);
    }

    Svec<T>
    colData(uint row) const
    {
        FGASSERT(row < nrows);
        Svec<T>       ret;
        ret.reserve(nrows);
        for (uint cc=0; cc<ncols; ++cc)
            ret.push_back(rc(row,cc));
        return ret;
    }

    void
    addRow(const Svec<T> & data)
    {
        FGASSERT(data.size() == ncols);
        cat_(m_data,data);
        ++nrows;
    }

    // Operators

    bool
    operator==(const MatV & rhs) const
    {return ((nrows==rhs.nrows) && (ncols==rhs.ncols) && (m_data==rhs.m_data)); }

    bool
    operator!=(const MatV & rhs) const
    {return !(operator==(rhs)); }

    MatV
    operator*(T v) const
    {
        MatV<T> newMat(nrows,ncols);
        for (uint ii=0; ii<m_data.size(); ii++)
            newMat.m_data[ii] = m_data[ii] * v;
        return newMat;
    }

    MatV
    operator+(const MatV & rhs) const
    {
        MatV<T>    ret(nrows,ncols);
        FGASSERT((nrows == rhs.nrows) && (ncols == rhs.ncols));
        for (uint ii=0; ii<m_data.size(); ii++)
            ret.m_data[ii] = m_data[ii] + rhs.m_data[ii];
        return ret;
    }

    MatV
    operator-(const MatV & rhs) const
    {
        MatV<T>    ret(nrows,ncols);
        FGASSERT((nrows == rhs.nrows) && (ncols == rhs.ncols));
        for (uint ii=0; ii<m_data.size(); ii++)
            ret.m_data[ii] = m_data[ii] - rhs.m_data[ii];
        return ret;
    }

    const MatV &
    operator*=(T v)
    {
        for (uint ii=0; ii<m_data.size(); ii++)
            m_data[ii] *= v;
        return *this;
    }

    const MatV &
    operator+=(const MatV & rhs)
    {
        FGASSERT((nrows == rhs.nrows) && (ncols == rhs.ncols));
        for (uint ii=0; ii<m_data.size(); ii++)
            m_data[ii] += rhs.m_data[ii];
        return *this;
    }

    const MatV &
    operator-=(const MatV & rhs)
    {
        FGASSERT((nrows == rhs.nrows) && (ncols == rhs.ncols));
        for (uint ii=0; ii<m_data.size(); ii++)
            m_data[ii] -= rhs.m_data[ii];
        return *this;
    }

    void
    setSubMat(size_t row,size_t col,const MatV & m)
    {
        FGASSERT((m.nrows+row <= nrows) && (m.ncols+col <= ncols));
        for (uint rr=0; rr<m.nrows; ++rr)
            for (uint cc=0; cc<m.ncols; ++cc)
                rc(row+rr,col+cc) = m.rc(rr,cc);
    }

    // Set submatrix from the transpose of the given matrix (saves an allocation and a copy):
    void
    setSubMatTr(size_t row,size_t col,const MatV & m)
    {
        FGASSERT((m.ncols+row <= nrows) && (m.nrows+col <= ncols));
        for (uint rr=0; rr<m.ncols; ++rr)
            for (uint cc=0; cc<m.nrows; ++cc)
                rc(row+rr,col+cc) = m.rc(cc,rr);
    }

    // Accumulate in sub-matrix:
    void
    accSubMat(size_t row,size_t col,const MatV & m)
    {
        FGASSERT((m.nrows+row <= nrows) && (m.ncols+col <= ncols));
        for (uint rr=0; rr<m.nrows; ++rr)
            for (uint cc=0; cc<m.ncols; ++cc)
                rc(row+rr,col+cc) += m.rc(rr,cc);
    }
    template<uint mrows,uint mcols>
    void
    accSubMat(size_t row,size_t col,Mat<T,mrows,mcols> m)
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

    MatV
    transpose() const
    {
        MatV<T> tMat(ncols,nrows);
        for (uint ii=0; ii<nrows; ii++)
            for (uint jj=0; jj<ncols; jj++)
                tMat.rc(jj,ii) = rc(ii,jj);
        return tMat;
    }

    double
    mag() const                     // Squared magnitude
    {return cMag(m_data); }

    double
    len() const                  // Euclidean length
    {return cLen(m_data); }

    T
    mean() const
    {return cMean(m_data); }

    bool
    isVector() const
    {return ((nrows*ncols>0) && ((nrows==1)||(ncols==1))); }

    MatV
    subMatrix(size_t firstRow,size_t firstCol,size_t numRows_,size_t numCols_) const
    {
        FGASSERT(((firstCol + numCols_) <= ncols) && 
                 ((firstRow + numRows_) <= nrows));
        MatV<T> retval(numRows_,numCols_);
        for (size_t ii=0; ii<numRows_; ii++)
            for (size_t jj=0; jj<numCols_; jj++)
                retval.rc(ii,jj) = this->rc(ii+firstRow,jj+firstCol);
        return retval;
    }

    MatV
    colVec(uint n) const
    {return subMatrix(0,n,nrows,1); }

    MatV
    rowVec(uint n) const
    {return subMatrix(n,0,1,ncols); }

    // Static creation functions:

    static MatV identity(size_t dim)
    {
        MatV       ret(dim,dim,T(0));
        for (size_t ii=0; ii<dim; ++ii)
            ret.rc(ii,ii) = T(1);
        return ret;
    }

    static MatV randNormal(size_t nrows,size_t ncols,T mean=T(0),T stdev=T(1))
    {return MatV<T>(nrows,ncols,randNormals(nrows*ncols,mean,stdev)); }

    static MatV randOrthogonal(size_t dim)
    {
        FGASSERT(dim > 1);
        MatV        ret(dim,dim);
        for (uint row=0; row<dim; ++row) {
            MatV    vec = MatV::randNormal(1,dim);
            for (uint rr=0; rr<row; ++rr) {
                MatV    axis = ret.rowVec(rr);
                vec -=  axis * cDot(vec,axis);
            }
            ret.setSubMat(row,0,fgNormalize(vec));
        }
        return ret;
    }
};

typedef MatV<float>         MatF;
typedef MatV<double>        MatD;
typedef Svec<MatD>          MatDs;

template <class T>
std::ostream &
operator<<(std::ostream & ss,const MatV<T> & mm)
{
    FGASSERT(mm.numRows()*mm.numCols()>0);
    bool        isVec((mm.numRows() == 1) || mm.numCols() == 1);
    std::ios::fmtflags
        oldFlag = ss.setf(
            std::ios::fixed |
            std::ios::showpos |
            std::ios::right);
    std::streamsize oldPrec = ss.precision(6);
    if (isVec) {
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

// The element type must be static_cast-able from from 0:
template<class T>
MatV<T>
matMul(const MatV<T> & lhs,const MatV<T> & rhs)
{
    FGASSERT(lhs.ncols == rhs.nrows);
    // Block sub-loop cache optimization - no multithreading or explicit SIMD.
    // Use eigen instead if speed is important:
    size_t constexpr    CN = 64 / sizeof(T);    // Number of elements that fit in L1 Cache (est)
    MatV<T>             ret(lhs.nrows,rhs.ncols,static_cast<T>(0));
    for (size_t rr=0; rr<ret.nrows; rr+=CN) {
        size_t const        R2 = cMin(CN,ret.nrows-rr);
        for (size_t cc=0; cc<ret.ncols; cc+=CN) {
            size_t const        C2 = cMin(CN,ret.ncols-cc);
            if (C2 < CN) {                          // Keep paths separate so inner loop can be unrolled below
                for (size_t kk=0; kk<lhs.ncols; kk+=CN) {
                    size_t const    K2 = cMin(CN,lhs.ncols-kk);
                    for (size_t rr2=0; rr2<R2; ++rr2) {
                        size_t const    mIdx = (rr+rr2)*ret.ncols + cc;
                        for (size_t kk2=0; kk2<K2; ++kk2) {
                            T const         lv = lhs.rc(rr+rr2,kk+kk2);
                            size_t const    rIdx = (kk+kk2)*rhs.ncols + cc;
                            for (size_t cc2=0; cc2<C2; ++cc2)
                                ret.m_data[mIdx+cc2] += lv * rhs.m_data[rIdx+cc2];
                        }
                    }
                }
            }
            else {
                for (size_t kk=0; kk<lhs.ncols; kk+=CN) {
                    size_t const    K2 = cMin(CN,lhs.ncols-kk);
                    for (size_t rr2=0; rr2<R2; ++rr2) {
                        size_t const    mIdx = (rr+rr2)*ret.ncols + cc;
                        for (size_t kk2=0; kk2<K2; ++kk2) {
                            T const         lv = lhs.rc(rr+rr2,kk+kk2);
                            size_t const    rIdx = (kk+kk2)*rhs.ncols + cc;
                            for (size_t cc2=0; cc2<CN; ++cc2)   // CN is compile-time const so this loop is unrolled
                                ret.m_data[mIdx+cc2] += lv * rhs.m_data[rIdx+cc2];
                        }
                    }
                }
            }
        }
    }
    return ret;
}

template<class T>
inline MatV<T>
operator*(const MatV<T> & lhs,const MatV<T> & rhs)
{return matMul(lhs,rhs); }

// Specializations for float and double use Eigen library when matrix is large enough to be worth
// copying over:
template<>
MatF
operator*(const MatF & lhs,const MatF & rhs);

template<>
MatD
operator*(const MatD & lhs,const MatD & rhs);

template<class T>
MatV<T>
operator*(const T & lhs,const MatV<T> & rhs)
{return (rhs*lhs); }

template<typename T>
MatV<T>
fgColVec(const Svec<T> & v)
{return MatV<T>(uint(v.size()),1,v.data()); }

template<typename T>
MatV<T>
fgRowVec(const Svec<T> & v)
{
    FGASSERT(!v.empty());
    return MatV<T>(1,uint(v.size()),&v[0]);
}

MatD &
operator/=(MatD & mat,double div);

// MatV<> * Svec<> treats rhs side as a column vector and returns same:
template<class T>
Svec<T>
operator*(const MatV<T> & lhs,const Svec<T> & rhs)
{
    Svec<T>       ret(lhs.nrows,T(0));
    FGASSERT(lhs.ncols == rhs.size());
    for (uint rr=0; rr<lhs.nrows; ++rr)
        for (uint cc=0; cc<lhs.ncols; ++cc)
            ret[rr] += lhs.rc(rr,cc) * rhs[cc];
    return ret;
}

// Svec<> * MatV<> treats lhs side as a row vector and returns same:
template<class T>
Svec<T>
operator*(const Svec<T> & lhs,const MatV<T> & rhs)
{
    Svec<T>       ret(rhs.ncols,T(0));
    FGASSERT(lhs.size() == rhs.nrows);
    for (uint rr=0; rr<rhs.nrows; ++rr)
        for (uint cc=0; cc<rhs.ncols; ++cc)
            ret[cc] += lhs[rr] * rhs.rc(rr,cc);
    return ret;
}

double
fgMatSumElems(const MatD & mat);

template<class T>
T
cDot(const MatV<T> & lhs,const MatV<T> & rhs)
{return cDot(lhs.m_data,rhs.m_data); }

// Map 'abs':
template<class T>
MatV<T>
mapAbs(const MatV<T> & mat)
{return MatV<T>(mat.nrows,mat.ncols,mapAbs(mat.m_data)); }

// Matrix join is the inverse of partition:
// 2x2 block matrix join:
template<class T>
MatV<T>
fgJoin(const MatV<T> & ul,const MatV<T> & ur,const MatV<T> & ll,const MatV<T> & lr)
{
    MatV<T>        ret(ul.nrows+ll.nrows,ul.ncols+ur.ncols);
    FGASSERT((ul.ncols == ll.ncols) && (ur.ncols == lr.ncols));
    FGASSERT((ul.nrows == ur.nrows) && (ll.nrows == lr.nrows));
    ret.setSubMat(0,0,ul);
    ret.setSubMat(0,ul.ncols,ur);
    ret.setSubMat(ul.nrows,0,ll);
    ret.setSubMat(ul.nrows,ul.ncols,lr);
    return ret;
}
template <class T>
MatV<T>
fgJoinHoriz(const Svec<MatV<T> > & ms)
{
    MatV<T>    ret;
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
MatV<T>
fgJoinHoriz(const MatV<T> & left,const MatV<T> & right)
{
    MatV<T>        retval;
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
MatV<T>
fgJoinVert(const Svec<MatV<T> > & ms)
{
    MatV<T>    ret;
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
MatV<T>
fgJoinVert(const MatV<T> & upper,const MatV<T> & lower)
{
    MatV<T>      ret;
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
MatV<T>
fgJoinVert(const MatV<T> & upper,const MatV<T> & middle,const MatV<T> & lower)
{
    FGASSERT(upper.numCols() == middle.numCols());
    FGASSERT(middle.numCols() == lower.numCols());
    MatV<T>      retval(upper.numRows()+middle.numRows()+lower.numRows(),upper.numCols());
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
MatV<T>
fgJoinDiagonal(const Svec<MatV<T> > & blocks)
{
    MatV<T>        ret;
    Sizes             rows(blocks.size(),0),
                        cols(blocks.size(),0);
    for (size_t ii=0; ii<blocks.size(); ++ii) {
        rows[ii] = blocks[ii].numRows();
        cols[ii] = blocks[ii].numCols();
    }
    size_t              nrows = cSum(rows),
                        ncols = cSum(cols),
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
MatV<T>
fgJoinDiagonal(const MatV<T> & b0,const MatV<T> & b1)
{
    MatV<T>        ret;
    size_t              nrows = b0.nrows + b1.nrows,
                        ncols = b0.ncols + b1.ncols;
    ret.resize(nrows,ncols,T(0));
    ret.setSubMat(0,0,b0);
    ret.setSubMat(b0.nrows,b0.ncols,b1);
    return ret;
}
template<class T>
MatV<T>
fgJoinDiagonal(const MatV<T> & b0,const MatV<T> & b1,const MatV<T> & b2)
{
    MatV<T>        ret;
    size_t              nrows = b0.nrows + b1.nrows + b2.nrows,
                        ncols = b0.ncols + b1.ncols + b2.ncols;
    ret.resize(nrows,ncols,T(0));
    ret.setSubMat(0,0,b0);
    ret.setSubMat(b0.nrows,b0.ncols,b1);
    ret.setSubMat(b0.nrows+b1.nrows,b0.ncols+b1.ncols,b2);
    return ret;
}

// Partition a square matrix into 4 matrices symmetrically:
Mat<MatD,2,2>
fgPartition(const MatD & m,size_t loSize);

template<class T>
MatV<T>
fgModulateCols(
    const MatV<T> &    matrix,
    const MatV<T> &    modVector)
{
    FGASSERT(matrix.numCols() == modVector.numElems());
    MatD       ret = matrix;
    for (uint rr=0; rr<matrix.numRows(); ++rr)
        for (uint cc=0; cc<matrix.numCols(); ++cc)
            ret.rc(rr,cc) *= modVector[cc];
    return ret;
}

template<class T>
MatV<T>
fgDiagonal(size_t dim,const T & val)
{
    MatV<T>    ret(dim,dim,T(0));
    for (uint ii=0; ii<dim; ++ii)
        ret.rc(ii,ii) = val;
    return ret;
}

template<class T>
MatV<T>
fgDiagonal(const Svec<T> & vec)
{
    uint            dim = uint(vec.size());
    MatV<T>    ret(dim,dim,T(0));
    for (uint ii=0; ii<dim; ++ii)
        ret.rc(ii,ii) = vec[ii];
    return ret;
}

template<class T>
MatV<T>
fgDiagonal(const MatV<T> & vec)
{
    FGASSERT((vec.numRows() == 1) || (vec.numCols() == 1));
    uint            dim = vec.numRows() * vec.numCols();
    MatV<T>    ret(dim,dim,T(0));
    for (uint ii=0; ii<dim; ++ii)
        ret.rc(ii,ii) = vec[ii];
    return ret;
}

template<class T>
MatV<T>
fgNormalize(const MatV<T> & m)
{return m * (1/std::sqrt(m.mag())); }

template<class T>
MatV<T>
fgOuterProduct(const Svec<T> & rowFacs,const Svec<T> & colFacs)
{
    MatV<T>        ret(rowFacs.size(),colFacs.size());
    size_t              cnt = 0;
    for (size_t rr=0; rr<rowFacs.size(); ++rr)
        for (size_t cc=0; cc<colFacs.size(); ++cc)
            ret[cnt++] = rowFacs[rr] * colFacs[cc];
    return ret;
}

MatD
fgRelDiff(const MatD & a,const MatD & b,double minAbs=0.0);

// Form a matrix from a vector of vectors representing each row:
template<class T>
MatV<T>
fgVecVecToMatrix(const Svec<Svec<T> > & vss)    // vss must be non-empty with all sub-vects of same size
{
    FGASSERT(!vss.empty());
    size_t          numRows = vss.size(),
                    numCols = vss[0].size();
    Svec<T>       data = fgFlat(vss);
    FGASSERT(data.size() == numRows*numCols);
    return MatV<T>(numRows,numCols,data);
}

// Return a vector of vectors for each row in a matrix:
template<class T>
Svec<Svec<T> >
fgMatrixToVecVec(const MatV<T> & v)
{
    Svec<Svec<T> >      ret(v.numRows());
    for (size_t rr=0; rr<ret.size(); ++rr)
        ret[rr] = v.rowData(uint(rr));
    return ret;
}

template<class T>
T
cMag(const MatV<T> & mat)
{return mat.mag(); }

template<class T>
T
fgOffDiagonalRelMag(const MatV<T> & mat)
{
    FGASSERT(mat.ncols == mat.nrows);
    T       sz = T(mat.ncols),
            diag = T(0),
            offd = T(0);
    for (size_t rr=0; rr<sz; ++rr) {
        diag += cMag(mat.rc(rr,rr));
        for (size_t cc=rr+1; cc<sz; ++cc)
            offd += cMag(mat.rc(rr,cc)) + cMag(mat.rc(cc,rr));
    }
    return offd * sz / (diag * sz * (sz-1));
}

}

#endif
