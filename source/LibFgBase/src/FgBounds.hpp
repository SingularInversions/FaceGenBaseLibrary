//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// min/max bounds of n-D data structures, and operations on bounds.
//
// * bounds matrices (or vectors) always have 2 columns: [min,max]
// * min values are always treated as inclusive
// * max values are documented as either inclusive or exclusive
//

#ifndef FGBOUNDS_HPP
#define FGBOUNDS_HPP

#include "FgStdLibs.hpp"
#include "FgMatrixC.hpp"
#include "FgMatrixV.hpp"

namespace Fg {

template<typename T>
inline void
setIfGreater(T & max,T val)
{max = (val > max) ? val : max; }

template<typename T>
inline void
setIfLess(T & min,T val)
{min = (val < min) ? val : min; }

template<typename T>
Mat<T,1,2>
cBounds(Svec<T> const & data)
{
    FGASSERT(data.size() > 0);
    Mat<T,1,2>    ret(data[0]);
    for (size_t ii=1; ii<data.size(); ++ii) {
        setIfLess   (ret[0],data[ii]);
        setIfGreater(ret[1],data[ii]);
    }
    return ret;
}

template<typename T,size_t S>
Mat<T,1,2>
cBounds(Arr<T,S> const & arr)
{
    Mat<T,1,2>          ret(arr[0]);
    for (size_t ii=1; ii<S; ++ii) {
        setIfLess   (ret[0],arr[ii]);
        setIfGreater(ret[0],arr[ii]);
    }
    return ret;
}

// Returns inclusive bounds of vectors
template<typename T,uint dim>
Mat<T,dim,2>
cBounds(const Svec<Mat<T,dim,1> > & vecs) // If empty, return [max,lowest]
{
    Mat<T,dim,2>      ret;
    for (uint dd=0; dd<dim; ++dd) {
        ret.rc(dd,0) = std::numeric_limits<T>::max();
        ret.rc(dd,1) = std::numeric_limits<T>::lowest();
    }
    for (Mat<T,dim,1> v : vecs) {
        for (uint dd=0; dd<dim; ++dd) {
            T           val = v[dd];
            setIfLess     (ret.rc(dd,0),val);
            setIfGreater  (ret.rc(dd,1),val);
        }
    }
    return ret;
}

// Returns combined bounds of two bounds (inclusive or exclusive):
template<typename T,uint dim>
Mat<T,dim,2>
cBoundsUnion(const Mat<T,dim,2> & b1,const Mat<T,dim,2> & b2)
{
    Mat<T,dim,2>     ret(b1);
    for (uint dd=0; dd<dim; ++dd) {
        setIfLess     (ret.cr(0,dd),b2.cr(0,dd));
        setIfGreater  (ret.cr(1,dd),b2.cr(1,dd));
    }
    return ret;
}

// Returns inclusive bounds of 3 column vectors:
template<typename T,uint dim>
Mat<T,dim,2>
cBounds(
    const Mat<T,dim,1> & v0,
    const Mat<T,dim,1> & v1,
    const Mat<T,dim,1> & v2)
{
    Mat<T,dim,2>  ret;
    for (uint dd=0; dd<dim; ++dd) {
        ret.cr(0,dd) = ret.cr(1,dd) = v0[dd];
        setIfLess(ret.cr(0,dd),v1[dd]);
        setIfLess(ret.cr(0,dd),v2[dd]);
        setIfGreater(ret.cr(1,dd),v1[dd]);
        setIfGreater(ret.cr(1,dd),v2[dd]);
    }
    return ret;
}

// If bounds is empty, returns [max,-max]
template<class T,uint dim>
Mat<T,dim,2>
cBoundsUnion(Svec<Mat<T,dim,2> > const & bounds)
{
    T                       max = std::numeric_limits<T>::max(),
                            min = std::numeric_limits<T>::lowest();
    Mat<T,dim,2>      ret;
    for (uint dd=0; dd<dim; ++dd) {
        ret.rc(dd,0) = max;
        ret.rc(dd,1) = min;
    }
    for (Mat<T,dim,2> bound : bounds) {
        for (uint dd=0; dd<dim; ++dd) {
            setIfLess(ret.rc(dd,0),bound.rc(dd,0));
            setIfGreater(ret.rc(dd,1),bound.rc(dd,1));
        }
    }
    return ret;
}

template<typename T,uint nrows,uint ncols>
Mat<T,nrows,1>
cMaxColumnwise(Mat<T,nrows,ncols> const & mat)
{
    Mat<T,nrows,1>    ret(mat.colVec(0));
    for (uint row=0; row<nrows; ++row)
        for (uint col=1; col<ncols; ++col)
            setIfGreater(ret[row],mat.cr(col,row));
    return ret;
}

template<typename T,uint nrows,uint ncols>
T
cMaxElem(Mat<T,nrows,ncols> const & mat)
{
    T           ret(mat[0]);
    size_t      sz = mat.size();
    for (size_t ii=1; ii<sz; ++ii)
        setIfGreater(ret,mat[ii]);
    return ret;
}

template<typename T,uint nrows,uint ncols>
T
cMinElem(Mat<T,nrows,ncols> const & mat)
{
    T           ret(mat[0]);
    size_t      sz = mat.size();
    for (size_t ii=1; ii<sz; ++ii)
        setIfLess(ret,mat[ii]);
    return ret;
}

template<typename T,uint nrows,uint ncols>
uint
cMaxIdx(Mat<T,nrows,ncols> const & mat)
{
    uint        idx(0);
    for (uint ii=1; ii<mat.numElems(); ++ii)
        if (mat[ii] > mat[idx])
            idx = ii;
    return idx;
}

template<typename T,uint nrows,uint ncols>
uint
cMinIdx(Mat<T,nrows,ncols> const & mat)
{
    uint        idx(0);
    for (uint ii=1; ii<mat.numElems(); ++ii)
        if (mat[ii] < mat[idx])
            idx = ii;
    return idx;
}

// Element-wise max:
template<class T,uint nrows,uint ncols>
Mat<T,nrows,ncols>
cMax(
    Mat<T,nrows,ncols> const & m1,
    Mat<T,nrows,ncols> const & m2)
{
    Mat<T,nrows,ncols>    ret;
    for (uint ii=0; ii<nrows*ncols; ++ii)
        ret[ii] = cMax(m1[ii],m2[ii]);
    return ret;
}

template<typename T>
inline T
cMaxElem(const MatV<T> & mat)
{return cMax(mat.dataVec()); }

template<typename T,uint nrows>
Mat<T,nrows,1>
cDims(const Svec<Mat<T,nrows,1> > & vec)
{
    Mat<T,nrows,2>    bounds = cBounds(vec);
    return (bounds.colVec(1)-bounds.colVec(0));
}

template<typename T,uint dim>
bool
boundsIntersect(
    const Mat<T,dim,2> &  bnds1,
    const Mat<T,dim,2> &  bnds2)
{
    Mat<T,dim,2>      tmp;
    for (uint dd=0; dd<dim; ++dd) {
        tmp.cr(0,dd) = std::max(bnds1.cr(0,dd),bnds2.cr(0,dd));
        tmp.cr(1,dd) = std::min(bnds1.cr(1,dd),bnds2.cr(1,dd));
        if (tmp.cr(0,dd) > tmp.cr(1,dd))
            return false;
    }
    return true;
}

template<typename T,uint dim>
bool
boundsIntersect(
    const Mat<T,dim,2> &  bnds1,
    const Mat<T,dim,2> &  bnds2,
    Mat<T,dim,2> &        retval)     // Not assigned if bounds do not intersect
{
    Mat<T,dim,2>      tmp;
    for (uint dd=0; dd<dim; ++dd) {
        tmp.cr(0,dd) = std::max(bnds1.cr(0,dd),bnds2.cr(0,dd));
        tmp.cr(1,dd) = std::min(bnds1.cr(1,dd),bnds2.cr(1,dd));
        if (tmp.cr(0,dd) > tmp.cr(1,dd))
            return false;
    }
    retval = tmp;
    return true;
}

// The returned bounds will have negative volume if the bounds do not intersect.
// Bounds must both be EUB or both be IUB:
template<typename T,uint dim>
Mat<T,dim,2>
cBoundsIntersection(
    const Mat<T,dim,2> &  b1,
    const Mat<T,dim,2> &  b2)
{
    Mat<T,dim,2>      ret;
    for (uint dd=0; dd<dim; ++dd) {
        ret.rc(dd,0) = cMax(b1.rc(dd,0),b2.rc(dd,0));
        ret.rc(dd,1) = cMin(b1.rc(dd,1),b2.rc(dd,1));
    }
    return ret;
}

template<typename T,uint dim>
bool
isInBounds(Mat<T,dim,2> const & boundsEub,Mat<T,dim,1> const & point)
{
    for (uint dd=0; dd<dim; ++dd) {
        if (point[dd] < boundsEub.cr(0,dd))         // inclusive lower bound
            return false;
        if (!(point[dd] < boundsEub.cr(1,dd)))      // exclusive upper bound
           return false;
    }
    return true;
}

template<typename T,uint dim>
bool
isInUpperBounds(Mat<uint,dim,1> exclusiveUpperBounds,Mat<T,dim,1> pnt)
{
    for (uint dd=0; dd<dim; ++dd) {
        if (pnt[dd] < 0)
            return false;
        // We can now safely cast T to uint since it's >= 0 (and one hopes smaller than 2Gig):
        if (!(uint(pnt[dd]) < exclusiveUpperBounds[dd]))
            return false;
    }
    return true;
}

// Returns inclusive bounds for the given [0,eub]^dim range:
template<uint dim>
Mat<uint,dim,2>
rangeToBounds(Mat<uint,dim,1> rangeEub)
{
    FGASSERT(cMinElem(rangeEub) > 0);
    return catHoriz(Mat<uint,dim,1>(0),rangeEub-Mat<uint,dim,1>(1));
}

// Returns true if (upper >= lower) for all dims:
template<typename T,uint dim>
bool
boundsValid(Mat<T,dim,2> bounds)
{
    bool        ret = true;
    for (uint dd=0; dd<dim; ++dd)
        ret = ret && (bounds.rc(dd,1)>=bounds.rc(dd,0));
    return ret;
}

// Return a cube bounding box around the given verts whose centre is the centre of the
// rectangular bounding box and whose dimension is that of the largest axis bounding dimension,
// optionally scaled by 'padRatio':
template<typename T,uint dim>
Mat<T,dim,2>  // First column is lower bound corner of cube, second is upper
cCubeBounds(const Svec<Mat<T,dim,1> > & verts,T padRatio=1)
{
    Mat<T,dim,2>        bounds = cBounds(verts);
    Mat<T,dim,1>        lo = bounds.colVec(0),
                        hi = bounds.colVec(1),
                        centre = (lo + hi) * T(0.5);
    T                   halfSize = cMaxElem(hi - lo) * 0.5f * padRatio;
    Mat<T,dim,1>        halfDim {halfSize};
    return catHoriz(centre-halfDim,centre+halfDim);
}

// Convert bounds from inclusive upper to exclusive upper:
template<class T,uint nrows>
Mat<T,nrows,2>
fgInclToExcl(Mat<T,nrows,2> boundsInclusiveUpper)
{
    Mat<T,nrows,2>    ret;
    for (uint rr=0; rr<nrows; ++rr) {
        ret.rc(rr,0) = boundsInclusiveUpper.rc(rr,0);
        ret.rc(rr,1) = boundsInclusiveUpper.rc(rr,1) + T(1);
    }
    return ret;
}

// Clamp (aka clip) functions below are all inclusive bounds since exclusive bounds
// do not explicitly provide the value to clip to:

template<typename T>
inline T
clampBounds(T val,T lo,T hi)
{return val < lo ? lo : (val > hi ? hi : val); }

template<typename T>
inline T
clampBounds(T val,Mat<T,1,2> bounds)
{return val < bounds[0] ? bounds[0] : (val > bounds[1] ? bounds[1] : val); }

// Base case for template specialization same as std::max:
template<typename T>
inline T
clampLo(T val,T lo)
{return (val < lo) ? lo : val; }

template<typename T>
inline T
clampHi(T val,T hi)
{return (val > hi) ? hi : val; }

template<class T,uint nrows,uint ncols>
Mat<T,nrows,ncols>
clampBounds(Mat<T,nrows,ncols> const & mat,T lo,T hi)
{
    Mat<T,nrows,ncols>      ret;
    for (uint ii=0; ii<nrows*ncols; ++ii)
        ret[ii] = clampBounds(mat[ii],lo,hi);
    return ret;
}

template<class T,uint nrows,uint ncols>
Mat<T,nrows,ncols>
clampBounds(Mat<T,nrows,ncols> const & mat,Mat<T,nrows,ncols> lo,Mat<T,nrows,ncols> hi)
{
    Mat<T,nrows,ncols>      ret;
    for (uint ii=0; ii<nrows*ncols; ++ii)
        ret[ii] = clampBounds(mat[ii],lo[ii],hi[ii]);
    return ret;
}

template<typename T,uint nrows,uint ncols>
Mat<T,nrows,ncols>
clampLo(Mat<T,nrows,ncols> m,T lo)
{
    Mat<T,nrows,ncols>    ret;
    for (uint ii=0; ii<nrows*ncols; ++ii)
        ret[ii] = clampLo(m[ii],lo);
    return ret;
}

template<typename T,uint nrows,uint ncols>
Mat<T,nrows,ncols>
clampHi(Mat<T,nrows,ncols> m,T hi)
{
    Mat<T,nrows,ncols>    ret;
    for (uint ii=0; ii<nrows*ncols; ++ii)
        ret[ii] = clampHi(m[ii],hi);
    return ret;
}

template<class T,uint dim>
Mat<T,dim,1>
clampBounds(const Mat<T,dim,1> & pos,const Mat<T,dim,2> & boundsInclusive)
{
    Mat<T,dim,1>  ret;
    for (uint ii=0; ii<dim; ++ii)
        ret[ii] = clampBounds(pos[ii],boundsInclusive.rc(ii,0),boundsInclusive.rc(ii,1));
    return ret;
}

// Clip to [0,EUBs) with change from signed to unsigned:
template<uint nrows,uint ncols>
Mat<uint,nrows,ncols>
clampZeroEub(Mat<int,nrows,ncols> mat,Mat<uint,nrows,1> exclusiveUpperBounds)
{
    Mat<uint,nrows,ncols>     ret;
    for (uint rr=0; rr<nrows; ++rr) {
        uint        eub = exclusiveUpperBounds[rr];
        FGASSERT(eub != 0);
        for (uint cc=0; cc<ncols; ++cc) {
            int         val = mat.rc(rr,cc);
            uint        valu = uint(val);
            ret.rc(rr,cc) = val < 0 ? 0U : (valu < eub ? valu : eub-1);
        }
    }
    return ret;
}

}

#endif
