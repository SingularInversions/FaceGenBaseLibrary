//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Nov 6, 2006
//
// Global functions related to variable size matrices.

#ifndef FGMATRIXV_HPP
#define FGMATRIXV_HPP

#include "FgMatrixVBase.hpp"
#include "FgAlgs.hpp"
#include "FgRandom.hpp"

// Useful in place of constructor since no template specification is required:
template<class T>
FgMatrixV<T>
fgMatrix(uint nrows,uint ncols,T v0)
{return FgMatrixV<T>(nrows,ncols,v0); }

template<class T>
FgMatrixV<T>
fgMatrix(uint nrows,uint ncols,T v0,T v1)
{return FgMatrixV<T>(nrows,ncols,fgSvec(v0,v1)); }

template<class T>
FgMatrixV<T>
fgMatrix(uint nrows,uint ncols,T v0,T v1,T v2)
{return FgMatrixV<T>(nrows,ncols,fgSvec(v0,v1,v2)); }

template<class T>
FgMatrixV<T>
fgMatrix(uint nrows,uint ncols,T v0,T v1,T v2,T v3)
{return FgMatrixV<T>(nrows,ncols,fgSvec(v0,v1,v2,v3)); }

template<class T>
FgMatrixV<T>
fgMatrix(uint nrows,uint ncols,T v0,T v1,T v2,T v3,T v4,T v5)
{return FgMatrixV<T>(nrows,ncols,fgSvec(v0,v1,v2,v3,v4,v5)); }

template<class T>
FgMatrixV<T>
fgIdentity(size_t dim)
{
    FgMatrixV<T>    ret(dim,dim,T(0));
    for (size_t ii=0; ii<dim; ++ii)
        ret.rc(ii,ii) = T(1);
    return ret;
}

template<class T>
FgMatrixV<T>
fgVectHoriz(T v0,T v1)
{return FgMatrixV<T>(1,2,fgSvec(v0,v1)); }

template<class T>
FgMatrixV<T>
fgVectHoriz(T v0,T v1,T v2)
{return FgMatrixV<T>(1,3,fgSvec(v0,v1,v2)); }

template<class T>
FgMatrixV<T>
fgVectHoriz(const std::vector<T> & v)
{return FgMatrixV<T>(1,v.size(),v); }

template<class T>
FgMatrixV<T>
fgVectVert(T v0,T v1)
{return FgMatrixV<T>(2,1,fgSvec(v0,v1)); }

template<class T>
FgMatrixV<T>
fgVectVert(T v0,T v1,T v2)
{return FgMatrixV<T>(3,1,fgSvec(v0,v1,v2)); }

template<class T>
FgMatrixV<T>
fgVectVert(const std::vector<T> & v)
{return FgMatrixV<T>(v.size(),1,v); }

FgMatrixD &
operator/=(
    FgMatrixD & mat,
    double      div);

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

template <class T>
FgMatrixV<T>
fgConcatHoriz(const std::vector<FgMatrixV<T> > & ms)
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
fgConcatHoriz(
    const FgMatrixV<T> &    left,
    const FgMatrixV<T> &    right)
{
    if (left.empty())
        return right;
    if (right.empty())
        return left;
    FGASSERT(left.numRows() == right.numRows());
    uint            numRows = left.numRows(),
                    numCols = left.numCols() + right.numCols();
    FgMatrixV<T>    retval(numRows,numCols);
    for (uint rr=0; rr<numRows; rr++) {
        uint    col=0;
        for (uint cc=0; cc<left.numCols(); ++cc)
            retval.rc(rr,col++) = left.rc(rr,cc);
        for (uint cc=0; cc<right.numCols(); ++cc)
            retval.rc(rr,col++) = right.rc(rr,cc);
    }
    return retval;
}

template <class T>
FgMatrixV<T>
fgConcatVert(const std::vector<FgMatrixV<T> > & ms)
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
fgConcatVert(
    const FgMatrixV<T> &    upper,
    const FgMatrixV<T> &    lower)
{
    if (upper.empty())
        return lower;
    if (lower.empty())
        return upper;
    FGASSERT(upper.numCols() == lower.numCols());
    FgMatrixV<T>      ret(upper.numRows()+lower.numRows(),upper.numCols());
    for (uint ii=0; ii<upper.numElems(); ++ii)
        ret[ii] = upper[ii];
    uint    off = upper.numElems();
    for (uint ii=0; ii<lower.numElems(); ++ii)
        ret[off+ii] = lower[ii];
    return ret;
}

template <class T>
FgMatrixV<T>
fgConcatVert(
    const FgMatrixV<T> &    upper,
    const FgMatrixV<T> &    middle,
    const FgMatrixV<T> &    lower)
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

// Block matrix formation
template<class T>
FgMatrixV<T>
fgBlock(const FgMatrixV<FgMatrixV<T> > & ms)
{
    FgMatrixV<T>        ret;
    FgUints             nrows(ms.nrows),
                        ncols(ms.ncols);
    for (uint rr=0; rr<ms.nrows; ++rr)
        nrows[rr] = ms.rc(rr,0).nrows;
    for (uint cc=0; cc<ms.ncols; ++cc)
        ncols[cc] = ms.rc(0,cc).ncols;
    ret.resize(fgSum(nrows),fgSum(ncols));
    uint                row = 0;
    for (uint rr=0; rr<ms.nrows; ++rr) {
        uint            col = 0;
        for (uint cc=0; cc<ms.nrows; ++cc) {
            const FgMatrixV<T> & mat = ms.rc(rr,cc);
            FGASSERT(mat.nrows == nrows[rr]);
            FGASSERT(mat.ncols == ncols[cc]);
            ret.setSubMat(row,col,mat);
            col += ncols[cc];
        }
        row += nrows[rr];
    }
    return ret;
}

// 2x2 block matrix formation
template<class T>
FgMatrixV<T>
fgBlock(const FgMatrixV<T> & ul,const FgMatrixV<T> & ur,const FgMatrixV<T> & ll,const FgMatrixV<T> & lr)
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

// Off-diagonal blocks are all set to zero. Blocks are not required to be square.
template<class T>
FgMatrixV<T>
fgBlockDiagonal(const std::vector<FgMatrixV<T> > & blocks)
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
fgBlockDiagonal(const FgMatrixV<T> & b0,const FgMatrixV<T> & b1)
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
fgBlockDiagonal(const FgMatrixV<T> & b0,const FgMatrixV<T> & b1,const FgMatrixV<T> & b2)
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
fgMatRandNormal(size_t nrows,size_t ncols)
{
    FgMatrixV<T>    ret(nrows,ncols);
    for (size_t ii=0; ii<ret.numElems(); ++ii)
        ret[ii] = fgRandNormal();
    return ret;
}

template<class T>
FgMatrixV<T>
fgNormalize(const FgMatrixV<T> & m)
{return m * (1/std::sqrt(m.mag())); }

template<class T>
FgMatrixV<T>
fgMatRandOrtho(size_t dim)
{
    FGASSERT(dim > 1);
    FgMatrixV<T>    ret(dim,dim);
    for (uint row=0; row<dim; ++row) {
        FgMatrixV<T>    vec = fgMatRandNormal<T>(1,dim);
        for (uint rr=0; rr<row; ++rr) {
            FgMatrixV<T>    axis = ret.rowVec(rr);
            vec -=  axis * fgDot(vec,axis);
        }
        ret.setSubMat(row,0,fgNormalize(vec));
    }
    return ret;
}

/*
// Outputs the U^T * U Cholesky decomposition of a symmetric positive definite matrix
// The template can be any type of matrix so in future we can use symmetric-specific data
// structure matrices and index-reverse-order matrix adapters.
// NOTES: symmetry of the input matrix is assumed, not checked.
// RETURNS: true if successful, false if the matrix was not positive definite.
template<class Matrix>
bool    fgCholesky(const FgMatrixD& mat)
{
    uint    dim = mat.numRows();
    FGASSERT(dim > 0);
    FGASSERT(dim == mat.numCols());
    Matrix  upper(dim,dim);
    for (uint row=0; row<dim; row++)
    {
        double      sum = 0.0;
        for (uint row2=0; row2<row; row2++)
            sum += fgSqr(upper.rc(row2,row));
        double      ref = mat.rc(row,row),
                    sqv = ref - sum;
        if (sqv <= 0.0) return false;
        if ((sqv / (ref+sum)) < (numeric_limits<double>::epsilon()*100.0))
            return false;
        upper.rc(row,row) = std::sqrt(sqv);
        for (uint col=row; col<dim; col++)
        {
            
        }
    }
}
*/

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

#endif
