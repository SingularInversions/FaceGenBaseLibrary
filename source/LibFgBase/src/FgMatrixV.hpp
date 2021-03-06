//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
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

    MatV(size_t numRows,size_t numCols,T const *ptr)
    : nrows(uint(numRows)), ncols(uint(numCols)), m_data(ptr,ptr+numRows*numCols)
    {}

    MatV(size_t nr,size_t nc,Svec<T> const & v) : nrows(uint(nr)), ncols(uint(nc)), m_data(v)
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

    Vec2UI          dims() const {return Vec2UI(nrows,ncols); }
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
    T const &
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
    T const &
    cr(size_t col,size_t row) const
    {
        FGASSERT_FAST((row < nrows) && (col < ncols));
        return m_data[row*ncols+col];
    }

    T &
    operator[](size_t ii)
    {FGASSERT_FAST(ii<m_data.size()); return m_data[ii]; }

    T const &
    operator[](size_t ii) const
    {FGASSERT_FAST(ii<m_data.size()); return m_data[ii]; }

    T &
    operator[](Vec2UI coord)
    {return cr(coord[0],coord[1]); }

    T const &
    operator[](Vec2UI coord) const
    {return cr(coord[0],coord[1]); }

    T const *
    data() const
    {FGASSERT(!m_data.empty()); return &m_data[0]; }

    T const *
    rowPtr(size_t row) const
    {
        FGASSERT(!m_data.empty());
        FGASSERT(row < nrows);
        return &m_data[row*ncols];
    }

    Svec<T> const &
    dataVec() const
    {return m_data; }

    Svec<T>
    rowData(uint row) const
    {
        T const *       rPtr = rowPtr(row);
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
    addRow(Svec<T> const & data)
    {
        FGASSERT(data.size() == ncols);
        cat_(m_data,data);
        ++nrows;
    }

    // Operators

    bool
    operator==(MatV const & rhs) const
    {return ((nrows==rhs.nrows) && (ncols==rhs.ncols) && (m_data==rhs.m_data)); }

    bool
    operator!=(MatV const & rhs) const
    {return !(operator==(rhs)); }

    MatV
    operator*(T v) const
    {
        return MatV {nrows,ncols,m_data*v};
    }

    MatV
    operator+(MatV const & rhs) const
    {
        FGASSERT((nrows == rhs.nrows) && (ncols == rhs.ncols));
        return MatV {nrows,ncols,m_data+rhs.m_data};
    }

    MatV
    operator-(MatV const & rhs) const
    {
        FGASSERT((nrows == rhs.nrows) && (ncols == rhs.ncols));
        return MatV {nrows,ncols,m_data-rhs.m_data};
    }

    void
    operator*=(T v)
    {m_data *= v; };

    void
    operator+=(MatV const & rhs)
    {
        FGASSERT((nrows == rhs.nrows) && (ncols == rhs.ncols));
        m_data += rhs.m_data;
    }

    void
    operator-=(MatV const & rhs)
    {
        FGASSERT((nrows == rhs.nrows) && (ncols == rhs.ncols));
        m_data -= rhs.m_data;
    }

    void
    setSubMat(size_t row,size_t col,MatV const & m)
    {
        FGASSERT((m.nrows+row <= nrows) && (m.ncols+col <= ncols));
        for (uint rr=0; rr<m.nrows; ++rr)
            for (uint cc=0; cc<m.ncols; ++cc)
                rc(row+rr,col+cc) = m.rc(rr,cc);
    }

    // Set submatrix from the transpose of the given matrix (saves an allocation and a copy):
    void
    setSubMatTr(size_t row,size_t col,MatV const & m)
    {
        FGASSERT((m.ncols+row <= nrows) && (m.nrows+col <= ncols));
        for (uint rr=0; rr<m.ncols; ++rr)
            for (uint cc=0; cc<m.nrows; ++cc)
                rc(row+rr,col+cc) = m.rc(cc,rr);
    }

    // Accumulate in sub-matrix:
    void
    accSubMat(size_t row,size_t col,MatV const & m)
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

    static
    MatV
    identity(size_t dim)
    {
        MatV                ret {dim,dim,T(0)};
        for (size_t ii=0; ii<dim; ++ii)
            ret.rc(ii,ii) = T(1);
        return ret;
    }

    static
    MatV
    randNormal(size_t nrows,size_t ncols,T mean=T(0),T stdev=T(1))
    {
        return MatV<T>(nrows,ncols,cRandNormals(nrows*ncols,mean,stdev));
    }
};

typedef MatV<float>         MatF;
typedef MatV<double>        MatD;
typedef Svec<MatD>          MatDs;

template <class T>
std::ostream &
operator<<(std::ostream & ss,MatV<T> const & mm)
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
matMul(MatV<T> const & lhs,MatV<T> const & rhs)
{
    FGASSERT(lhs.ncols == rhs.nrows);
    // Simple block sub-loop cache optimization, no multithreading or explicit SIMD.
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
operator*(MatV<T> const & lhs,MatV<T> const & rhs)
{return matMul(lhs,rhs); }

// Specializations for float and double use Eigen library when matrix is large enough to be worth
// copying over:
template<>
MatF
operator*(const MatF & lhs,const MatF & rhs);

template<>
MatD
operator*(MatD const & lhs,MatD const & rhs);

template<class T>
MatV<T>
operator*(T const & lhs,MatV<T> const & rhs)
{return (rhs*lhs); }

template<typename T>
MatV<T>
asColVec(Svec<T> const & v)
{
    FGASSERT(!v.empty());
    return MatV<T>(uint(v.size()),1,v.data());
}

template<typename T>
MatV<T>
asRowVec(Svec<T> const & v)
{
    FGASSERT(!v.empty());
    return MatV<T>(1,uint(v.size()),&v[0]);
}

MatD &
operator/=(MatD & mat,double div);

// MatV<> * Svec<> treats rhs side as a column vector and returns same:
template<class T>
Svec<T>
operator*(MatV<T> const & lhs,Svec<T> const & rhs)
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
operator*(Svec<T> const & lhs,MatV<T> const & rhs)
{
    Svec<T>       ret(rhs.ncols,T(0));
    FGASSERT(lhs.size() == rhs.nrows);
    for (uint rr=0; rr<rhs.nrows; ++rr)
        for (uint cc=0; cc<rhs.ncols; ++cc)
            ret[cc] += lhs[rr] * rhs.rc(rr,cc);
    return ret;
}

template<class T>
T
cDot(MatV<T> const & lhs,MatV<T> const & rhs)
{return cDot(lhs.m_data,rhs.m_data); }

template<class T>
MatV<T>
cDiagMat(size_t dim,T const & val)
{
    MatV<T>    ret(dim,dim,T(0));
    for (uint ii=0; ii<dim; ++ii)
        ret.rc(ii,ii) = val;
    return ret;
}

template<class T>
MatV<T>
cDiagMat(Svec<T> const & diagVals)
{
    size_t              D = diagVals.size();
    MatV<T>             ret {D,D,T(0)};
    for (size_t ii=0; ii<D; ++ii)
        ret.rc(ii,ii) = diagVals[ii];
    return ret;
}

template<class T>
MatV<T>
cDiagMat(MatV<T> const & vec)
{
    FGASSERT((vec.numRows() == 1) || (vec.numCols() == 1));
    uint            dim = vec.numRows() * vec.numCols();
    MatV<T>         ret(dim,dim,T(0));
    for (uint ii=0; ii<dim; ++ii)
        ret.rc(ii,ii) = vec[ii];
    return ret;
}

template<class T>
T
cTrace(MatV<T> const & m)
{
    T                   ret(0);
    size_t              dim = cMinElem(m.dims());
    for (size_t ii=0; ii<dim; ++ii)
        ret += m.rc(ii,ii);
    return ret;
}

// Linear interpolation between matrices of equal dimensions:
template<class T>
void
interpolate_(MatV<T> const & v0,MatV<T> const & v1,T val,MatV<T> & ret)
{
    Vec2UI              dims = v0.dims();
    FGASSERT(v1.dims() == dims);
    ret.resize(dims);
    interpolate_(v0.m_data,v1.m_data,val,ret.m_data);
}

// Map 'abs':
template<class T>
MatV<T>
mapAbs(MatV<T> const & mat)
{return MatV<T>(mat.nrows,mat.ncols,mapAbs(mat.m_data)); }

// Matrix join is the inverse of partition:
// 2x2 block matrix join:
template<class T>
MatV<T>
fgJoin(MatV<T> const & ul,MatV<T> const & ur,MatV<T> const & ll,MatV<T> const & lr)
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
catHoriz(const Svec<MatV<T> > & ms)
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
catHoriz(MatV<T> const & left,MatV<T> const & right)
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
catVertical(const Svec<MatV<T> > & ms)
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
catVertical(MatV<T> const & upper,MatV<T> const & lower)
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
catVertical(MatV<T> const & upper,MatV<T> const & middle,MatV<T> const & lower)
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

// Matrix "Direct Sum" - concatenate diagonally with all off-diagonal blocks set to zero.
// If all matrices are square the result will be block diagonal.
template<class T>
MatV<T>
catDiagonal(const Svec<MatV<T> > & blocks)
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
catDiagonal(MatV<T> const & b0,MatV<T> const & b1)
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
catDiagonal(MatV<T> const & b0,MatV<T> const & b1,MatV<T> const & b2)
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
cPartition(MatD const & m,size_t loSize);

template<class T>
MatV<T>
normalize(MatV<T> const & m)
{return m * (1/std::sqrt(m.mag())); }

template<class T>
MatV<T>
cOuterProduct(Svec<T> const & rowFacs,Svec<T> const & colFacs)
{
    MatV<T>             ret(rowFacs.size(),colFacs.size());
    for (size_t rr=0; rr<rowFacs.size(); ++rr)
        for (size_t cc=0; cc<colFacs.size(); ++cc)
            ret.rc(rr,cc) = rowFacs[rr] * colFacs[cc];
    return ret;
}

MatD
cRelDiff(MatD const & a,MatD const & b,double minAbs=0.0);

// Return a std::vector of std::vectors for each row in a matrix:
template<class T>
Svec<Svec<T> >
toSvecOfSvec(MatV<T> const & v)
{
    Svec<Svec<T> >      ret(v.numRows());
    for (size_t rr=0; rr<ret.size(); ++rr)
        ret[rr] = v.rowData(uint(rr));
    return ret;
}

template<class T>
T
cMag(MatV<T> const & mat)
{return mat.mag(); }

// Return sum of squared values of diagonal elements:
template<class T>
T
cDiagMag(MatV<T> const & mat)
{
    size_t          sz = cMin(mat.numRows(),mat.numCols());
    T               acc {0};
    for (size_t ii=0; ii<sz; ++ii)
        acc += sqr(mat.rc(ii,ii));
    return acc;
}

// Return sum of squared values of off-diagonal elements:
template<class T>
T
cOffDiagMag(MatV<T> const & mat)
{
    size_t          nrows = mat.numRows(),
                    ncols = mat.numCols();
    T               acc {0};
    for (size_t rr=0; rr<nrows; ++rr)
        for (size_t cc=0; cc<ncols; ++cc)
            if (cc != rr)
                acc += sqr(mat.rc(rr,cc));
    return acc;
}

// row covariance of M == M * M^T
template<class T>
MatV<T>
cRowCovariance(MatV<T> const & m)
{
    size_t                  R = m.numRows(),
                            C = m.numCols();
    MatV<T>                 ret {R,R,T(0)};
    for (size_t r0=0; r0<R; ++r0) {
        {   // Optimize the self-covariance computation:
            T                   acc {0};
            for (size_t cc=0; cc<C; ++cc)
                acc += sqr(m.rc(r0,cc));
            ret.rc(r0,r0) = acc;
        }
        for (size_t r1=r0+1; r1<R; ++r1) {
            T                   acc {0};
            for (size_t cc=0; cc<C; ++cc)
                acc += m.rc(r0,cc) * m.rc(r1,cc);
            ret.rc(r0,r1) = acc;
            ret.rc(r1,r0) = acc;
        }
    }
    return ret;
}

// Random orthogonal matrix of given dimension
MatD        cRandOrthogonal(size_t dim);
// Mahalanobis transform formed by D*R where R is random orthogonal, and
// D is a diagonal matrix of scales whose logarithms are distributed as N(0,logScaleStdev):
MatD        cRandMahalanobis(size_t dims,double logScaleStdev);
// Random symmetric positive definite matrix with eigenvalue square roots (scales) whose
// logarithms are distributed as N(0,logScaleStdev):
MatD        cRandSPD(size_t dims,double logScaleStdev);

}

#endif
