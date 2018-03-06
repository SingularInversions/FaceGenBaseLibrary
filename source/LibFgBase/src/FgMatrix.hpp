//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Feb 9, 2009
//

#ifndef FGMATRIX_HPP
#define FGMATRIX_HPP

#include "FgMatrixC.hpp"
#include "FgMatrixV.hpp"

typedef FgMatrixV<FgVect3D>     FgMatV3D;

template<class Matrix>
std::ostream &
fgMatOstream(std::ostream& ss,const Matrix & mm)
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

template <class T,uint nrows,uint ncols>
std::ostream& operator<<(std::ostream& ss,const FgMatrixC<T,nrows,ncols> & mm)
{return fgMatOstream(ss,mm); }

template <class T>
std::ostream& operator<<(std::ostream& ss,const FgMatrixV<T> & mm)
{return fgMatOstream(ss,mm); }

template<class T,uint nrows,uint ncols>
FgMatrixV<T>
fgMatrixV(const FgMatrixC<T,nrows,ncols> & m)
{return FgMatrixV<T>(nrows,ncols,&m[0]); }

// vector of Matrix functions:

template<class Xform,class T,uint nrows,uint ncols>
std::vector<FgMatrixC<T,nrows,ncols> >
operator*(
    const Xform &                                   xform,
    const std::vector<FgMatrixC<T,nrows,ncols> > &  rhs)
{
    std::vector<FgMatrixC<T,nrows,ncols> >  ret(rhs.size());
    fgTransform_(rhs,ret,xform);
    return ret;
}

template<typename T,uint nrows,uint ncols>
inline T
fgLength(const std::vector<FgMatrixC<T,nrows,ncols> > & v)
{return std::sqrt(fgMag(v)); }

template<typename T,uint nrows,uint ncols>
FgMatrixC<T,nrows,ncols>
fgVariance(
    const std::vector<FgMatrixC<T,nrows,ncols> > & v,
    const FgMatrixC<T,nrows,ncols> & mean)
{
    FgMatrixC<T,nrows,ncols>    var(0);
    for (size_t ii=0; ii<v.size(); ++ii)
        var += fgSquare(v[ii]-mean);
    return (var / T(v.size()));
}

template<typename T,uint nrows,uint ncols>
inline FgMatrixC<T,nrows,ncols>
fgVariance(const std::vector<FgMatrixC<T,nrows,ncols> > & v)
{return fgVariance(v,fgMean(v)); }

template<class T>
typename FgTraits<T>::Scalar
fgRmsd(const vector<T> & a,const vector<T> & b)
{
    FGASSERT(a.size() == b.size());
    typename FgTraits<T>::Scalar   acc(0);
    for (size_t ii=0; ii<a.size(); ++ii)
        acc += fgMag(a[ii]-b[ii]);
    return std::sqrt(acc / a.size());
}

// Partition a square matrix into 4 matrices symmetrically:
FgMatrixC<FgMatrixD,2,2>
fgPartition(const FgMatrixD & m,size_t loSize);

// Join 4 matrices into a single matrix (oppposite of partition):
FgMatrixD
fgJoin(
    const FgMatrixD &   ll,     // ncols == hl.ncols, nrows == lh.nrows
    const FgMatrixD &   lh,
    const FgMatrixD &   hl,
    const FgMatrixD &   hh);    // ncols == lh.ncols, nrows == hl.nrows

#endif
