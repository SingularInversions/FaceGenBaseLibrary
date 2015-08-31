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

template<class T>
FgMatrixV<T>
fgMatDiagBlockSum(const std::vector<FgMatrixV<T> > & blocks);

double
fgMatSumElems(const FgMatrixD & mat);

double
fgMatSumPrecise(const FgMatrixD & mat);     // Experimental, do not use.

template<class T>
T
fgDot(
    const FgMatrixV<T> &    lhs,
    const FgMatrixV<T> &    rhs)
{
    uint        nrows = lhs.numRows(),
                ncols = lhs.numCols(),
                nelems = nrows*ncols;
    FGASSERT((nrows == rhs.numRows()) && (ncols == rhs.numCols()));
    typename FgTraits<T>::Accumulator    tot = 0;
    for (uint ii=0; ii<nelems; ++ii)
        tot += lhs[ii] * rhs[ii];
    return T(tot);
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
    for (uint rr=0; rr<numRows; rr++)
    {
        uint    col=0;
        for (uint cc=0; cc<left.numCols(); ++cc)
            retval.elem(rr,col++) = left.elem(rr,cc);
        for (uint cc=0; cc<right.numCols(); ++cc)
            retval.elem(rr,col++) = right.elem(rr,cc);
    }
    return retval;
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
            retval.elem(row++,col) = upper.elem(rr,col);
    for (uint rr=0; rr<middle.numRows(); rr++)
        for (uint col=0; col<middle.numCols(); col++)
            retval.elem(row++,col) = middle.elem(rr,col);
    for (uint rr=0; rr<lower.numRows(); rr++)
        for (uint col=0; col<lower.numCols(); col++)
            retval.elem(row++,col) = lower.elem(rr,col);
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
            ret.elem(rr,cc) *= modVector[cc];
    return ret;
}

// Forms the "block sum" matrix by concatenating a series of square matrices as the block
// diagonal of a larger matrix, with off-diagonal blocks set to zero:
template<class T>
FgMatrixV<T>   fgMatDiagBlockSum(const std::vector<FgMatrixV<T> > &blocks)
{
    uint    dim = 0;
    for (uint bb=0; bb<blocks.size(); bb++)
    {
        FGASSERT(blocks[bb].numRows() == blocks[bb].numCols());
        dim += blocks[bb].numRows();
    }

    uint    ind = 0;
    FgMatrixV<T>   retval(dim,dim,T(0));
    for (uint bb=0; bb<blocks.size(); bb++)
    {
        const FgMatrixV<T> &block = blocks[bb];
        for (uint row=0; row<block.numRows(); row++)
            for (uint col=0; col<block.numCols(); col++)
                retval.elem(ind+row,ind+col) = block.elem(row,col);
        ind += block.numRows();
    }

    return retval;
}

template<class T>
FgMatrixV<T>
fgDiagonal(const FgMatrixV<T> & vec)
{
    FGASSERT((vec.numRows() == 1) || (vec.numCols() == 1));
    uint            dim = vec.numRows() * vec.numCols();
    FgMatrixV<T>    ret(dim,dim,T(0));
    for (uint ii=0; ii<dim; ++ii)
        ret.elem(ii,ii) = vec[ii];
    return ret;
}

template<class T>
FgMatrixV<T>
fgMatRandNormal(uint nrows,uint ncols)
{
    FgMatrixV<T>    ret(nrows,ncols);
    for (uint ii=0; ii<ret.numElems(); ++ii)
        ret[ii] = fgRandNormal();
    return ret;
}

template<class T>
FgMatrixV<T>
fgMatRandOrtho(uint dim)
{
    FGASSERT(dim > 1);
    FgMatrixV<T>    ret(dim,dim);
    for (uint row=0; row<dim; ++row)
    {
        FgMatrixV<T>    vec = fgMatRandNormal<T>(1,dim);
        for (uint rr=0; rr<row; ++rr)
        {
            FgMatrixV<T>    axis = ret.rowVector(rr);
            vec -=  axis * fgDot(vec,axis);
        }
        ret.setSubMatrix(row,0,vec * (1.0/vec.length()));
    }
    return ret;
}

template<class T>
FgMatrixV<T>
fgMatRandExp(uint nrows,uint ncols)
{
    FgMatrixV<T>    ret(nrows,ncols);
    for (uint ii=0; ii<ret.numElems(); ++ii)
        ret[ii] = fgRandExp();
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
            sum += fgSqr(upper.elem(row2,row));
        double      ref = mat.elem(row,row),
                    sqv = ref - sum;
        if (sqv <= 0.0) return false;
        if ((sqv / (ref+sum)) < (numeric_limits<double>::epsilon()*100.0))
            return false;
        upper.elem(row,row) = std::sqrt(sqv);
        for (uint col=row; col<dim; col++)
        {
            
        }
    }
}
*/

FgMatrixD
fgRelDiff(const FgMatrixD & a,const FgMatrixD & b);

#endif
