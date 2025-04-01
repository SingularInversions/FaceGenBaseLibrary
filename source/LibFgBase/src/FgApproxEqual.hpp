//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGAPPROXEQUAL_HPP
#define FGAPPROXEQUAL_HPP

#include "FgBounds.hpp"

namespace Fg {

// Simpler to test for larger value than cast to larger signed type defined by some type trait,
// which doesn't work for uint64 in any case:
template<class T,FG_ENABLE_IF(T,is_unsigned)>
inline bool         isApproxEqual(T l,T r,T maxDiff)
{
    T               diff = (l>r) ? (l-r) : (r-l);
    return (diff <= maxDiff);
}

template<class T,FG_ENABLE_IF(T,is_integral),FG_ENABLE_IF(T,is_signed)>
inline bool         isApproxEqual(T l,T r,T maxDiff)
{
    T               diff = (l>r) ? (l-r) : (r-l);       // ensure it's positive so we can
    return ((diff >= 0) && (diff <= maxDiff));          // test for overflow
}

template<class T,FG_ENABLE_IF(T,is_floating_point)>
inline bool         isApproxEqual(T l,T r,T maxDiff)
{
    // must check for valid FP values since INF and NAN will just return true for all comparisons (UB):
    FGASSERT(std::isfinite(l) && std::isfinite(r));
    return (std::abs(l-r) <= maxDiff);
}

// forward declare to handle Arr<Svec<...>>
template<class T> bool isApproxEqual(Svec<T> const & l,Svec<T> const & r,typename Traits<T>::Scalar maxDiff);

template<class T,size_t S>
bool                isApproxEqual(Arr<T,S> const & l,Arr<T,S> const & r,typename Traits<T>::Scalar maxDiff)
{
    for (size_t ii=0; ii<S; ++ii)
        if (!isApproxEqual(l[ii],r[ii],maxDiff))
            return false;
    return true;
}

template<class T>
bool                isApproxEqual(Svec<T> const & l,Svec<T> const & r,typename Traits<T>::Scalar maxDiff)
{
    FGASSERT(l.size() == r.size());
    for (size_t ii=0; ii<l.size(); ++ii)
        if (!isApproxEqual(l[ii],r[ii],maxDiff))
            return false;
    return true;
}

template<class T,size_t R,size_t C>
inline bool         isApproxEqual(Mat<T,R,C> const & l,Mat<T,R,C> const & r,T maxDiff)
{
    return isApproxEqual(l.m,r.m,maxDiff);
}

template<class T>
bool                isApproxEqual(MatV<T> const & l,MatV<T> const & r,T maxDiff)
{
    FGASSERT(l.dims() == r.dims());
    return isApproxEqual(l.m_data,r.m_data,maxDiff);
}

template<class T>
inline bool         isApproxEqual(MatS<T> const & l,MatS<T> const & r,T tol)
{
    FGASSERT(l.dim == r.dim);
    return isApproxEqual(l.data,r.data,tol);
}

template<class T> uint constexpr defPrecBits();
template<> uint constexpr defPrecBits<float>() {return 18;}
template<> uint constexpr defPrecBits<double>() {return 30;}

// express the tolerance relative to the absolute sizes of the values:
template<class T,FG_ENABLE_IF(T,is_floating_point)>
bool                isApproxEqualRel(T l,T r,double maxRelDiff)
{
    return isApproxEqual(l,r,maxRelDiff*0.5*(std::abs(l)+std::abs(r)));
}

template<class T,FG_ENABLE_IF(T,is_floating_point)>
bool                isApproxEqualPrec(T v0,T v1,size_t precBits=defPrecBits<T>())
{
    return isApproxEqualRel(v0,v1,epsBits(precBits));
}

template<class T>
bool                isApproxEqualPrec(Svec<T> const & l,Svec<T> const & r,size_t precBits=defPrecBits<T>())
{
    T                   scale = cMaxElem(mapAbs(l)) + cMaxElem(mapAbs(r));
    return isApproxEqual(l,r,epsBits(precBits+1)*scale);
}

template<class T,size_t S>
bool                isApproxEqualPrec(Arr<T,S> const & l,Arr<T,S> const & r,size_t precBits=defPrecBits<T>())
{
    T                   scale = cMaxElem(mapAbs(l))+cMaxElem(mapAbs(r));
    return isApproxEqual(l,r,epsBits(precBits+1)*scale);
}

template<class T>
bool                isApproxEqualPrec(MatV<T> const & l,MatV<T> const & r,size_t precBits=defPrecBits<T>())
{
    FGASSERT(l.dims() == r.dims());
    return isApproxEqualPrec(l.m_data,r.m_data,precBits);
}

template<class T,size_t R,size_t C>
inline bool         isApproxEqualPrec(Mat<T,R,C> const & l,Mat<T,R,C> const & r,size_t precBits=defPrecBits<T>())
{
    return isApproxEqualPrec(l.m,r.m,precBits);
}

template<class T,size_t nrows,size_t ncols>
bool                isApproxEqualPrec(
    Svec<Mat<T,nrows,ncols> > const &   lhs,
    Svec<Mat<T,nrows,ncols> > const &   rhs,
    size_t                              precBits=defPrecBits<T>())
{
    FGASSERT(lhs.size() == rhs.size());
    T                   scale = cMaxElem((mapAbs(cDims(lhs)) + mapAbs(cDims(rhs))) * T(0.5)),
                        precision = epsBits(precBits),
                        maxDiff = scale * precision;
    for (size_t ii=0; ii<lhs.size(); ++ii)
        if (!isApproxEqual(lhs[ii],rhs[ii],maxDiff))
            return false;
    return true;
}

}

#endif
