//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Variable-size (heap based) matrix / vector

#ifndef FGMATRIXV_HPP
#define FGMATRIXV_HPP

#include "FgMatrixC.hpp"    // Used to represent dimensions 

namespace Fg {

template<class T>
Svec<T>             genRectangulars(size_t R,size_t C,Sfun<T(size_t rr,size_t cc)> const & fn)
{
    Svec<T>             ret; ret.reserve(R*C);
    for (size_t rr=0; rr<R; ++rr)
        for (size_t cc=0; cc<C; ++cc)
            ret.push_back(fn(rr,cc));
    return ret;
}

template <class T>
struct  MatV
{
    // dimensions can't be changed to 'size_t' without breaking existing serialized data
    uint            nrows = 0;      // only one of these non-zero is valid state; used for incremental construction
    uint            ncols = 0;
    Svec<T>         m_data;         // invariant: m_data.size() == nrows*ncols
    FG_SER(nrows,ncols,m_data)

    MatV() {}
    // WARNING unlike Image or Iter, this ctor is in order of the major index then the minor index:
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
    T &             operator[](Vec2UL colRow) {return cr(colRow[0],colRow[1]); }
    T const &       operator[](Vec2UL colRow) const {return cr(colRow[0],colRow[1]); }
    // these 2 overloads are a bit dangerous but useful when the MatV is a vector (ie ncols==1 or nrows==1)
    T &             operator[](size_t ii) {FGASSERT_FAST(ii<m_data.size()); return m_data[ii]; }
    T const &       operator[](size_t ii) const {FGASSERT_FAST(ii<m_data.size()); return m_data[ii]; }
    T const *       dataPtr() const {FGASSERT(!m_data.empty()); return &m_data[0]; }
    T const *       rowPtr(size_t row) const {FGASSERT(row < nrows); return &m_data[row*ncols]; }
    T *             rowPtr(size_t row) {FGASSERT(row < nrows); return &m_data[row*ncols]; }
    PArr<T>         rowParr(size_t row) const {FGASSERT(row<nrows); return {dataPtr(),ncols}; }
    PArr<T>         colParr(size_t col) const {FGASSERT(col<ncols); return {dataPtr()+col,nrows,ncols}; }

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
    double          magD() const {return cMagD(m_data); }
    double          len() const {return cLenD(m_data); }
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
    typedef typename Traits<T>::Floating            Floating;
};

typedef MatV<int>           MatI;
typedef MatV<float>         MatF;
typedef MatV<double>        MatD;
typedef Svec<MatD>          MatDs;

template<class T,class C>
auto                mapCall(MatV<T> const & mat,C fn)
{
    typedef decltype(fn(mat[0]))   R;
    return MatV<R>{mat.nrows,mat.ncols,mapCall(mat.m_data,fn)};
}

template<class T,class U>
MatV<T>             mapCast(MatV<U> const & m) {return MatV<T> {m.nrows,m.ncols,mapCast<T>(m.m_data)}; }

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

// matMul is a multiply-accumulate contraction between the minor index of 'lhs' and the major index of 'rhs'.
// the return type must support construction from '0'.
template<class T,class U>
auto                matMul(MatV<T> const & lhs,MatV<U> const & rhs)
{
    typedef decltype(T{}*U{})   R;
    FGASSERT(lhs.ncols == rhs.nrows);
    // block sub-loop cache optimization, no multithreading or explicit SIMD.
    // use eigen instead if speed is important and elements are scalars.
    // below we calculate the number of elements that fit in a typical cache line of 64 bytes:
    size_t constexpr    CN = std::max(std::min(64/sizeof(T),64/sizeof(U)),size_t(2));
    MatV<R>             ret {lhs.nrows,rhs.ncols,R(0)};
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

template<class T,class U>
auto                operator*(MatV<T> const & lhs,MatV<U> const & rhs) {return matMul(lhs,rhs); }

// treat RHS as a column vector:
template<class T,class U>
auto                operator*(MatV<T> const & lhs,Svec<U> const & rhs)
{
    typedef decltype(lhs[0]*rhs[0]) V;
    size_t              R = lhs.numRows(),
                        C = lhs.numCols();
    FGASSERT(rhs.size() == C);
    Svec<V>             ret; ret.reserve(R);
    for (size_t rr=0; rr<R; ++rr) {
        V                   acc {0};
        T const *           lhsPtr = lhs.rowPtr(rr);
        for (size_t cc=0; cc<C; ++cc)
            acc += lhsPtr[cc] * rhs[cc];
        ret.push_back(acc);
    }
    return ret;
}
// treat LHS as a row vector:
template<class T,class U>
auto                operator*(Svec<T> const & lhs,MatV<U> const & rhs)
{
    typedef decltype(lhs[0]*rhs[0]) V;
    size_t              R = rhs.numRows(),
                        C = rhs.numCols();
    FGASSERT(lhs.size() == R);
    Svec<V>             ret (C,V{0});
    for (size_t rr=0; rr<R; ++rr) {
        T                   val = lhs[rr];
        U const *           rowPtr = rhs.rowPtr(rr);
        for (size_t cc=0; cc<C; ++cc)
            ret[cc] += val * rowPtr[cc];
    }
    return ret;
}

// Specializations for float and double use Eigen library when matrix is large enough to be worth
// copying over:
template<> MatF     operator*(const MatF & lhs,const MatF & rhs);
template<> MatD     operator*(MatD const & lhs,MatD const & rhs);

// L * R^T faster as a single operation:
template<class T>
MatV<T>             matMulTr(MatV<T> const & l,MatV<T> const & r)
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
MatV<T>             cColVec(Svec<T> const & v)
{
    FGASSERT(!v.empty());
    return MatV<T>{v.size(),1,v};
}
template<typename T>
MatV<T>             cRowVec(Svec<T> const & v)
{
    FGASSERT(!v.empty());
    return MatV<T>{1,v.size(),v};
}
template<class T>
Svec<Svec<T>>       asSvecSvec(MatV<T> const & m)
{
    size_t              R = m.numRows(),
                        C = m.numCols();
    Svec<Svec<T>>       ret; ret.reserve(R);
    for (size_t rr=0; rr<R; ++rr) {
        auto                it = m.m_data.begin() + rr*C;
        ret.emplace_back(it,it+C);
    }
    return ret;
}

template<class T>
MatV<T>             cMatDiag(size_t D,T const & diagVal)
{
    MatV<T>             ret {D,D,0};
    for (size_t ii=0; ii<D; ++ii)
        ret.rc(ii,ii) = diagVal;
    return ret;
}

template<class T>
MatV<T>             cMatDiag(Svec<T> const & diagVals)
{
    size_t              D = diagVals.size();
    MatV<T>             ret {D,D,0};
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

template <class T>
MatV<T>             catH(MatV<T> const & l,MatV<T> const & r)
{
    size_t              R = l.numRows(),
                        C = l.numCols() + r.numCols();
    FGASSERT(r.numRows() == R);
    auto                fn = [&,R,C]()
    {
        Svec<T>             ret(R*C);
        T *                 retPtr = ret.data();
        for (size_t rr=0; rr<R; ++rr) {
            retPtr = copy_(l.rowPtr(rr),retPtr,l.numCols());
            retPtr = copy_(r.rowPtr(rr),retPtr,r.numCols());
        }
        return ret;
    };
    return MatV<T>{R,C,fn()};
}

template<class T>
MatV<T>             normalize(MatV<T> const & m) {return m * (1/std::sqrt(m.magD())); }

template<class T>
MatV<T>             cOuterProduct(Svec<T> const & rowFacs,Svec<T> const & colFacs)
{
    size_t              R = rowFacs.size(),
                        C = colFacs.size();
    return MatV<T>{R,C,[&](size_t rr,size_t cc){return rowFacs[rr] * colFacs[cc]; }};
}

MatD                cRelDiff(MatD const & a,MatD const & b,double minAbs=0.0);

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
        ret += cMagD(mat.m_data[cc]);
    return ret;
}

template<class T>
Svec<T>             cSumRows(MatV<T> const & m)
{
    Svec<T>             acc (m.numCols(),0);
    for (size_t rr=0; rr<m.numRows(); ++rr) {
        T const *           ptr = m.rowPtr(rr);
        for (size_t cc=0; cc<acc.size(); ++cc)
            acc[cc] += ptr[cc];
    }
    return acc;
}

template<class T>
inline Svec<T>      cMeanRow(MatV<T> const & m) {return cSumRows(m) / m.numRows(); }

template<class T>
MatV<T>             scaleCols(MatV<T> const & mat,Svec<T> const & vals)
{
    size_t              R = mat.numRows(),
                        C = mat.numCols();
    FGASSERT(vals.size() == C);
    auto                fn = [&,R,C]()
    {
        Svec<T>             ret; ret.reserve(R*C);
        for (size_t rr=0; rr<R; ++rr) {
            T const *           ptr = mat.rowPtr(rr);
            for (size_t cc=0; cc<C; ++cc)
                ret.push_back(ptr[cc] * vals[cc]);
        }
        return ret;
    };
    return {R,C,fn()};
}

template<class T>
MatV<T>             scaleRows(MatV<T> const & mat,Svec<T> const & vals)
{
    size_t              R = mat.numRows(),
                        C = mat.numCols();
    FGASSERT(vals.size() == R);
    auto                fn = [&,R,C]()
    {
        Svec<T>             ret; ret.reserve(R*C);
        for (size_t rr=0; rr<R; ++rr) {
            T const *           ptr = mat.rowPtr(rr);
            T                   val = vals[rr];
            for (size_t cc=0; cc<C; ++cc)
                ret.push_back(ptr[cc] * val);
        }
        return ret;
    };
    return {R,C,fn()};
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
MatD                cRandMatOrthogonal(size_t dim);
// Mahalanobis transform formed by D*R where R is random orthogonal, and
// D is a diagonal matrix of scales whose logarithms are distributed as N(0,logScaleStdev):
MatD                cRandMatMhlbs(size_t dim,double logScaleStdev);

inline size_t       cTriangular(size_t D) {return (D*(D+1))/2; }    // triangular numbers

template<class T>
Svec<T>             genTriangulars(size_t D,Sfun<T(size_t rr,size_t cc)> const & fn)
{
    Svec<T>             ret; ret.reserve(cTriangular(D));
    for (uint rr=0; rr<D; ++rr)
        for (uint cc=rr; cc<D; ++cc)
            ret.push_back(fn(rr,cc));
    return ret;
}

// Symmetric matrix. Useful for clarity, type safety, function overloading and avoiding state duplication:
template<class T>
struct          MatS
{
    size_t              dim;
    Svec<T>             data;   // stored in UT raster order
    FG_SER(dim,data)

    MatS() : dim{0} {}
    MatS(size_t D,T val) : dim{D}, data(cTriangular(D),val) {}     // const value ctor
    MatS(size_t D,Svec<T> const & d) : dim{D}, data{d} {FGASSERT(d.size() == cTriangular(D)); }
    explicit MatS(MatS3D const & m) : dim{3}, data{m.diag[0],m.offd[0],m.offd[1],m.diag[1],m.offd[2],m.diag[2]} {}
    explicit MatS(MatD const & m) : dim{m.numRows()}
    {
        FGASSERT(m.numCols() == dim);
        for (size_t rr=0; rr<dim; ++rr) {
            data.push_back(m.rc(rr,rr));
            for (size_t cc=rr+1; cc<dim; ++cc) {
                T                   v = m.rc(rr,cc);
                FGASSERT(m.rc(cc,rr) == v);
                data.push_back(v);
            }
        }
    }

    T const &           rc(size_t rr,size_t cc) const  {return data[rcToOffset(rr,cc)]; }
    T &                 rc(size_t rr,size_t cc) {return data[rcToOffset(rr,cc)]; }

    T const &           operator[](Arr2UI rc) const {return data[rcToOffset(rc[0],rc[1])]; }
    T &                 operator[](Arr2UI rc) {return data[rcToOffset(rc[0],rc[1])]; }
    // DO NOT USE, just for back compatibility, will be removed:
    T const &           operator[](size_t idx) const {return rc(idx/dim,idx%dim); }
    T &                 operator[](size_t idx) {return rc(idx/dim,idx%dim); }

    // apply to a vector. 'v' must point to 'dim' values.
    // much faster (4x) than using the rc() functions:
    Svec<T>             apply(T const * v) const
    {
        Svec<T>             ret; ret.reserve(dim);
        T const *           ptr = data.data();
        for (size_t rr=0; rr<dim; ++rr) {
            T                   acc {0};
            T const *           ptrM = data.data()+rr;
            size_t              step = dim-1;
            for (size_t cc=0; cc<rr; ++cc) {
                acc += *ptrM * v[cc];
                ptrM += step;
                --step;
            }
            for (size_t cc=rr; cc<dim; ++cc)
                acc += *ptr++ * v[cc];
            ret.push_back(acc);
        }
        return ret;
    };
    // surprisingly, this was slower than the above (MSVS22 x64)
    //Svec<T>             apply2(T const * v) const
    //{
    //    Svec<T>             ret (dim,0);
    //    T const *           ptr = data.data();
    //    for (size_t rr=0; rr<dim; ++rr) {
    //        ret[rr] += *ptr++ * v[rr];
    //        for (size_t cc=rr+1; cc<dim; ++cc) {
    //            T               d = *ptr++;
    //            ret[rr] += d * v[cc];
    //            ret[cc] += d * v[rr];
    //        }
    //    }
    //    return ret;
    //}
    Svec<T>             operator*(Svec<T> const & v) const
    {
        FGASSERT(v.size() == dim);
        return apply(v.data());
    }

    inline void         operator+=(MatS const & rhs) {data += rhs.data; }
    inline MatS         operator+(MatS const & rhs) const {return {dim,data+rhs.data}; }
    inline MatS         operator-(MatS const & rhs) const {return {dim,data-rhs.data}; }
    inline MatS         operator*(T fac) const {return {dim,data*fac}; }
    inline MatS         operator/(T fac) const {return {dim,data/fac}; }
    inline bool         operator==(MatS const & r) const {return (data == r.data); }

    MatV<T>             asMatV() const
    {
        size_t              D = dim;
        auto                fn = [&,D]() -> Svec<T>
        {
            Svec<T>             ret; ret.reserve(D*D);
            // looping through output may be more cache efficient than looping through input:
            T const *           ptr = &data[0];
            for (size_t rr=0; rr<D; ++rr) {
                T const *           ptrM = &data[0]+rr;     // first column mirrored data for this row
                size_t              step = D-1;             // step down to next row
                for (size_t cc=0; cc<rr; ++cc) {
                    ret.push_back(*ptrM);
                    ptrM += step;
                    --step;
                }
                for (size_t cc=rr; cc<D; ++cc)
                    ret.push_back(*ptr++);
            }
            return ret;
        };
        return {D,D,fn()};
    }

    size_t              rcToOffset(size_t rr,size_t cc) const   // slow, avoid if speed sensitive
    {
        FGASSERT((rr<dim) && (cc<dim));
        if (cc<rr) std::swap(rr,cc);
        return rr*dim + cc - cTriangular(rr);
    }

    Svec<T>             diagVals() const
    {
        Svec<T>             ret; ret.reserve(dim);
        size_t              idx {0},
                            step {dim};
        while (step>0) {
            ret.push_back(data[idx]);
            idx += step--;
        }
        return ret;
    }

    Svec<T>             offDiagVals() const
    {
        Svec<T>             ret; ret.reserve(cTriangular(dim-1));
        for (size_t rr=0; rr<dim; ++rr)
            for (size_t cc=rr+1; cc<dim; ++cc)
                ret.push_back(rc(rr,cc));
        return ret;
    }

    void                addToDiagonal(Svec<T> const & vs)
    {
        FGASSERT(vs.size() == dim);
        size_t              idx = 0,
                            step = dim;
        for (size_t ii=0; ii<dim; ++ii) {
            data[idx] += vs[ii];
            idx += step--;
        }
    }
};

typedef MatS<double>    MatSD;
typedef MatS<int>       MatSI;
typedef Svec<MatSD>     MatSDs;

template <class T>
std::ostream &      operator<<(std::ostream & os,MatS<T> const & M)
{
    std::ios::fmtflags  oldFlag = os.setf(std::ios::fixed | std::ios::showpos | std::ios::right);
    std::streamsize     oldPrec = os.precision(6);
    os << fgpush;
    // just print the LT with the remaining left blank for clarity:
    for (size_t rr=0; rr<M.dim; rr++) {
        os << fgnl;
        os << "[ ";
        for (size_t cc=0; cc<=rr; cc++)
            os << M.rc(rr,cc) << " ";
        for (size_t cc=rr+1; cc<M.dim; ++cc)
            os << "          ";
        os << "]";
    }
    os << fgpop;
    os.flags(oldFlag);
    os.precision(oldPrec);
    return os;
}

// S = Diag(vals)
template<typename T>
MatS<T>             cMatSDiag(Svec<T> const & diagVals)
{
    size_t              D = diagVals.size();
    auto                fn = [&,D]()
    {
        Svec<T>             ret; ret.reserve(cTriangular(D));
        for (size_t rr=0; rr<D; ++rr) {
            ret.push_back(diagVals[rr]);
            for (size_t cc=rr+1; cc<D; ++cc)
                ret.push_back(0);
        }
        return ret;
    };
    return MatS<T>{D,fn()};
}

// S = v * v^T = v^T * v
template<typename T>
MatS<T>             outerProductSelf(Svec<T> const & v)
{
    size_t              D = v.size();
    return {D,genTriangulars<T>(D,[&](size_t rr,size_t cc){return v[rr]*v[cc]; })};
}

// S = M * M^T
template<typename T>
MatS<T>             selfTransposeProduct(MatV<T> const & mat)
{
    size_t              R = mat.numRows();
    auto                fn = [&mat](size_t rr,size_t cc)
    {
        return multAccPtr(mat.rowPtr(rr),mat.rowPtr(cc),mat.numCols());
    };
    return {R,genTriangulars<T>(R,fn)};
}

// [_ij Q_ij * r_i * r_j ]
template<class T>
T                   cQuadForm(MatS<T> const & qf,T const * relPos)  // relPos must point to qf.dim values
{
    Svec<T>             tmp = qf.apply(relPos);
    T                   ret {0};
    for (size_t ii=0; ii<tmp.size(); ++ii)
        ret += tmp[ii] * relPos[ii];
    return ret;
}

// sum the application of a quadratic form to a series of relative positions as row vectors of a matrix:
template<class T>
T                   cQuadForms(MatS<T> const & qf,MatV<T> const & relPoss)
{
    FGASSERT(relPoss.numCols() == qf.dim);
    T                   ret {0};
    for (size_t ss=0; ss<relPoss.numRows(); ++ss)
        ret += cQuadForm(qf,relPoss.rowPtr(ss));
    return ret;
}

// matrix form:     S = M * D(d) * M^T
// subscript form:  S_ik = [_j M_ij * d_j * M_jk ]
template<typename T>
MatS<T>             selfDiagTransposeProduct(MatV<T> const & mat,Svec<T> const & diags)
{
    size_t              J = mat.numCols(),      // inner (contracted) index size
                        K = mat.numRows();      // outer (non-contracted) indices size
    FGASSERT(diags.size() == J);
    auto                fn = [&,J](size_t rr,size_t cc)
    {
        return multAccPtr(mat.rowPtr(rr),mat.rowPtr(cc),diags.data(),J);
    };
    return {K,genTriangulars<T>(K,fn)};
}

// symmetric matrix congruence transform: M * S * M^T
// subscript form: R_ad = [_bc M_ab * S_bc * M_dc ]
template<class T>
MatS<T>             selfSymmTransposeProduct(MatV<T> const & M,MatS<T> const & S)
{
    size_t              A = M.numRows(),
                        B = M.numCols();
    FGASSERT(S.dim == B);
    auto                fn = [&]()
    {
        Svec<T>             ret; ret.reserve(cTriangular(A));
        for (size_t aa=0; aa<A; ++aa) {
            for (size_t dd=aa; dd<A; ++dd) {
                T               acc {0};
                for (size_t bb=0; bb<B; ++bb)
                    for (size_t cc=0; cc<B; ++cc)
                        acc += M.rc(aa,bb) * S.rc(bb,cc) * M.rc(dd,cc);
                ret.push_back(acc);
            }
        }
        return ret;
    };
    return MatS<T>{A,fn()};
}

template<class T>
struct      SymmPart        // symmetric matrix partition
{
    MatS<T>         p00;
    MatS<T>         p11;
    MatV<T>         p01;    // p10 = p01^T
};

typedef SymmPart<double>    SymmPartD;

// Partition a symmetric matrix into 4, returned as {P00,P11,P01} where P10 = P01^T:
template<class T>
SymmPart<T>         cPartition(MatS<T> const & mat,size_t S0) // size of first partition
{
    FGASSERT((S0>0) && (S0<mat.dim));
    size_t              S1 = mat.dim-S0;
    auto                diag = [&](size_t bg,size_t sz)
    {
        return genTriangulars<T>(sz,[&](size_t rr,size_t cc){return mat.rc(bg+rr,bg+cc); });
    };
    auto                offd = [&,S0,S1]()
    {
        Svec<T>             ret; ret.reserve(S0*S1);
        for (size_t rr=0; rr<S0; ++rr)
            for (size_t cc=0; cc<S1; ++cc)
                ret.push_back(mat.rc(rr,S0+cc));
        return ret;
    };
    return {
        MatS<T>{S0,diag(0,S0)},
        MatS<T>{S1,diag(S0,S1)},
        MatV<T>{S0,S1,offd()}
    };
}

template<class T>
MatS<T>             catRect(MatS<T> const & ll,MatS<T> const & uu,MatV<T> const & lu)
{
    size_t              L = ll.dim,
                        U = uu.dim,
                        R = L+U;
    FGASSERT((lu.numRows()==L) && (lu.numCols()==U));
    auto                fn = [&,L]()
    {
        Svec<T>             ret; ret.reserve(cTriangular(R));
        size_t              cnt {0};
        for (size_t rr=0; rr<L; ++rr) {
            for (size_t cc=rr; cc<L; ++cc)
                ret.push_back(ll.data[cnt++]);
            for (size_t cc=0; cc<U; ++cc)
                ret.push_back(lu.rc(rr,cc));
        }
        cat_(ret,uu.data);
        return ret;
    };
    return MatS<T>{R,fn()};
}

// random real symmetric matrix with eigenvals distributed as N(0,eigvalStdev):
MatSD               cRandMatRsm(size_t dim,double eigvalStdev);
// random symmetric positive definite matrix with ln eigenvals distributed as N(0,lnEigvalStdev):
MatSD               cRandMatSpd(size_t dim,double lnEigvalStdev);

struct      IdxVal
{
    Arr2UI              rc;             // rc[1] > rc[0]
    double              val;            // != 0
    FG_SER(rc,val)

    IdxVal() {}
    IdxVal(Arr2UI r,double v) : rc{r}, val{v} {FGASSERT(r[1]>r[0]);}

    bool                operator==(Arr2UI a) const {return a==rc; }
};
typedef Svec<IdxVal>    IdxVals;

// NB: calculating the determinant of the below using Eigen's sparse Cholesky solver was *slower* than
// converting to dense and calculating the determinant, so no direct determinant function is provided:
struct      SparseRsm                   // sparse real symmetric matrix
{
    Doubles             diags;          // dense diagonal
    IdxVals             offds;          // sparse off-diagonals. Keep sorted by index (row then col) for speed.
    FG_SER(diags,offds)

    size_t              dim() const {return diags.size(); }
    size_t              numParams() const {return diags.size() + offds.size(); }
    Doubles             apply(double const * v) const;          // v must point to diags.size() values
    Doubles             operator*(Doubles const & v) const;
    MatSD               asMatS() const;
    void                addOffd(IdxVal iv){insertSorted_(offds,iv,[](IdxVal a,IdxVal b){return a.rc < b.rc; }); }
};
typedef Svec<SparseRsm> SparseRsms;

std::ostream &      operator<<(std::ostream &,SparseRsm const &);
double              cQuadForm(SparseRsm const & prec,double const * v);
double              cQuadForms(SparseRsm const & prec,MatD const & vs);     // coords are rows
SparseRsm           cRandSparseRsm(size_t D);       // diags exp{N}, ~50% sparsity

// Returns the U of the U^T * U Cholesky decomposition of a symmetric positive definite (SPD) matrix.
// Throws if matrix not PD. Potential loss of precision if matrix has high condition number.
MatUT2D             cCholesky(MatS2D spd);
MatUT3D             cCholesky(MatS3D const & spd);

struct      UTUDecomp
{
    MatUT3D         U;      // Upper triangular part of the U^T * U decomposition
    Vec3UI          p;      // Permutation map from input index to solution index
};

// Solve linear system of equations of the form Mx = b. Assumes matrix is well-conditioned:
Vec2D               solveLinear(Mat22D const & M,Vec2D const & b);
// Solve the Matrix equation Mx = b when M is full rank:
Vec2D               solveLinear(MatS2D M,Vec2D b);
// Solve the Matrix equation Mx = b when M is symmetric positive definite (this is not checked):
Vec3D               solveLinear(MatS3D SPD,Vec3D b);
// Solve the matrix equation Mx = b when M is any matrix.
// If M is singular, this returns a solution vector with one or more components equal to zero:
Vec3D               solveLinearRobust(Mat33D const & M,Vec3D const & b);
// M must be full rank:
Vec3D               solveLinear(Mat33D const & M,Vec3D const & b);
Vec4D               solveLinear(Mat44D const & M,Vec4D const & b);

// Compute only the eigenvalues (smallest to largest) of a real symmetrix matrix
// (about ~5x faster than computing the eigenvactors at the same time (cRsmEigs):
Doubles             cEigvalsRsm(MatSD const & rsm);
// positive definite matrix ln determinant preserving precision. Throws if matrix not PD.
double              cLnDeterminant(MatSD const & rsm);
// positive definite matrix determinant using LDLT decomposition. Throws if matrix is not PD.
// Note that ln determinant cannot be calculated preserving precision as the diagonal factors are not the eigenvalues:
double              cPdDeterminant(MatSD const & pdrsm);

// Eigenspectrum of a real symmetric matrix: RSM = vecs * diag(vals) * vecs^T
struct      RsmEigs
{
    Doubles             vals;                   // Eigenvalues, smallest to largest
    MatD                vecs;                   // Column vectors are the respective eigenvectors
    FG_SER(vals,vecs)

    inline MatSD        asMatS() const {return selfDiagTransposeProduct(vecs,vals); }
};
std::ostream & operator<<(std::ostream &,RsmEigs const &);

// Compute eigenvalues and eigenvectors of a real symmetric matrix in O(dim^3) time
// eigenvectors are returned in order of smallest to largest:
RsmEigs             cRsmEigs(MatSD const & rsm);
// inverts the matrix if the ratio of smallest to largest absolute eigenvalue is larger
// (exclusive lower bound) than the given limit. Throws otherwise:
MatSD               cInverse(MatSD const & rsm,double ratioELB=epsBits(30));

// solve a linear system with a symmetric definite (ie non-zero determinant) matrix.
// Throws if the matrix is ill conditioned:
Doubles             solveLinear(MatSD const & M,Doubles b);

// Real eigenvalues and eigenvectors of a real symmetric square const-size matrix:
template<size_t D>
struct      EigsRsmC
{
    Mat<double,D,1>         vals;   // Eigenvalues
    Mat<double,D,D>         vecs;   // Column vectors are the respective eigenvectors.
};
typedef EigsRsmC<3>         EigsRsm3;
typedef EigsRsmC<4>         EigsRsm4;

template<size_t D>
std::ostream &      operator<<(std::ostream & os,EigsRsmC<D> const & e)
{
    return os
        << fgnl << "Eigenvalues: " << e.vals.transpose()
        << fgnl << "Eigvenvector columns: " << e.vecs;
}

// Eigenvalues of a square real symmetric matrix, returned in order from smallest to largest eigval:
EigsRsm3            cRsmEigs(Mat33D const & rsm);
EigsRsm3            cRsmEigs(MatS3D const & rsm);
EigsRsm4            cRsmEigs(Mat44D const & rsm);

template<size_t D>
struct      EigsC
{
    Mat<std::complex<double>,D,1>       vals;
    Mat<std::complex<double>,D,D>       vecs;   // Column vectors are the respective eigenvectors.
};

template<size_t D>
std::ostream &      operator<<(std::ostream & os,const EigsC<D> & e)
{
    return os
        << fgnl << "Eigenvalues: " << e.vals.transpose()
        << fgnl << "Eigenvector columns: " << e.vecs;
}

// Eigensolver for arbitrary matrix, returned in arbitrary order since eigenvalues
// can be complex. The eigenvectors can always be made real when the associated eigevalue
// is real but do not default to a real representation:
EigsC<3>            cEigs(Mat33D const & mat);
EigsC<4>            cEigs(Mat44D const & mat);

}

#endif
