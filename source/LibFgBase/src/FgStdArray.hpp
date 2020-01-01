//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGSTDARRAY_HPP
#define FGSTDARRAY_HPP

#include "FgStdLibs.hpp"
#include "FgTypes.hpp"

namespace Fg {

template<typename T,size_t S>
using Arr = std::array<T,S>;

typedef Arr<uchar,2>        Arr2UC;
typedef Arr<uchar,3>        Arr3UC;
typedef Arr<uchar,4>        Arr4UC;

typedef Arr<schar,3>        Arr3SC;

typedef Arr<int,2>          Arr2I;
typedef Arr<int,3>          Arr3I;
typedef Arr<int,4>          Arr4I;

typedef Arr<float,2>        Arr2F;
typedef Arr<float,3>        Arr3F;
typedef Arr<float,4>        Arr4F;

template<class T,size_t S>
struct  Traits<Arr<T,S> >
{
    typedef typename Traits<T>::Scalar                  Scalar;
    typedef Arr<typename Traits<T>::Accumulator,S>      Accumulator;
    typedef Arr<typename Traits<T>::Floating,S>         Floating;
};

template<typename T,typename U,size_t S>
void scast_(Arr<T,S> const & from,Arr<U,S> & to)
{
    for (size_t ii=0; ii<S; ++ii)
        scast_(from[ii],to[ii]);
}

template<typename To,typename From,size_t S>
Arr<To,S>
scast(Arr<From,S> const & v)
{
    Arr<To,S>    ret;
    for (size_t ii=0; ii<S; ++ii)
        ret[ii] = scast<To>(v[ii]);
    return ret;
}

template<class To,class From,size_t S>
void
round_(Arr<From,S> const & from,Arr<To,S> & to)
{
    for (size_t ii=0; ii<S; ++ii)
        round_(from[ii],to[ii]);
}

template<class T,size_t S>
std::ostream &
operator<<(std::ostream & os,const Arr<T,S> & arr)
{
    os << "[";
    for (const T & e : arr)
        os << e << " ";
    return os << "]";
}

template<class T,size_t S>
Arr<T,S>
operator+(Arr<T,S> const & l,Arr<T,S> const & r)
{
    Arr<T,S>      ret;
    for (size_t ii=0; ii<S; ++ii)
        ret[ii] = l[ii] + r[ii];
    return ret;
}

template<class T,size_t S>
Arr<T,S>
operator-(Arr<T,S> const & l,Arr<T,S> const & r)
{
    Arr<T,S>      ret;
    for (size_t ii=0; ii<S; ++ii)
        ret[ii] = l[ii] - r[ii];
    return ret;
}

template<class T,size_t S>
Arr<T,S>
operator*(Arr<T,S> const & l,T r)
{
    Arr<T,S>      ret;
    for (size_t ii=0; ii<S; ++ii)
        ret[ii] = l[ii] * r;
    return ret;
}

template<class T,size_t S>
Arr<T,S>
operator/(Arr<T,S> const & l,T r)
{
    Arr<T,S>      ret;
    for (size_t ii=0; ii<S; ++ii)
        ret[ii] = l[ii] / r;
    return ret;
}

template<class T,size_t S>
void
operator+=(Arr<T,S> & l,Arr<T,S> const & r)
{
    for (size_t ii=0; ii<S; ++ii)
        l[ii] += r[ii];
}

template<class T,size_t S>
void
operator-=(Arr<T,S> & l,Arr<T,S> const & r)
{
    for (size_t ii=0; ii<S; ++ii)
        l[ii] -= r[ii];
}

template<class T,size_t S>
void
operator*=(Arr<T,S> & l,T r)
{
    for (size_t ii=0; ii<S; ++ii)
        l[ii] *= r;
}

template<class T,size_t S>
Arr<T,S>
cFloor(Arr<T,S> const & arr)
{
    Arr<T,S>      ret;
    for (size_t ii=0; ii<S; ++ii)
        ret[ii] = std::floor(arr[ii]);
    return ret;
}

template<class T,size_t S>
T
cSum(Arr<T,S> const & a)
{return std::accumulate(cbegin(a)+1,cend(a),a[0]); }

template<class T,size_t S>
T
cMean(Arr<T,S> const & a)
{return cSum(a)/a.size(); }

template<class T,size_t S>
T
cProd(Arr<T,S> const & a)
{return std::accumulate(cbegin(a)+1,cend(a),a[0],std::multiplies<T>{}); }

template<class T,size_t S>
size_t
cMinIdx(const Arr<T,S> & v)
{
    size_t      ret = 0;
    for (size_t ii=1; ii<v.size(); ++ii)
        if (v[ii] < v[ret])
            ret = ii;
    return ret;
}

template<class T,size_t S>
size_t
cMaxIdx(const Arr<T,S> & v)
{
    size_t      ret = 0;
    for (size_t ii=1; ii<v.size(); ++ii)
        if (v[ii] > v[ret])
            ret = ii;
    return ret;
}

}

#endif
