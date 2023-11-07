//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Variable-size (heap based) matrix / vector

#ifndef FGMATRIXV_HPP
#define FGMATRIXV_HPP

#include "FgMatrixC.hpp"    // Used to represent dimensions 

namespace Fg {

template <class T>
struct  MatV
{
    // the dimensions can't be changed to 'size_t' without breaking existing serialized data
    // having only one of the dimensions non-zero is valid and can used as a base case for incremental
    // composition:
    uint            nrows = 0;      // having only one of these non-zero *is* a valid state
    uint            ncols = 0;
    Svec<T>         m_data;         // invariant: m_data.size() == nrows*ncols

    FG_SER3(nrows,ncols,m_data)

    MatV() {}

    MatV(size_t rows,size_t cols) : nrows{scast<uint>(rows)}, ncols{scast<uint>(cols)}, m_data(rows*cols) {}
    MatV(size_t rows,size_t cols,T val) : nrows{scast<uint>(rows)}, ncols{scast<uint>(cols)}, m_data(rows*cols,val) {}
    MatV(size_t rows,size_t cols,Svec<T> const & v) : nrows{scast<uint>(rows)}, ncols{scast<uint>(cols)}, m_data(v)
        {FGASSERT(rows*cols == v.size()); }
    explicit MatV(Vec2UI colsRows) : nrows(colsRows[1]), ncols(colsRows[0]) {m_data.resize(ncols*nrows); }
    explicit MatV(Svec<Svec<T>> const & vecOfVecs)
    {
        if (!vecOfVecs.empty()) {
            nrows = scast<uint>(vecOfVecs.size());
            ncols = scast<uint>(vecOfVecs[0].size());
            m_data.reserve(nrows*ncols);
            for (Svec<T> const & vec : vecOfVecs) {
                FGASSERT(vec.size() == ncols);
                cat_(m_data,vec);
            }
        }
    }
    template<uint nr,uint nc>
    explicit MatV(Mat<T,nr,nc> const & rhs) : nrows{nr}, ncols{nc}, m_data{rhs.m.begin(),rhs.m.end()} {}

    void            clear() {m_data.clear(); nrows = ncols = 0; }
    void            resize(size_t rows,size_t cols) {nrows=scast<uint>(rows); ncols=scast<uint>(cols); m_data.resize(rows*cols); }
    void            resize(size_t rows,size_t cols,T v) {nrows=scast<uint>(rows); ncols=scast<uint>(cols); m_data.resize(rows*cols,v); }
    void            resize(Vec2UI cols_rows) {resize(cols_rows[1],cols_rows[0]); }
    // 'size_t' finds the default 'uint' constructor even on 64-bit so we must cast explicity:
    Vec2UI          dims() const {return Vec2UI{scast<uint>(nrows),scast<uint>(ncols)}; }
    size_t          numRows() const {return nrows; }
    size_t          numCols() const {return ncols; }
    size_t          numElems() const {return scast<size_t>(nrows)*scast<size_t>(ncols); }
    bool            empty() const {return m_data.empty(); }
    // Element access by (row,column):
    T &             rc(size_t row,size_t col)
    {
        FGASSERT_FAST((row < nrows) && (col < ncols));
        return m_data[row*ncols+col];
    }
    T const &       rc(size_t row,size_t col) const
    {
        FGASSERT_FAST((row < nrows) && (col < ncols));
        return m_data[row*ncols+col];
    }
    // Element access by (column,row):
    T &             cr(size_t col,size_t row)
    {
        FGASSERT_FAST((row < nrows) && (col < ncols));
        return m_data[row*ncols+col];
    }
    T const &       cr(size_t col,size_t row) const
    {
        FGASSERT_FAST((row < nrows) && (col < ncols));
        return m_data[row*ncols+col];
    }
    T &             operator[](Vec2UI colRow) {return cr(colRow[0],colRow[1]); }
    T const &       operator[](Vec2UI colRow) const {return cr(colRow[0],colRow[1]); }
    // these 2 overloads are a bit dangerous but useful when the MatV is a vector (ie ncols==1 or nrows==1)
    T &             operator[](size_t ii) {FGASSERT_FAST(ii<m_data.size()); return m_data[ii]; }
    T const &       operator[](size_t ii) const {FGASSERT_FAST(ii<m_data.size()); return m_data[ii]; }
    T const *       dataPtr() const {FGASSERT(!m_data.empty()); return &m_data[0]; }
    T const *       rowPtr(size_t row) const {FGASSERT(row < nrows); return &m_data[row*ncols]; }
    T *             rowPtr(size_t row) {FGASSERT(row < nrows); return &m_data[row*ncols]; }

    // Operators
    bool            operator==(MatV const & rhs) const
    {
        return ((nrows==rhs.nrows) && (ncols==rhs.ncols) && (m_data==rhs.m_data));
    }
    bool            operator!=(MatV const & rhs) const {return !(operator==(rhs)); }
    MatV            operator+(MatV const & rhs) const
    {
        FGASSERT((nrows == rhs.nrows) && (ncols == rhs.ncols));
        return MatV {nrows,ncols,m_data+rhs.m_data};
    }
    MatV            operator-(MatV const & rhs) const
    {
        FGASSERT((nrows == rhs.nrows) && (ncols == rhs.ncols));
        return MatV {nrows,ncols,m_data-rhs.m_data};
    }
    void            operator*=(T v) {m_data *= v; };
    void            operator+=(MatV const & rhs)
    {
        FGASSERT((nrows == rhs.nrows) && (ncols == rhs.ncols));
        m_data += rhs.m_data;
    }
    void            operator-=(MatV const & rhs)
    {
        FGASSERT((nrows == rhs.nrows) && (ncols == rhs.ncols));
        m_data -= rhs.m_data;
    }
    void            setSubMat(size_t row,size_t col,MatV const & m)
    {
        FGASSERT((m.nrows+row <= nrows) && (m.ncols+col <= ncols));
        for (size_t rr=0; rr<m.nrows; ++rr)
            for (size_t cc=0; cc<m.ncols; ++cc)
                rc(row+rr,col+cc) = m.rc(rr,cc);
    }
    // Set submatrix from the transpose of the given matrix (saves an allocation and a copy):
    void            setSubMatTr(size_t row,size_t col,MatV const & m)
    {
        FGASSERT((m.ncols+row <= nrows) && (m.nrows+col <= ncols));
        for (size_t rr=0; rr<m.ncols; ++rr)
            for (size_t cc=0; cc<m.nrows; ++cc)
                rc(row+rr,col+cc) = m.rc(cc,rr);
    }
    void            setConst(T v)
    {
        for (size_t ii=0; ii<m_data.size(); ii++)
            m_data[ii] = v;
    }
    void            setZero() {setConst(T(0)); }
    void            setIdentity()
    {
        setZero();
        size_t nn = (nrows < ncols) ? nrows : ncols;
        for (size_t ii=0; ii<nn; ii++)
            this->rc(ii,ii) = T(1);
    }
    double          mag() const {return cMag(m_data); }
    double          len() const {return cLen(m_data); }
    bool            isVector() const {return ((nrows*ncols>0) && ((nrows==1)||(ncols==1))); }
    Svec<T>         sliceCol(size_t col) const
    {
        FGASSERT(col < ncols);
        Svec<T>             ret; ret.reserve(nrows);
        T const             *ptr = (nrows>0) ? &m_data[col] : nullptr;
        for (size_t rr=0; rr<nrows; ++rr)
            ret.push_back(ptr[rr*ncols]);
        return ret;
    }
    MatV            subMatrix(size_t firstRow,size_t firstCol,size_t numRows_,size_t numCols_) const
    {
        FGASSERT(((firstCol + numCols_) <= ncols) && 
                 ((firstRow + numRows_) <= nrows));
        MatV<T> retval(numRows_,numCols_);
        for (size_t ii=0; ii<numRows_; ii++)
            for (size_t jj=0; jj<numCols_; jj++)
                retval.rc(ii,jj) = this->rc(ii+firstRow,jj+firstCol);
        return retval;
    }
    Svec<T>         colVals(size_t col) const
    {
        FGASSERT(col < ncols);
        Svec<T>             ret; ret.reserve(nrows);
        for (size_t rr=0; rr<nrows; ++rr)
            ret.push_back(m_data[rr*ncols + col]);
        return ret;
    }
    Svec<T>         rowVals(size_t row) const
    {
        FGASSERT(row < nrows);
        auto                it = m_data.cbegin() + row*ncols;
        return Svec<T>(it,it+ncols);
    }
    Svec<T>         getDiagonal() const
    {
        size_t          D = cMin(nrows,ncols);
        Svec<T>         ret; ret.reserve(D);
        for (size_t ii=0; ii<D; ++ii)
            ret.push_back(m_data[ii*ncols+ii]);
        return ret;
    }
    Svec<T>         sumCols() const         // sum values for each column
    {
        Svec<T>         ret (ncols,T{0});
        for (size_t rr=0; rr<nrows; ++rr) {
            T const         *ptr = (ncols>0) ? rowPtr(rr) : nullptr;
            for (size_t cc=0; cc<ncols; ++cc)
                ret[cc] += ptr[cc];
        }
        return ret;
    }

    static MatV     identity(size_t dim)
    {
        MatV                ret {dim,dim,T(0)};
        for (size_t ii=0; ii<dim; ++ii)
            ret.rc(ii,ii) = T(1);
        return ret;
    }
    static MatV     randNormal(size_t nrows,size_t ncols,T mean=T(0),T stdev=T(1))
    {
        return MatV<T>(nrows,ncols,cRandNormals(nrows*ncols,mean,stdev));
    }
};

template<class T>
struct Traits<MatV<T>>
{
    typedef typename Traits<T>::Scalar              Scalar;
    typedef typename Traits<T>::Accumulator         Accumulator;
    typedef typename Traits<T>::Floating            Floating;
};

typedef MatV<int>           MatI;
typedef MatV<float>         MatF;
typedef MatV<double>        MatD;
typedef Svec<MatD>          MatDs;

template<class T,class U>
MatV<T>             mapCast(MatV<U> const & m) {return MatV<T> {m.nrows,m.ncols,mapCast<T>(m.m_data)}; }

template<class T,class F>
void inline         dcast_(MatV<F> const & f,MatV<T> & t)
{
    t.resize(f.dims());
    dcast_(f.m_data,t.m_data);
}

template<class T>
MatV<T>             transpose(MatV<T> const & m)
{
    size_t              R = m.numRows(),
                        C = m.numCols();
    Svec<T>             ret; ret.reserve(R*C);
    for (size_t cc=0; cc<C; ++cc)
        for (size_t rr=0; rr<R; ++rr)
            ret.push_back(m.rc(rr,cc));
    return {C,R,ret};
}

// scalar multiplication of a matrix (whose elements may not be scalars):
template<typename T,typename U>
MatV<T>             operator*(MatV<T> const & m,U v)
{
    return MatV<T> {m.nrows,m.ncols,m.m_data*v};
}

template <class T>
std::ostream &      operator<<(std::ostream & ss,MatV<T> const & mm)
{
    if (mm.numRows()*mm.numCols() == 0)
        return ss << "[]";
    bool        isVec((mm.numRows() == 1) || mm.numCols() == 1);
    std::ios::fmtflags
        oldFlag = ss.setf(
            std::ios::fixed |
            std::ios::showpos |
            std::ios::right);
    std::streamsize oldPrec = ss.precision(6);
    if (isVec) {
        ss << "[" << mm[0];
        for (size_t ii=1; ii<mm.numElems(); ii++)
            ss << "," << mm[ii];
        ss << "]";
        if (mm.numRows() > 1) ss << "^T";       // Indicate transpose of column vector
    }
    else {
        ss << fgpush;
        for (size_t row=0; row<mm.numRows(); row++) {
            ss << fgnl;
            ss << "[ ";
            for (size_t col=0; col<mm.numCols(); col++)
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
MatV<T>             matMul(MatV<T> const & lhs,MatV<T> const & rhs)
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
MatV<T>             operator*(MatV<T> const & lhs,MatV<T> const & rhs) {return matMul(lhs,rhs); }

// treat RHS as a column vector (and assume output type is same as matrix type):
template<class T,class U>
Svec<T>             operator*(MatV<T> const & lhs,Svec<U> const & rhs)
{
    size_t              R = lhs.numRows(),
                        C = lhs.numCols();
    FGASSERT(rhs.size() == C);
    Svec<T>             ret; ret.reserve(R);
    for (size_t rr=0; rr<R; ++rr) {
        T                   acc {0};
        T const             *lhsPtr = &lhs.rc(rr,0);
        for (size_t cc=0; cc<C; ++cc)
            acc += lhsPtr[cc] * rhs[cc];
        ret.push_back(acc);
    }
    return ret;
}

// Specializations for float and double use Eigen library when matrix is large enough to be worth
// copying over:
template<> MatF     operator*(const MatF & lhs,const MatF & rhs);
template<> MatD     operator*(MatD const & lhs,MatD const & rhs);

// L * R^T much faster as a single operation:
template<class T>
MatV<T>             mulTr(MatV<T> const & l,MatV<T> const & r)
{
    FGASSERT(l.ncols == r.ncols);
    MatV<T>                 ret {l.nrows,r.nrows,T(0)};
    for (size_t rr=0; rr<l.nrows; ++rr) {
        for (size_t cc=0; cc<r.nrows; ++cc) {
            T                   acc {0};
            for (size_t ii=0; ii<l.ncols; ++ii)
                acc += l.rc(rr,ii) * r.rc(cc,ii);
            ret.rc(rr,cc) = acc;
        }
    }
    return ret;
}

template<class T>
MatV<T>             operator*(T const & lhs,MatV<T> const & rhs) {return (rhs*lhs); }

template<typename T>
MatV<T>             asColVec(Svec<T> const & v)
{
    FGASSERT(!v.empty());
    return MatV<T>(v.size(),1,v.data());
}
template<typename T>
MatV<T>             asRowVec(Svec<T> const & v)
{
    FGASSERT(!v.empty());
    return MatV<T>(1,v.size(),&v[0]);
}
template<class T>
Svec<Svec<T>>       asVecOfVecs(MatV<T> const & m)
{
    size_t              R = m.numRows(),
                        C = m.numCols();
    Svec<Svec<T>>       ret; ret.reserve(m.numRows());
    for (size_t rr=0; rr<R; ++rr) {
        auto                it = m.m_data.begin() + rr*C;
        ret.emplace_back(it,it+C);
    }
    return ret;
}

// treat rhs as a column vector:
template<class T>
Svec<T>             operator*(MatV<T> const & mat,Svec<T> const & vec)
{
    size_t              R = mat.numRows(),
                        C = mat.numCols();
    FGASSERT(vec.size() == C);
    Svec<T>             ret; ret.reserve(R);
    for (size_t rr=0; rr<R; ++rr) {
        T                   acc {0};
        T const *           rowPtr = mat.rowPtr(rr);
        for (size_t cc=0; cc<C; ++cc)
            acc += rowPtr[cc] * vec[cc];
        ret.push_back(acc);
    }
    return ret;
}

// treat lhs as row vector:
template<class T>
Svec<T>             operator*(Svec<T> const & vec,MatV<T> const & mat)
{
    size_t              R = mat.numRows(),
                        C = mat.numCols();
    FGASSERT(vec.size() == R);
    Svec<T>             ret (R,T{0});
    for (size_t rr=0; rr<R; ++rr) {
        T                   val = vec[rr];
        T const *           rowPtr = mat.rowPtr(rr);
        for (size_t cc=0; cc<C; ++cc)
            ret[cc] += val * rowPtr[cc];
    }
    return ret;
}

template<class T>
MatV<T>             cDiagMat(size_t dim,T const & val)
{
    MatV<T>    ret(dim,dim,T(0));
    for (size_t ii=0; ii<dim; ++ii)
        ret.rc(ii,ii) = val;
    return ret;
}

template<class T>
MatV<T>             cDiagMat(Svec<T> const & diagVals)
{
    size_t              D = diagVals.size();
    MatV<T>             ret {D,D,T(0)};
    for (size_t ii=0; ii<D; ++ii)
        ret.rc(ii,ii) = diagVals[ii];
    return ret;
}

template<class T>
T                   cTrace(MatV<T> const & m)
{
    T                   ret(0);
    size_t              dim = cMinElem(m.dims());
    for (size_t ii=0; ii<dim; ++ii)
        ret += m.rc(ii,ii);
    return ret;
}

template<class T>
inline MatV<T>      mapAbs(MatV<T> const & mat) {return MatV<T>(mat.nrows,mat.ncols,mapAbs(mat.m_data)); }

template<class T>
MatV<T>             appendCol(MatV<T> const & mat,Svec<T> const & col)
{
    size_t              R = mat.numRows(),
                        C = mat.numCols();
    FGASSERT(col.size() == R);
    return {
        R,C+1,
        [&,R,C](){
            Svec<T>         ret; ret.reserve(R*(C+1));
            for (size_t rr=0; rr<R; ++rr) {
                auto            it = mat.m_data.cbegin() + rr*C;
                ret.insert(ret.end(),it,it+C);
                ret.push_back(col[rr]);
            }
            return ret;
        }()
    };
}

template<class T>
MatV<T>             eraseCol(MatV<T> const & mat,size_t col)
{
    size_t              R = mat.numRows(),
                        C = mat.numCols();
    FGASSERT(col < C);
    return {
        R,C-1,
        [&mat,R,C,col]() {
            Svec<T>             ret; ret.reserve(R*(C-1));
            for (size_t rr=0; rr<R; ++rr) {
                auto                it = mat.m_data.cbegin() + rr*C;
                ret.insert(ret.end(),it,it+col);
                ret.insert(ret.end(),it+col+1,it+C);
            }
            return ret;
        }()
    };
}

// Matrix join is the inverse of partition:
// 2x2 block matrix join:
template<class T>
MatV<T>             joinMat(MatV<T> const & ul,MatV<T> const & ur,MatV<T> const & ll,MatV<T> const & lr)
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
MatV<T>             catHoriz(const Svec<MatV<T> > & ms)
{
    MatV<T>    ret;
    FGASSERT(!ms.empty());
    size_t            rows = ms[0].numRows(),
                    cols = 0;
    for (size_t ii=0; ii<ms.size(); ++ii) {
        FGASSERT(ms[ii].nrows == rows);
        cols += ms[ii].numCols();
    }
    ret.resize(rows,cols);
    size_t            col = 0;
    for (size_t ii=0; ii<ms.size(); ++ii) {
        ret.setSubMat(0,col,ms[ii]);
        col += ms[ii].numCols();
    }
    return ret;
}
template <class T>
MatV<T>             catHoriz(MatV<T> const & left,MatV<T> const & right)
{
    MatV<T>        retval;
    if (left.empty())
        retval = right;
    else if (right.empty())
        retval = left;
    else {
        FGASSERT(left.numRows() == right.numRows());
        size_t            numRows = left.numRows(),
                        numCols = left.numCols() + right.numCols();
        retval.resize(numRows,numCols);
        for (size_t rr=0; rr<numRows; rr++) {
            size_t    col=0;
            for (size_t cc=0; cc<left.numCols(); ++cc)
                retval.rc(rr,col++) = left.rc(rr,cc);
            for (size_t cc=0; cc<right.numCols(); ++cc)
                retval.rc(rr,col++) = right.rc(rr,cc);
        }
    }
    return retval;
}
template <class T>
MatV<T>             catVertical(const Svec<MatV<T> > & ms)
{
    MatV<T>    ret;
    FGASSERT(!ms.empty());
    size_t            rows = 0,
                    cols = ms[0].ncols;
    for (size_t ii=0; ii<ms.size(); ++ii) {
        FGASSERT(ms[ii].ncols == cols);
        rows += ms[ii].nrows;
    }
    ret.resize(rows,cols);
    size_t            row = 0;
    for (size_t ii=0; ii<ms.size(); ++ii) {
        ret.setSubMat(row,0,ms[ii]);
        row += ms[ii].nrows;
    }
    return ret;
}
template <class T>
MatV<T>             catVertical(MatV<T> const & upper,MatV<T> const & lower)
{
    MatV<T>      ret;
    if (upper.empty())
        ret = lower;
    else if (lower.empty())
        ret = upper;
    else {
        FGASSERT(upper.numCols() == lower.numCols());
        ret.resize(upper.numRows()+lower.numRows(),upper.numCols());
        for (size_t ii=0; ii<upper.numElems(); ++ii)
            ret[ii] = upper[ii];
        size_t    off = upper.numElems();
        for (size_t ii=0; ii<lower.numElems(); ++ii)
            ret[off+ii] = lower[ii];
    }
    return ret;
}
template <class T>
MatV<T>             catVertical(MatV<T> const & upper,MatV<T> const & middle,MatV<T> const & lower)
{
    FGASSERT(upper.numCols() == middle.numCols());
    FGASSERT(middle.numCols() == lower.numCols());
    MatV<T>      retval(upper.numRows()+middle.numRows()+lower.numRows(),upper.numCols());
    size_t    row=0;
    for (size_t rr=0; rr<upper.numRows(); rr++)
        for (size_t col=0; col<upper.numCols(); col++)
            retval.rc(row++,col) = upper.rc(rr,col);
    for (size_t rr=0; rr<middle.numRows(); rr++)
        for (size_t col=0; col<middle.numCols(); col++)
            retval.rc(row++,col) = middle.rc(rr,col);
    for (size_t rr=0; rr<lower.numRows(); rr++)
        for (size_t col=0; col<lower.numCols(); col++)
            retval.rc(row++,col) = lower.rc(rr,col);
    return retval;
}

// Matrix "Direct Sum" - concatenate diagonally with all off-diagonal blocks set to zero.
// If all matrices are square the result will be block diagonal.
template<class T>
MatV<T>             catDiagonal(const Svec<MatV<T> > & blocks)
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
MatV<T>             catDiagonal(MatV<T> const & b0,MatV<T> const & b1)
{
    MatV<T>         ret;
    size_t          nrows = b0.nrows + b1.nrows,
                    ncols = b0.ncols + b1.ncols;
    ret.resize(nrows,ncols,T(0));
    ret.setSubMat(0,0,b0);
    ret.setSubMat(b0.nrows,b0.ncols,b1);
    return ret;
}
template<class T>
MatV<T>             catDiagonal(MatV<T> const & b0,MatV<T> const & b1,MatV<T> const & b2)
{
    MatV<T>         ret;
    size_t          nrows = b0.nrows + b1.nrows + b2.nrows,
                    ncols = b0.ncols + b1.ncols + b2.ncols;
    ret.resize(nrows,ncols,T(0));
    ret.setSubMat(0,0,b0);
    ret.setSubMat(b0.nrows,b0.ncols,b1);
    ret.setSubMat(b0.nrows+b1.nrows,b0.ncols+b1.ncols,b2);
    return ret;
}

// Partition a square matrix into 4 matrices symmetrically:
Mat<MatD,2,2>       cPartition(MatD const & m,size_t loSize);

template<class T>
MatV<T>             normalize(MatV<T> const & m) {return m * (1/std::sqrt(m.mag())); }

template<class T>
MatV<T>             cOuterProduct(Svec<T> const & rowFacs,Svec<T> const & colFacs)
{
    MatV<T>             ret(rowFacs.size(),colFacs.size());
    for (size_t rr=0; rr<rowFacs.size(); ++rr)
        for (size_t cc=0; cc<colFacs.size(); ++cc)
            ret.rc(rr,cc) = rowFacs[rr] * colFacs[cc];
    return ret;
}

MatD                cRelDiff(MatD const & a,MatD const & b,double minAbs=0.0);

template<class T>
T                   cMag(MatV<T> const & mat) {return mat.mag(); }

// Return sum of squared values of diagonal elements:
template<class T>
T                   cDiagMag(MatV<T> const & mat)
{
    size_t          sz = cMin(mat.numRows(),mat.numCols());
    T               acc {0};
    for (size_t ii=0; ii<sz; ++ii)
        acc += sqr(mat.rc(ii,ii));
    return acc;
}

// Return sum of squared values of off-diagonal elements:
template<class T>
double              cOffDiagMag(MatV<T> const & mat)
{
    size_t          nrows = mat.numRows(),
                    ncols = mat.numCols();
    double          acc {0};
    for (size_t rr=0; rr<nrows; ++rr)
        for (size_t cc=0; cc<ncols; ++cc)
            acc += (cc == rr) ? 0.0 : sqr(mat.rc(rr,cc));
    return acc;
}

template<class T>
double              cRowMag(MatV<T> const & mat,size_t row)
{
    FGASSERT(row < mat.numRows());
    double              ret {0};
    size_t              off = row * mat.numCols();
    for (size_t cc=off; cc<off+mat.numCols(); ++cc)
        ret += cMag(mat.m_data[cc]);
    return ret;
}

template<class T>
double              cRowDotProd(MatV<T> const & mat,size_t row0,size_t row1)
{
    double              ret {0};
    T const             *ptr0 = mat.rowPtr(row0),
                        *ptr1 = mat.rowPtr(row1);
    for (size_t cc=0; cc<mat.ncols; ++cc)
        ret += cDot(ptr0[cc],ptr1[cc]);
    return ret;
}

// row covariance of M == M * M^T using ignorance prior on mean and stdev.
// if D<3, will still return matrices but they cannot reflect a proper prior:
template<class T>
MatD                cRowCovariance(MatV<T> const & mat)
{
    size_t                  R = mat.numRows();
    MatD                    ret {R,R};
    double                  denom = cMax(1.0,scast<double>(mat.numCols())-2), // be robust for dimensions < 3
                            fac = 1.0 / denom;
    for (size_t r0=0; r0<R; ++r0) {
        ret.rc(r0,r0) = cRowMag(mat,r0) * fac;
        for (size_t r1=0; r1<r0; ++r1) {
            double              dot = cRowDotProd(mat,r0,r1) * fac;
            ret.rc(r0,r1) = dot;
            ret.rc(r1,r0) = dot;
        }
    }
    return ret;
}

// Pearson (normalized) correlation:
// UT option only fills non-diagonal upper triangular values, leaving the rest at zero.
template<class T>
MatD                cRowCorrelation(MatV<T> const & mat,bool utOnly=false)
{
    size_t              R = mat.numRows();
    Doubles             lens; lens.reserve(R);
    MatD                ret {R,R,0.0};
    for (size_t r0=0; r0<R; ++r0) {
        double          len = sqrt(cRowMag(mat,r0)),
                        lenNz = (len == 0.0) ? 1.0 : len;
        lens.push_back(lenNz);
        if (!utOnly)
            ret.rc(r0,r0) = (len == 0.0) ? 0.0 : 1.0;
        for (size_t r1=0; r1<r0; ++r1) {
            double              corr = cRowDotProd(mat,r0,r1) / (lens[r0]*lens[r1]);
            ret.rc(r1,r0) = corr;
            if (!utOnly)
                ret.rc(r0,r1) = corr;
        }
    }
    return ret;
}

template<class T>
MatV<T>             scaleRows(MatV<T> const & mat,Svec<T> const & vals)
{
    FGASSERT(mat.numRows() == vals.size());
    MatV<T>                 ret {mat.dims()};
    for (size_t rr=0; rr<mat.numRows(); ++rr)
        for (size_t cc=0; cc<mat.numCols(); ++cc)
            ret.rc(rr,cc) = mat.rc(rr,cc) * vals[rr];
    return ret;
}

template<class T>
MatV<T>             scaleColumns(MatV<T> const & mat,Svec<T> const & vals)
{
    FGASSERT(mat.numCols() == vals.size());
    MatV<T>                 ret {mat.dims()};
    for (size_t rr=0; rr<mat.numRows(); ++rr)
        for (size_t cc=0; cc<mat.numCols(); ++cc)
            ret.rc(rr,cc) = mat.rc(rr,cc) * vals[cc];
    return ret;
}

template<class T>
Vec2UI              cMaxIdx(MatV<T> const & m)      // returns (col,row)
{
    size_t              maxIdx = cMaxIdx(m.m_data),
                        r = maxIdx / m.ncols,
                        c = maxIdx % m.ncols;
    return {scast<uint>(c),scast<uint>(r)};
}

// Random orthogonal matrix of given dimension
MatD        cRandOrthogonal(size_t dim);
// Mahalanobis transform formed by D*R where R is random orthogonal, and
// D is a diagonal matrix of scales whose logarithms are distributed as N(0,logScaleStdev):
MatD        cRandMahalanobis(size_t dims,double logScaleStdev);

template<class T>
struct      MatSV                   // symmetric variable-size matrix
{
    size_t              dim;        // matrix must be square
    Svec<T>             data;       // only UT elements are stored, in row major order
    FG_SER2(dim,data);

    MatSV() : dim{0} {}
    explicit MatSV(size_t s) : dim{s}, data(s*(s+1)/2) {}
    MatSV(size_t s,Svec<T> const & d) : dim{s}, data{d} {FGASSERT(d.size() == s*(s+1)/2); }

    inline size_t       cIdx(size_t row,size_t col) const
    {
        if (col < row) std::swap(row,col);
        return row * dim - row*(row-1)/2 + col;
    }
    T const &           rc(size_t row,size_t col) const
    {
        FGASSERT_FAST((row<dim) && (col<dim));
        return data[cIdx(row,col)];
    }
    T &                 rc(size_t row,size_t col)
    {
        FGASSERT_FAST((row<dim) && (col<dim));
        return data[cIdx(row,col)];
    }

    MatV<T>             asMatV() const
    {
        MatV<T>             ret {dim,dim};
        size_t              idx {0};
        for (size_t rr=0; rr<dim; ++rr) {
            ret.rc(rr,rr) = data[idx++];
            for (size_t cc=rr+1; cc<dim; ++cc) {
                T const &           e = data[idx++];
                ret.rc(rr,cc) = e;
                ret.rc(cc,rr) = e;
            }
        }
        return ret;
    }
};

// Random symmetric positive definite matrix with eigenvalue square roots (scales) whose
// logarithms are distributed as N(0,logScaleStdev):
MatD        cRandSPD(size_t dims,double logScaleStdev);

}

#endif
