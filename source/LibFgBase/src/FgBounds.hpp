//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// min/max vector axial bounds, and operations on bounds, for both fixed and floating point values.
//
// * bounds matrices have 2 columns: [min,max]
// * min values are always treated as inclusive
// * floating point max values are always treated as inclusive
// * integer max values are either inclusive upper bounds (IUB) or exclusive upper bounds (EUB)
//

#ifndef FGBOUNDS_HPP
#define FGBOUNDS_HPP

#include "FgMatrixV.hpp"

namespace Fg {

template<class T>
struct      ValRange
{
    T                   loPos;      // low end of range (high end if size is negative)
    T                   size;       // size of range. Can be -ve. (other end of range implicitly = low + size)
};
typedef ValRange<float>        RangeF;
typedef ValRange<double>       RangeD;

template<class T,size_t D>
struct      PosSize
{
    Mat<T,D,1>          loPos;      // lower-valued coordinate corner position
    T                   size;       // typically > 0
    FG_SER2(loPos,size);
};
typedef PosSize<float,2>    SquareF;
typedef PosSize<double,2>   SquareD;

template<class T,class U,size_t D>
PosSize<T,D>        mapCast(PosSize<U,D> ps) {return {mapCast<T>(ps.loPos),scast<T>(ps.size)}; }

template<class T,size_t D>
struct      Rect
{
    Mat<T,D,1>          loPos;      // Lower-valued coordinate corner position
    Mat<T,D,1>          dims;       // typically > 0
    FG_SER2(loPos,dims)

    Rect() {}
    Rect(Mat<T,D,1> l,Mat<T,D,1> d) : loPos{l}, dims{d} {}
    explicit Rect(Mat<T,D,2> bounds) : loPos{bounds.colVec(0)}, dims{bounds.colVec(1)-bounds.colVec(0)} {}

    inline T            volume() const {return dims.elemsProduct(); }
    Mat<T,D,1>          hiPos() const {return loPos + dims; }
    Mat<T,D,1>          centre() const {return loPos + dims / 2; }
    ValRange<T>         asRange(size_t dd) const {return {loPos[dd],dims[dd]}; }
};
typedef Rect<int,2>     Rect2I;
typedef Rect<float,2>   Rect2F;
typedef Rect<double,2>  Rect2D;
typedef Rect<double,3>  Rect3D;

template<typename T>
inline void         updateMax_(T & maxVal,T nextVal) {maxVal = (nextVal > maxVal) ? nextVal : maxVal; }
template<typename T>
inline void         updateMin_(T & minVal,T nextVal) {minVal = (nextVal < minVal) ? nextVal : minVal; }

// the bounds measure is area in 2D, volume in 3D, etc. Negative values result from invalid bounds:
template<typename T,size_t D>
T                   boundsMeasure(Mat<T,D,2> boundsEub)
{
    T                   ret {1};
    for (size_t dd=0; dd<D; ++dd)
        ret *= (boundsEub.rc(dd,1)-boundsEub.rc(dd,0));
    return ret;
}

template<typename T,size_t D>
inline void         updateBounds_(Mat<T,D,2> & boundsIub,Arr<T,D> const & arr)
{
    for (size_t ii=0; ii<D; ++ii) {
        updateMin_(boundsIub.rc(ii,0),arr[ii]);
        updateMax_(boundsIub.rc(ii,1),arr[ii]);
    }
}

template<typename T,size_t D>
inline void         updateBounds_(Mat<T,D,2> & boundsIub,Mat<T,D,1> const & vec)
{
    for (size_t ii=0; ii<D; ++ii) {
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
// bounds of array of arrays:
template<typename T,size_t S,size_t R>
Mat<T,R,2>          cBounds(Arr<Mat<T,R,1>,S> const & arr)
{
    Mat<T,R,2>          ret = catHoriz(arr[0],arr[0]);
    for (size_t ss=1; ss<S; ++ss) {
        for (size_t rr=0; rr<R; ++rr) {
            updateMin_(ret.rc(rr,0),arr[ss][rr]);
            updateMax_(ret.rc(rr,1),arr[ss][rr]);
        }
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
template<typename T,size_t dim>
Mat<T,dim,2>        cBounds(Svec<Mat<T,dim,1> > const & vecs) // If empty, returns [max,lowest]
{
    Mat<T,dim,2>      ret = catHoriz(
        Mat<T,dim,1>(std::numeric_limits<T>::max()),
        Mat<T,dim,1>(std::numeric_limits<T>::lowest()));
    for (Mat<T,dim,1> const & v : vecs)
        updateBounds_(ret,v);
    return ret;
}

// Returns IUB bounds of vector elements over list of 3:
template<typename T,size_t dim>
Mat<T,dim,2>        cBounds(Mat<T,dim,1> const & v0,Mat<T,dim,1> const & v1,Mat<T,dim,1> const & v2)
{
    Mat<T,dim,2>        ret = catHoriz(v0,v0);
    updateBounds_(ret,v1);
    updateBounds_(ret,v2);
    return ret;
}

// Returns combined bounds of two bounds (inclusive or exclusive):
template<typename T,size_t dim>
Mat<T,dim,2>        cBoundsUnion(Mat<T,dim,2> const & b1,Mat<T,dim,2> const & b2)
{
    Mat<T,dim,2>     ret(b1);
    for (size_t dd=0; dd<dim; ++dd) {
        updateMin_(ret.rc(dd,0),b2.rc(dd,0));
        updateMax_(ret.rc(dd,1),b2.rc(dd,1));
    }
    return ret;
}

// If bounds is empty, returns [max,-max]
template<class T,size_t dim>
Mat<T,dim,2>        cBoundsUnion(Svec<Mat<T,dim,2> > const & bounds)
{
    Mat<T,dim,2>      ret = catHoriz(
        Mat<T,dim,1>(std::numeric_limits<T>::max()),
        Mat<T,dim,1>(std::numeric_limits<T>::lowest()));
    for (Mat<T,dim,2> bound : bounds) {
        for (size_t dd=0; dd<dim; ++dd) {
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

template<typename T,size_t R,size_t C>
inline T            cMaxElem(Mat<T,R,C> const & mat) {return cMaxElem(mat.m); }

template<typename T,size_t R,size_t C>
inline T            cMinElem(Mat<T,R,C> const & mat) {return cMinElem(mat.m); }

template<typename T,size_t R,size_t C>
inline size_t       cMaxIdx(Mat<T,R,C> const & mat) {return cMaxIdx(mat.m); }

template<typename T,size_t R,size_t C>
inline size_t       cMinIdx(Mat<T,R,C> const & mat) {return cMinIdx(mat.m); }

template<typename T>
inline T            cMaxElem(MatV<T> const & mat) {return cMaxElem(mat.m_data); }

template<typename T,size_t R,size_t C>
T                   cMaxElem(Svec<Mat<T,R,C>> const & v)
{
    T               ret {lims<T>::min()};
    for (Mat<T,R,C> const & a : v)
        for (T e : a.m)
            updateMax_(ret,e);
    return ret;
}

template<typename T,size_t S>
T                   cMaxElmMaxElm(Svec<Arr<T,S>> const & va)
{
    T                   ret = lims<T>::lowest();
    for (Arr<T,S> const & arr : va)
        for (T elm : arr)
            updateMax_(ret,elm);
    return ret;
}

// calculate the rectangular dimensions (size) of the axial-aligned bounding box of the given points:
template<typename T,size_t R>
Mat<T,R,1>          cDims(Svec<Mat<T,R,1>> const & pts)
{
    return cBounds(pts) * Mat<T,2,1>{-1,1};
}

// The returned bounds will have negative volume if the bounds do not intersect.
// Bounds must both be EUB or both be IUB:
template<typename T,size_t D>
Mat<T,D,2>        intersectBounds(Mat<T,D,2> const &  b1,Mat<T,D,2> const &  b2)
{
    Mat<T,D,2>      ret;
    for (size_t dd=0; dd<D; ++dd) {
        ret.rc(dd,0) = cMax(b1.rc(dd,0),b2.rc(dd,0));
        ret.rc(dd,1) = cMin(b1.rc(dd,1),b2.rc(dd,1));
    }
    return ret;
}

// add a border to the bounds (can also shrink if 'border' is negative):
template<class T,size_t D>
Mat<T,D,2>          expandBounds(Mat<T,D,2> bnds,T border)
{
    for (size_t rr=0; rr<D; ++rr) {
        bnds.rc(rr,0) -= border;
        bnds.rc(rr,1) += border;
    }
    return bnds;
}

template<typename T,size_t D>
bool                isInBounds(Mat<T,D,2> const & boundsEub,Mat<T,D,1> const & point)
{
    for (size_t dd=0; dd<D; ++dd) {
        if (point[dd] < boundsEub.rc(dd,0))         // inclusive lower bound
            return false;
        if (!(point[dd] < boundsEub.rc(dd,1)))      // exclusive upper bound
           return false;
    }
    return true;
}

template<typename T,size_t D>
bool                isInUpperBounds(Mat<uint,D,1> exclusiveUpperBounds,Mat<T,D,1> pnt)
{
    for (size_t dd=0; dd<D; ++dd) {
        if (pnt[dd] < 0)
            return false;
        // We can now safely cast T to uint since it's >= 0 (and one hopes smaller than 2Gig):
        if (!(uint(pnt[dd]) < exclusiveUpperBounds[dd]))
            return false;
    }
    return true;
}

// Returns IUB bounds for the given [0,eub]^D range:
template<size_t D>
Mat<uint,D,2>     dimsToBoundsIub(Mat<uint,D,1> rangeEub)
{
    FGASSERT(cMinElem(rangeEub) > 0);
    return catHoriz(Mat<uint,D,1>(0),rangeEub-Mat<uint,D,1>(1));
}

template<typename T,size_t D>
bool                isBoundIubEmpty(Mat<T,D,2> bounds)
{
    for (size_t dd=0; dd<D; ++dd)
        if (bounds.rc(dd,1) < bounds.rc(dd,0))
            return true;
    return false;
}

template<typename T,size_t D>
bool                isBoundEubEmpty(Mat<T,D,2> bounds)
{
    for (size_t dd=0; dd<D; ++dd)
        if (!(bounds.rc(dd,0) < bounds.rc(dd,1)))
            return true;
    return false;
}

// Return a cube bounding box around the given verts whose centre is the centre of the
// rectangular bounding box and whose dimension is that of the largest axis bounding dimension,
// optionally scaled by 'padRatio':
template<typename T,size_t D>
// First column is lower bound corner of cube, second is upper:
Mat<T,D,2>        cCubeBounds(const Svec<Mat<T,D,1> > & verts,T padRatio=1)
{
    Mat<T,D,2>        bounds = cBounds(verts);
    Mat<T,D,1>        centre = bounds * Mat<T,2,1>{0.5,0.5},
                        dims = bounds * Mat<T,2,1>{-1,1};
    T                   halfSize = cMaxElem(dims) * 0.5f * padRatio;
    Mat<T,D,1>        halfDim {halfSize};
    return catHoriz(centre-halfDim,centre+halfDim);
}

// Convert bounds from inclusive upper to exclusive upper:
template<class T,size_t R>
Mat<T,R,2>      iubToEub(Mat<T,R,2> boundsInclusiveUpper)
{
    Mat<T,R,2>          ret;
    for (size_t rr=0; rr<R; ++rr) {
        ret.rc(rr,0) = boundsInclusiveUpper.rc(rr,0);
        ret.rc(rr,1) = boundsInclusiveUpper.rc(rr,1) + T(1);
    }
    return ret;
}

// Clamp (aka clip) is a two-sided threshold.
// Clamp bounds are inclusive as they are threshold values.
// Clamp limits are NOT checked for hi > lo.

// override std::clamp so that we can overload in same namespace to avoid lookup problems:
template<typename T>
inline T            clamp(T val,T lo,T hi) {return val < lo ? lo : (val > hi ? hi : val); }
template<typename T>
inline T            clamp(T val,Mat<T,1,2> bounds) {return clamp(val,bounds[0],bounds[1]); }

template<class T,size_t S>
Arr<T,S>            mapClamp(Arr<T,S> const & a,T l,T h){return mapCall(a,[l,h](T v){return clamp<T>(v,l,h); }); }

template<class T,size_t R,size_t C>
Mat<T,R,C>          mapClamp(Mat<T,R,C> const & mat,T lo,T hi){return Mat<T,R,C>{mapClamp(mat.m,lo,hi)}; }

// clamp each matrix row to respective lo/hi row values:
template<class T,size_t R,size_t C>
Mat<T,R,C>          mapClamp(Mat<T,R,C> mat,Mat<T,R,1> const & lo,Mat<T,R,1> const & hi)
{
    for (size_t rr=0; rr<R; ++rr)
        for (size_t cc=0; cc<C; ++cc)
            mat.rc(rr,cc) = clamp(mat.rc(rr,cc),lo[rr],hi[rr]);
    return mat;
}
template<class T,size_t R,size_t C>
Mat<T,R,C>          mapClamp(Mat<T,R,C> const & mat,Mat<T,R,2> const & bounds)
{
    return mapClamp(mat,bounds.colVec(0),bounds.colVec(1));
}

// Common special case where we convert float bounds to integer and clamp to image EUBs.
// Input bounds are not checked. Type cast bounds are not checked.
template<class T,size_t R>
Mat<uint,R,2>       intersectBoundsToImage(Mat<T,R,2> const & bounds,Mat<uint,R,1> eubs)
{
    Mat<uint,R,2>       ret;
    for (size_t rr=0; rr<R; ++rr) {
        int                 ilb = scast<int>(std::floor(bounds.rc(rr,0))),
                            eub = scast<int>(std::floor(bounds.rc(rr,1))) + 1;
        uint                ilbu = scast<uint>(cMax(ilb,0)),
                            eubu = scast<uint>(cMax(eub,0));
        ret.rc(rr,0) = ilbu;
        ret.rc(rr,1) = cMin(eubu,eubs[rr]);
    }
    return ret;
}

// Clamp to [0,EUBs) with change from signed to unsigned:
template<size_t R,size_t C>
Mat<uint,R,C>       clampZeroEub(Mat<int,R,C> mat,Mat<uint,R,1> exclusiveUpperBounds)
{
    Mat<uint,R,C>     ret;
    for (size_t rr=0; rr<R; ++rr) {
        uint        eub = exclusiveUpperBounds[rr];
        FGASSERT(eub != 0);
        for (size_t cc=0; cc<C; ++cc) {
            int         val = mat.rc(rr,cc);
            uint        valu = uint(val);
            ret.rc(rr,cc) = val < 0 ? 0U : (valu < eub ? valu : eub-1);
        }
    }
    return ret;
}

template<class T,size_t D>
Rect<T,D>           cBoundsRect(Svec<Mat<T,D,1>> const & pts)
{
    Mat<T,D,2>          bounds = cBounds(pts);
    return {
        bounds.colVec(0),
        bounds * Mat<T,D,1>{-1,1}
    };
}

// simple optimized 2D iterator for most common case of size_t counter:
class       Iter2
{
    Arr2Z               m_ilb {0},          // inclusive lower bounds
                        m_eub,              // exclusive upper bounds
                        m_idx {0};          // current index values

public:
    explicit Iter2(size_t eub) : m_eub{eub} {}
    explicit Iter2(Arr2UI eub) : m_eub{mapCast<size_t>(eub)} {}
    explicit Iter2(Arr2UL eub) : m_eub{mapCast<size_t>(eub)} {}
    explicit Iter2(Mat22UI boundsEub) :     // allows construction of an empty range; no iterations will be done
        m_ilb{mapCast<size_t>(boundsEub.colVec(0).m)},
        m_eub{mapCast<size_t>(boundsEub.colVec(1).m)}
    {
        m_idx = m_ilb;
    }
    Iter2(size_t eubX,size_t eubY) : m_eub{eubX,eubY} {}

    // m_idx components can never be less than m_ilb so we don't check for that:
    bool                valid() const {return ((m_idx[0]<m_eub[0]) && (m_idx[1]<m_eub[1])); }
    // must be in valid() state before this is called. Returns whether it's in valid state afterward.
    // 0'th index incremented first, carries to 1st index.
    bool                next()
    {
        ++m_idx[0];
        if (m_idx[0] < m_eub[0])
            return true;
        m_idx[0] = m_ilb[0];
        ++m_idx[1];
        return (m_idx[1]<m_eub[1]);
    }
    Arr2Z               operator()() const {return m_idx; }
    size_t              x() const {return m_idx[0]; }
    size_t              y() const {return m_idx[1]; }
};

// general D-dimensional case using any type counter and with variable strides:
template<typename T,size_t D>
struct      Iter
{
    static_assert(std::is_integral<T>::value,"Iter only supports integral types");
    Mat<T,D,1>          m_bndsLoIncl;   // Inclusive lower bounds
    Mat<T,D,1>          m_bndsHiExcl;   // Exclusive upper bounds
    // Strides (Step sizes) for each dimension. MUST NOT be larger than difference between bounds:
    Mat<T,D,1>          m_strides;
    Mat<T,D,1>          m_idx;
    bool                m_inBounds=true;    // Not redundant since m_idx wraps around.

    explicit Iter(Mat<T,D,2> boundsEub) :
        m_bndsLoIncl(boundsEub.colVec(0)),
        m_bndsHiExcl(boundsEub.colVec(1)),
        m_strides(Mat<T,D,1>(1)),
        m_idx(boundsEub.colVec(0))
    {setValid(); }
    Iter(Mat<T,D,1> lowerBounds,Mat<T,D,1> exclusiveUpperBounds) :
        m_bndsLoIncl(lowerBounds),
        m_bndsHiExcl(exclusiveUpperBounds),
        m_strides(Mat<T,D,1>(1)),
        m_idx(lowerBounds)
    {setValid(); }
    // Can't have last argument as optional on above since old versions of gcc have a bug
    // with non-type template arguments in default arguments:
    Iter(Mat<T,D,1> ilbs,Mat<T,D,1> eubs,Mat<T,D,1> strides) :
        m_bndsLoIncl(ilbs),
        m_bndsHiExcl(eubs),
        m_strides(strides),
        m_idx(ilbs)
    {setValid(); }
    // Lower bounds implicitly zero, strides implicitly 1:
    explicit Iter(Mat<T,D,1> dims)
    :   m_bndsLoIncl(0),
        m_bndsHiExcl(dims),
        m_strides(1),
        m_idx(0)
    {setValid(); }
    // Lower bounds implicitly zero, upper bounds all the same, strides 1:
    explicit Iter(T eub) :  m_bndsLoIncl(0), m_bndsHiExcl(eub), m_strides(1), m_idx(0) {setValid(); }

    bool                next()
    {
        FGASSERT(m_inBounds);
        for (size_t dd=0; dd<D; ++dd) {
            m_idx[dd] += m_strides[dd];
            if (m_idx[dd] >= m_bndsHiExcl[dd])
                m_idx[dd] = m_bndsLoIncl[dd] + m_idx[dd] % m_strides[dd];
            else
                return true;
        }
        m_inBounds = false;
        return false;
    }

    bool                valid() const {return m_inBounds; }
    Mat<T,D,1> const & operator()() const {return m_idx; }
    // Mirror the point around the centre in all axes:
    Mat<T,D,1>          mirror() const {return (m_bndsHiExcl - m_idx - Mat<T,D,1>(1)); }
    // Mirror the point around the centre in the given axis:
    Mat<T,D,1>          mirror(uint axis) const
    {
        Mat<T,D,1>      ret = m_idx;
        ret[axis] = m_bndsHiExcl[axis] - m_idx[axis] - 1;
        return ret;
    }
    Mat<T,D,1>        delta() const {return m_idx - m_bndsLoIncl; }
    Mat<T,D,2>        inclusiveRange() const {return catHoriz(m_bndsLoIncl,m_bndsHiExcl - Mat<T,D,1>(1)); }
    Mat<T,D,1>        dims() const {return (m_bndsHiExcl - m_bndsLoIncl); }
    // Return clipped bounds of all supremum norm (L_inf) distance = 1 neighbours:
    Mat<T,D,2>        neighbourBounds() const
    {
        return 
            Mat<T,D,2>(
                intersectBounds(
                    Mat<int,D,1>(m_idx) * Mat<int,1,2>(1) +
                        Mat<int,D,2>(-1,1,-1,1,-1,1),
                    Mat<int,D,2>(inclusiveRange())));
    }

private:
    // Return false if range is empty or negative (invalid):
    bool                validRange()
    {
        for (size_t dd=0; dd<D; dd++)
            if (m_bndsHiExcl[dd] <= m_bndsLoIncl[dd])
                return false;
        return true;
    }
    void                setValid()
    {
        FGASSERT(validRange());
        FGASSERT(m_strides.elemsProduct() > 0);
        m_inBounds = true;
        for (size_t ii=0; ii<D; ++ii)
            if (!(m_idx[ii] < m_bndsHiExcl[ii]))
                m_inBounds = false;
    }
};

template<typename T,size_t D>
std::ostream &      operator<<(std::ostream & os,const Iter<T,D> & it)
{
    return os
        << "lo: " << it.m_bndsLoIncl
        << " hi: " << it.m_bndsHiExcl
        << " stride: " << it.m_strides
        << " idx: " << it.m_idx
        << " valid: " << it.m_inBounds;
}

typedef Iter<int,2>         Iter2I;
typedef Iter<uint,2>        Iter2UI;
typedef Iter<uint,3>        Iter3UI;

}

#endif
