//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Sept. 28, 2009
//
// min/max bounds of n-D data structures, and operations on bounds.
//
// * bounds matrices (or vectors) always have 2 columns: [min,max]
// * min values are always treated as inclusive
// * max values are documented as either inclusive or exclusive

#ifndef FGBOUNDS_HPP
#define FGBOUNDS_HPP

#include "FgStdLibs.hpp"
#include "FgMatrix.hpp"

template<typename T>
inline void
fgSetIfGreater(T & max,T val)
{max = (val > max) ? val : max; }

template<typename T>
inline void
fgSetIfLess(T & min,T val)
{min = (val < min) ? val : min; }

template<typename T>
FgMatrixC<T,1,2>
fgBounds(const std::vector<T> & data)
{
    FGASSERT(data.size() > 0);
    FgMatrixC<T,1,2>    ret(data[0]);
    for (size_t ii=1; ii<data.size(); ++ii)
    {
        fgSetIfLess     (ret[0],data[ii]);
        fgSetIfGreater  (ret[1],data[ii]);
    }
    return ret;
}

// Returns inclusive bounds of vectors:
template<typename T,uint dim>
FgMatrixC<T,dim,2>
fgBounds(const std::vector<FgMatrixC<T,dim,1> > & data)
{
    FGASSERT(data.size() > 0);
    FgMatrixC<T,dim,2>     ret;
    ret.setSubMat(data[0],0,0);
    ret.setSubMat(data[0],0,1);
    for (size_t ii=1; ii<data.size(); ++ii) {
        for (uint dd=0; dd<dim; ++dd) {
            fgSetIfLess     (ret.cr(0,dd),data[ii][dd]);
            fgSetIfGreater  (ret.cr(1,dd),data[ii][dd]);
        }
    }
    return ret;
}

// Returns combined bounds of two bounds (inclusive or exclusive):
template<typename T,uint dim>
FgMatrixC<T,dim,2>
fgBounds(const FgMatrixC<T,dim,2> & b1,const FgMatrixC<T,dim,2> & b2)
{
    FgMatrixC<T,dim,2>     ret(b1);
    for (uint dd=0; dd<dim; ++dd) {
        fgSetIfLess     (ret.cr(0,dd),b2.cr(0,dd));
        fgSetIfGreater  (ret.cr(1,dd),b2.cr(1,dd));
    }
    return ret;
}

// Returns inclusive bounds of all elements of the given matrix:
template<typename T,uint nrows,uint ncols>
FgMatrixC<T,1,2>
fgBounds(const FgMatrixC<T,nrows,ncols> & mat)
{
    FGASSERT(mat.numElems() > 0);
    FgMatrixC<T,1,2>    ret(mat[0]);
    for (uint ii=0; ii<mat.numElems(); ++ii) {
        fgSetIfLess     (ret[0],mat[ii]);
        fgSetIfGreater  (ret[1],mat[ii]);
    }
    return ret;
}

// Returns inclusive bounds of all elements of 'mat':
template<typename T>
inline FgMatrixC<T,1,2>
fgBounds(const FgMatrixV<T> & mat)
{return fgBounds(mat.dataVec()); }

// Returns inclusive bounds of 3 column vectors:
template<typename T,uint dim>
FgMatrixC<T,dim,2>
fgBounds(
    const FgMatrixC<T,dim,1> & v0,
    const FgMatrixC<T,dim,1> & v1,
    const FgMatrixC<T,dim,1> & v2)
{
    FgMatrixC<T,dim,2>  ret;
    for (uint dd=0; dd<dim; ++dd) {
        ret.cr(0,dd) = ret.cr(1,dd) = v0[dd];
        fgSetIfLess(ret.cr(0,dd),v1[dd]);
        fgSetIfLess(ret.cr(0,dd),v2[dd]);
        fgSetIfGreater(ret.cr(1,dd),v1[dd]);
        fgSetIfGreater(ret.cr(1,dd),v2[dd]);
    }
    return ret;
}

template<typename T,uint nrows,uint ncols>
FgMatrixC<T,nrows,1>
fgMaxColwise(const FgMatrixC<T,nrows,ncols> & mat)
{
    FG_STATIC_ASSERT(ncols > 1);
    FgMatrixC<T,nrows,1>    ret(mat.colVec(0));
    for (uint row=0; row<nrows; ++row)
        for (uint col=1; col<ncols; ++col)
            fgSetIfGreater(ret[row],mat.cr(col,row));
    return ret;
}

template<typename T,uint nrows,uint ncols>
T
fgMaxElem(const FgMatrixC<T,nrows,ncols> & mat)
{
    T           ret(mat[0]);
    size_t      sz = mat.size();
    for (size_t ii=1; ii<sz; ++ii)
        fgSetIfGreater(ret,mat[ii]);
    return ret;
}

template<typename T,uint nrows,uint ncols>
T
fgMinElem(const FgMatrixC<T,nrows,ncols> & mat)
{
    T           ret(mat[0]);
    size_t      sz = mat.size();
    for (size_t ii=1; ii<sz; ++ii)
        fgSetIfLess(ret,mat[ii]);
    return ret;
}

template<typename T,uint nrows,uint ncols>
uint
fgMaxIdx(const FgMatrixC<T,nrows,ncols> & mat)
{
    uint        idx(0);
    T           max(mat[0]);
    uint        sz = mat.numElems();
    for (uint ii=1; ii<sz; ++ii) {
        if (mat[ii] > max) {
            max = mat[ii];
            idx = ii;
        }
    }
    return idx;
}

template<typename T,uint nrows,uint ncols>
uint
fgMinIdx(const FgMatrixC<T,nrows,ncols> & mat)
{
    uint        idx(0);
    T           min(mat[0]);
    uint        sz = mat.numElems();
    for (uint ii=1; ii<sz; ++ii) {
        if (mat[ii] < min) {
            min = mat[ii];
            idx = ii;
        }
    }
    return idx;
}

// Element-wise max:
template<class T,uint nrows,uint ncols>
FgMatrixC<T,nrows,ncols>
fgMax(
    const FgMatrixC<T,nrows,ncols> & m1,
    const FgMatrixC<T,nrows,ncols> & m2)
{
    FgMatrixC<T,nrows,ncols>    ret;
    for (uint ii=0; ii<nrows*ncols; ++ii)
        ret[ii] = fgMax(m1[ii],m2[ii]);
    return ret;
}

template<typename T>
inline T
fgMaxElem(const FgMatrixV<T> & mat)
{return fgMax(mat.dataVec()); }

template<typename T,uint nrows>
FgMatrixC<T,nrows,1>
fgDims(const std::vector<FgMatrixC<T,nrows,1> > & vec)
{
    FgMatrixC<T,nrows,2>    bounds = fgBounds(vec);
    return (bounds.colVec(1)-bounds.colVec(0));
}

template<typename T,uint dim>
bool
fgBoundsIntersect(
    const FgMatrixC<T,dim,2> &  bnds1,
    const FgMatrixC<T,dim,2> &  bnds2,
    FgMatrixC<T,dim,2> &        retval)     // Not assigned if bounds do not intersect
{
    FgMatrixC<T,dim,2>      tmp;
    for (uint dd=0; dd<dim; ++dd) {
        tmp.cr(0,dd) = std::max(bnds1.cr(0,dd),bnds2.cr(0,dd));
        tmp.cr(1,dd) = std::min(bnds1.cr(1,dd),bnds2.cr(1,dd));
        if (tmp.cr(0,dd) > tmp.cr(1,dd))
            return false;
    }
    retval = tmp;
    return true;
}

// The returned bounds will have negative area if the bounds do not intersect:
template<typename T,uint dim>
FgMatrixC<T,dim,2>
fgBoundsIntersection(
    const FgMatrixC<T,dim,2> &  b1,
    const FgMatrixC<T,dim,2> &  b2)
{
    FgMatrixC<T,dim,2>      ret(b1);
    for (uint dd=0; dd<dim; ++dd) {
        fgSetIfGreater(ret.cr(0,dd),b2.cr(0,dd));
        fgSetIfLess(ret.cr(1,dd),b2.cr(1,dd));
    }
    return ret;
}

template<typename T,uint dim>
bool
fgBoundsIncludes(
    const FgMatrixC<T,dim,2> &  inclusiveBounds,
    const FgMatrixC<T,dim,1> &  point)
{
    for (uint dd=0; dd<dim; ++dd) {
        if ((inclusiveBounds.cr(1,dd) < point[dd]) ||
            (inclusiveBounds.cr(0,dd) > point[dd]))
           return false;
    }
    return true;
}

template<typename T,uint dim>
bool
fgBoundsIncludes(FgMatrixC<uint,dim,1> dims,FgMatrixC<T,dim,1> pnt)
{
    for (uint dd=0; dd<dim; ++dd) {
        if (pnt[dd] < T(0))
            return false;
        if (uint(pnt[dd]) >= T(dims[dd]))
            return false;
    }
    return true;
}

template<uint dim>
FgMatrixC<uint,dim,2>
fgRangeToBounds(FgMatrixC<uint,dim,1> range)
{
    FGASSERT(fgMinElem(range) > 0);
    return fgConcatHoriz(FgMatrixC<uint,dim,1>(0),range-FgMatrixC<uint,dim,1>(1));
}

template<typename T,uint dim>
FgMatrixC<T,dim,1>
fgBoundsCentre(const std::vector<FgMatrixC<T,dim,1> > & verts)
{
    FgMatrixC<T,dim,2>  bounds = fgBounds(verts);
    return (bounds.colVector[0] + bounds.colVec(1)) * 0.5;
}

template<typename T,uint dim>
FgMatrixC<T,dim,2>
fgCubeBounds(const vector<FgMatrixC<T,dim,1> > & verts,T padRatio=1)
{
    FgMatrixC<T,dim,2>  ret;
    FgMatrixC<T,dim,2>  bounds = fgBounds(verts);
    FgMatrixC<T,dim,1>  lo = bounds.colVec(0),
                        hi = bounds.colVec(1),
                        centre = (lo + hi) * T(0.5);
    T                   hsize = fgMaxElem(hi - lo) * 0.5f * padRatio;
    ret = fgConcatHoriz(centre-FgMatrixC<T,dim,1>(hsize),centre+FgMatrixC<T,dim,1>(hsize));
    return ret;
}

template<typename T>
T
fgClip(T val,T inclusiveLowerBound,T inclusiveUpperBound)
{
    if (val < inclusiveLowerBound)
        return inclusiveLowerBound;
    if (val > inclusiveUpperBound)
        return inclusiveUpperBound;
    return val;
}

template<class T,uint nrows>
FgMatrixC<T,nrows,1>
fgClipElems(const FgMatrixC<T,nrows,1> & vals,T minInclusive,FgMatrixC<T,nrows,1> maxInclusive)
{
    FgMatrixC<T,nrows,1>    ret(vals);
    for (uint ii=0; ii<nrows; ++ii)
        ret[ii] = (vals[ii] < minInclusive ? minInclusive : (vals[ii] > maxInclusive[ii] ? maxInclusive[ii] : ret[ii]));
    return ret;
}

template<class T,uint nrows>
FgMatrixC<T,nrows,1>
fgClipElems(const FgMatrixC<T,nrows,1> & vals,T minInclusive,T maxInclusive)
{
    FgMatrixC<T,nrows,1>    ret(vals);
    for (uint ii=0; ii<nrows; ++ii)
        ret[ii] = (vals[ii] < minInclusive ? minInclusive : (vals[ii] > maxInclusive ? maxInclusive : ret[ii]));
    return ret;
}

template<class T,uint dim>
FgMatrixC<T,dim,1>
fgClipVol(const FgMatrixC<T,dim,1> & pos,const FgMatrixC<T,dim,2> & volBoundsInclusive)
{
    FgMatrixC<T,dim,1>  ret;
    for (uint ii=0; ii<dim; ++ii)
        ret[ii] = fgClip(pos[ii],volBoundsInclusive.rc(ii,0),volBoundsInclusive.rc(ii,1));
    return ret;
}

template<uint dim>
FgMatrixC<uint,dim,1>
fgClipVolHi(FgMatrixC<int,dim,1> coord,FgMatrixC<uint,dim,1> exclusiveUpperBounds)
{
    FgMatrixC<uint,dim,1>   ret;
    for (uint dd=0; dd<dim; ++dd)
        ret[dd] = uint(fgClip(coord[dd],0,int(exclusiveUpperBounds[dd]-1)));
    return ret;
}

template<typename T>
T
fgClipLo(T val,T inclusiveLowerBound)
{return (val < inclusiveLowerBound) ? inclusiveLowerBound : val; }

template<typename T,uint nrows,uint ncols>
FgMatrixC<T,nrows,ncols>
fgClipLo(FgMatrixC<T,nrows,ncols> m,T inclusiveLowerBound)
{
    FgMatrixC<T,nrows,ncols>    ret(m);
    for (uint ii=0; ii<nrows*ncols; ++ii)
        fgClipLo(ret.m[ii],inclusiveLowerBound);
    return ret;
}

// Convert bounds from inclusive upper to exclusive upper:
template<class T,uint nrows>
FgMatrixC<T,nrows,2>
fgInclToExcl(FgMatrixC<T,nrows,2> boundsInclusiveUpper)
{
    FgMatrixC<T,nrows,2>    ret;
    for (uint rr=0; rr<nrows; ++rr) {
        ret.rc(rr,0) = boundsInclusiveUpper.rc(rr,0);
        ret.rc(rr,1) = boundsInclusiveUpper.rc(rr,1) + T(1);
    }
    return ret;
}

#endif
