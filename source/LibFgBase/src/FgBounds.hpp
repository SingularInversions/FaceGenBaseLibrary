//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// min/max bounds of n-D data structures, and operations on bounds,
// for both fixed and floating point values.
//
// * Bounds matrices always have 2 columns: [min,max]
// * min values are always treated as inclusive
// * max values are documented as either inclusive upper bounds (IUB) or exclusive upper bounds (EUB)
//   where EUB means 1 greater than the highest value instance.
// * Floating point values should use IUB
// * Integer values should use EUB
//

#ifndef FGBOUNDS_HPP
#define FGBOUNDS_HPP

#include "FgSerial.hpp"
#include "FgMatrixC.hpp"
#include "FgMatrixV.hpp"

namespace Fg {

template<typename T>
inline void         updateMax_(T & maxVal,T nextVal) {maxVal = (nextVal > maxVal) ? nextVal : maxVal; }
template<typename T>
inline void         updateMin_(T & minVal,T nextVal) {minVal = (nextVal < minVal) ? nextVal : minVal; }

template<typename T,uint Dim,size_t D>      // type must match declaration ...
inline void         updateBounds_(Mat<T,Dim,2> & boundsIub,Arr<T,D> const & arr)
{
    static_assert(Dim==D,"template args don't match");
    for (uint ii=0; ii<Dim; ++ii) {
        updateMin_(boundsIub.rc(ii,0),arr[ii]);
        updateMax_(boundsIub.rc(ii,1),arr[ii]);
    }
}

template<typename T,uint Dim>
inline void         updateBounds_(Mat<T,Dim,2> & boundsIub,Mat<T,Dim,1> const & vec)
{
    for (uint ii=0; ii<Dim; ++ii) {
        updateMin_(boundsIub.rc(ii,0),vec[ii]);
        updateMax_(boundsIub.rc(ii,1),vec[ii]);
    }
}

// Returns IUB bounds of scalar array:
template<typename T,size_t S>
Mat<T,1,2>          cBounds(Arr<T,S> const & arr)
{
    Mat<T,1,2>          ret(arr[0]);
    for (size_t ii=1; ii<S; ++ii) {
        updateMin_(ret[0],arr[ii]);
        updateMax_(ret[1],arr[ii]);
    }
    return ret;
}

// Returns IUB bounds of scalar list:
template<typename T>
Mat<T,1,2>          cBounds(Svec<T> const & data)
{
    FGASSERT(data.size() > 0);
    Mat<T,1,2>    ret(data[0]);
    for (size_t ii=1; ii<data.size(); ++ii) {
        updateMin_(ret[0],data[ii]);
        updateMax_(ret[1],data[ii]);
    }
    return ret;
}

template<class T,size_t S>
Mat<T,S,2>          cBounds(Svec<Arr<T,S> > const & arrs) // If empty, returns [max,lowest]
{
    Mat<T,S,2>          ret = catHoriz(
        Mat<T,S,1>(std::numeric_limits<T>::max()),
        Mat<T,S,1>(std::numeric_limits<T>::lowest()));
    for (Arr<T,S> const & a : arrs)
        updateBounds_(ret,a);
    return ret;
}

// Returns IUB bounds of vector elements over list:
template<typename T,uint dim>
Mat<T,dim,2>        cBounds(Svec<Mat<T,dim,1> > const & vecs) // If empty, return [max,lowest]
{
    Mat<T,dim,2>      ret = catHoriz(
        Mat<T,dim,1>(std::numeric_limits<T>::max()),
        Mat<T,dim,1>(std::numeric_limits<T>::lowest()));
    for (Mat<T,dim,1> const & v : vecs)
        updateBounds_(ret,v);
    return ret;
}

// Returns IUB bounds of vector elements over list of 3:
template<typename T,uint dim>
Mat<T,dim,2>        cBounds(Mat<T,dim,1> const & v0,Mat<T,dim,1> const & v1,Mat<T,dim,1> const & v2)
{
    Mat<T,dim,2>        ret = catHoriz(v0,v0);
    updateBounds_(ret,v1);
    updateBounds_(ret,v2);
    return ret;
}

// Returns combined bounds of two bounds (inclusive or exclusive):
template<typename T,uint dim>
Mat<T,dim,2>        cBoundsUnion(Mat<T,dim,2> const & b1,Mat<T,dim,2> const & b2)
{
    Mat<T,dim,2>     ret(b1);
    for (uint dd=0; dd<dim; ++dd) {
        updateMin_(ret.cr(0,dd),b2.cr(0,dd));
        updateMax_(ret.cr(1,dd),b2.cr(1,dd));
    }
    return ret;
}

// If bounds is empty, returns [max,-max]
template<class T,uint dim>
Mat<T,dim,2>        cBoundsUnion(Svec<Mat<T,dim,2> > const & bounds)
{
    Mat<T,dim,2>      ret = catHoriz(
        Mat<T,dim,1>(std::numeric_limits<T>::max()),
        Mat<T,dim,1>(std::numeric_limits<T>::lowest()));
    for (Mat<T,dim,2> bound : bounds) {
        for (uint dd=0; dd<dim; ++dd) {
            updateMin_(ret.rc(dd,0),bound.rc(dd,0));
            updateMax_(ret.rc(dd,1),bound.rc(dd,1));
        }
    }
    return ret;
}

template<typename T,size_t S>
T                   cMedian(Arr<T,S> arr)       // Rounds up for even numbers of elements
{
    std::sort(arr.begin(),arr.end());
    return arr[S/2];
}

template<typename T,uint R,uint C>
inline T            cMaxElem(Mat<T,R,C> const & mat) {return cMax(mat.m); }

template<typename T,uint R,uint C>
inline T            cMinElem(Mat<T,R,C> const & mat) {return cMin(mat.m); }

template<typename T,uint R,uint C>
inline size_t       cMaxIdx(Mat<T,R,C> const & mat) {return cMaxIdx(mat.m); }

template<typename T,uint R,uint C>
inline size_t       cMinIdx(Mat<T,R,C> const & mat) {return cMinIdx(mat.m); }

// Element-wise max:
template<class T,uint R,uint C>
Mat<T,R,C>          cMax(Mat<T,R,C> const & m1,Mat<T,R,C> const & m2)
{
    Mat<T,R,C>    ret;
    for (uint ii=0; ii<R*C; ++ii)
        ret[ii] = cMax(m1[ii],m2[ii]);
    return ret;
}

template<typename T>
inline T            cMaxElem(MatV<T> const & mat) {return cMax(mat.m_data); }

template<typename T,uint R>
Mat<T,R,1>          cDims(const Svec<Mat<T,R,1> > & vec)
{
    Mat<T,R,2>          bounds = cBounds(vec);
    return (bounds.colVec(1)-bounds.colVec(0));
}

// The returned bounds will have negative volume if the bounds do not intersect.
// Bounds must both be EUB or both be IUB:
template<typename T,uint dim>
Mat<T,dim,2>        intersectBounds(Mat<T,dim,2> const &  b1,Mat<T,dim,2> const &  b2)
{
    Mat<T,dim,2>      ret;
    for (uint dd=0; dd<dim; ++dd) {
        ret.rc(dd,0) = cMax(b1.rc(dd,0),b2.rc(dd,0));
        ret.rc(dd,1) = cMin(b1.rc(dd,1),b2.rc(dd,1));
    }
    return ret;
}

template<typename T,uint dim>
bool                isInBounds(Mat<T,dim,2> const & boundsEub,Mat<T,dim,1> const & point)
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
bool                isInUpperBounds(Mat<uint,dim,1> exclusiveUpperBounds,Mat<T,dim,1> pnt)
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

// Returns EUB bounds for the given dimensions with implicit lower bound of 0:
template<typename T,uint dim>
Mat<T,dim,2>        dimsToBoundsEub(Mat<T,dim,1> rangeEub)
{
    return catHoriz(Mat<T,dim,1>{0},rangeEub);
}

// Returns IUB bounds for the given [0,eub]^dim range:
template<uint dim>
Mat<uint,dim,2>     dimsToBoundsIub(Mat<uint,dim,1> rangeEub)
{
    FGASSERT(cMinElem(rangeEub) > 0);
    return catHoriz(Mat<uint,dim,1>(0),rangeEub-Mat<uint,dim,1>(1));
}

template<typename T,uint dim>
bool                isBoundIubEmpty(Mat<T,dim,2> bounds)
{
    for (uint dd=0; dd<dim; ++dd)
        if (bounds.rc(dd,1) < bounds.rc(dd,0))
            return true;
    return false;
}

template<typename T,uint dim>
bool                isBoundEubEmpty(Mat<T,dim,2> bounds)
{
    for (uint dd=0; dd<dim; ++dd)
        if (!(bounds.rc(dd,0) < bounds.rc(dd,1)))
            return true;
    return false;
}

// Return a cube bounding box around the given verts whose centre is the centre of the
// rectangular bounding box and whose dimension is that of the largest axis bounding dimension,
// optionally scaled by 'padRatio':
template<typename T,uint dim>
// First column is lower bound corner of cube, second is upper:
Mat<T,dim,2>        cCubeBounds(const Svec<Mat<T,dim,1> > & verts,T padRatio=1)
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
template<class T,uint R>
Mat<T,R,2>      iubToEub(Mat<T,R,2> boundsInclusiveUpper)
{
    Mat<T,R,2>    ret;
    for (uint rr=0; rr<R; ++rr) {
        ret.rc(rr,0) = boundsInclusiveUpper.rc(rr,0);
        ret.rc(rr,1) = boundsInclusiveUpper.rc(rr,1) + T(1);
    }
    return ret;
}

// Clamp (aka clip) functions below are all inclusive bounds since exclusive bounds
// do not explicitly provide the value to clip to:

template<typename T>
inline T            clamp(T val,T lo,T hi) {return val < lo ? lo : (val > hi ? hi : val); }

template<typename T>
inline T            clamp(T val,Mat<T,1,2> bounds)
{
    return val < bounds[0] ? bounds[0] : (val > bounds[1] ? bounds[1] : val);
}

template<class T,uint R,uint C>
Mat<T,R,C>          mapClamp(Mat<T,R,C> const & mat,T lo,T hi)
{
    Mat<T,R,C>      ret;
    for (uint ii=0; ii<R*C; ++ii)
        ret[ii] = clamp(mat[ii],lo,hi);
    return ret;
}

template<class T,uint dim>
Mat<T,dim,1>        mapClamp(Mat<T,dim,1> const & pos,Mat<T,dim,2> const & boundsInclusive)
{
    Mat<T,dim,1>  ret;
    for (uint ii=0; ii<dim; ++ii)
        ret[ii] = clamp(pos[ii],boundsInclusive.rc(ii,0),boundsInclusive.rc(ii,1));
    return ret;
}

template<class T,uint R,uint C>
Mat<T,R,C>          mapClamp(Mat<T,R,C> const & mat,Mat<T,R,C> lo,Mat<T,R,C> hi)
{
    Mat<T,R,C>      ret;
    for (uint ii=0; ii<R*C; ++ii)
        ret[ii] = clamp(mat[ii],lo[ii],hi[ii]);
    return ret;
}

template<typename T,uint R,uint C>
Mat<T,R,C>          mapMax(Mat<T,R,C> m,T lo)
{
    Mat<T,R,C>    ret;
    for (uint ii=0; ii<R*C; ++ii)
        ret[ii] = cMax(m[ii],lo);
    return ret;
}

template<typename T,uint R,uint C>
Mat<T,R,C>          mapMin(Mat<T,R,C> m,T hi)
{
    Mat<T,R,C>    ret;
    for (uint ii=0; ii<R*C; ++ii)
        ret[ii] = cMin(m[ii],hi);
    return ret;
}

// Clip to [0,EUBs) with change from signed to unsigned:
template<uint R,uint C>
Mat<uint,R,C>       clampZeroEub(Mat<int,R,C> mat,Mat<uint,R,1> exclusiveUpperBounds)
{
    Mat<uint,R,C>     ret;
    for (uint rr=0; rr<R; ++rr) {
        uint        eub = exclusiveUpperBounds[rr];
        FGASSERT(eub != 0);
        for (uint cc=0; cc<C; ++cc) {
            int         val = mat.rc(rr,cc);
            uint        valu = uint(val);
            ret.rc(rr,cc) = val < 0 ? 0U : (valu < eub ? valu : eub-1);
        }
    }
    return ret;
}

}

#endif
