//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
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
typedef Arr<int,2>          Arr2I;
typedef Arr<float,2>        Arr2F;
typedef Arr<double,2>       Arr2D;

typedef Arr<uchar,3>        Arr3UC;
typedef Arr<schar,3>        Arr3SC;
typedef Arr<int,3>          Arr3I;
typedef Arr<uint,3>         Arr3UI;
typedef Arr<float,3>        Arr3F;
typedef Arr<double,3>       Arr3D;

typedef Arr<uchar,4>        Arr4UC;
typedef Arr<int,4>          Arr4I;
typedef Arr<uint,4>         Arr4UI;
typedef Arr<float,4>        Arr4F;
typedef Arr<double,4>       Arr4D;
typedef Arr<double,5>       Arr5D;

template<typename T,size_t S>
Arr<T,S>
cArr(T fillVal)
{
    Arr<T,S>        ret;
    ret.fill(fillVal);
    return ret;
}

inline Arr<bool,2>
cTrueFalse()
{return Arr<bool,2> {{true,false}}; }

template<class T,size_t S>
struct  Traits<Arr<T,S> >
{
    typedef typename Traits<T>::Scalar                  Scalar;
    typedef Arr<typename Traits<T>::Accumulator,S>      Accumulator;
    typedef Arr<typename Traits<T>::Floating,S>         Floating;
};

template<class T,size_t S>
std::ostream &
operator<<(std::ostream & os,const Arr<T,S> & arr)
{
    os << "[";
    for (T const & e : arr)
        os << e << " ";
    return os << "]";
}

// Map type-preserving binary callable operation:
template<typename T,size_t S,typename F>
Arr<T,S>
mapCall(Arr<T,S> const & lhs,Arr<T,S> const & rhs,F func)
{
    Arr<T,S>                ret;
    for (size_t ii=0; ii<S; ++ii)
        ret[ii] = func(lhs[ii],rhs[ii]);
    return ret;
}

template<class T,size_t S>
Arr<T,S>
operator+(Arr<T,S> const & l,Arr<T,S> const & r) {return mapCall(l,r,std::plus<T>{}); }
template<class T,size_t S>
Arr<T,S>
operator-(Arr<T,S> const & l,Arr<T,S> const & r) {return mapCall(l,r,std::minus<T>{}); }

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
void
operator/=(Arr<T,S> & l,T r)
{
    for (size_t ii=0; ii<S; ++ii)
        l[ii] /= r;
}

template<class T,size_t S>
Arr<T,S>
mapExp(Arr<T,S> const & a)
{
    Arr<T,S>                ret;
    for (size_t ii=0; ii<S; ++ii)
        ret[ii] = std::exp(a[ii]);
    return ret;
}

// Subtract same value from each element:
template<class T,size_t S>
Arr<T,S>
mapSub(Arr<T,S> const & lhs,T rhs)
{
    Arr<T,S>                ret;
    for (size_t ii=0; ii<S; ++ii)
        ret[ii] = lhs[ii] - rhs;
    return ret;
}

template<typename T,size_t S>
Arr<T,S>
mapMul(Arr<T,S> const & lhs,Arr<T,S> const & rhs)
{return mapCall(lhs,rhs,std::multiplies<T>{}); }

template<typename To,typename From,size_t S>
Arr<To,S>
mapCast(Arr<From,S> const & v)
{
    Arr<To,S>    ret;
    for (size_t ii=0; ii<S; ++ii)
        ret[ii] = scast<To>(v[ii]);
    return ret;
}

template<typename To,typename From,size_t S>
void
deepCast_(Arr<From,S> const & from,Arr<To,S> & to)
{
    for (size_t ii=0; ii<S; ++ii)
        deepCast_(from[ii],to[ii]);
}

template<class T,size_t S>
Arr<T,S>
mapSqr(Arr<T,S> const & a)
{
    Arr<T,S>        ret;
    for (size_t ii=0; ii<S; ++ii)
        ret[ii] = a[ii] * a[ii];
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
T
cDot(Arr<T,S> const & lhs,Arr<T,S> const & rhs)
{
    T               ret {0};
    for (size_t ii=0; ii<S; ++ii)
        ret += lhs[ii] * rhs[ii];
    return ret;
}

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

template<size_t N,class T,size_t M>
Arr<T,N>
cHead(Arr<T,M> const & v)
{
    static_assert(N<M,"Not a proper subset");
    Arr<T,N>        ret;
    for (size_t ii=0; ii<N; ++ii)
        ret[ii] = v[ii];
    return ret;
}

}

#endif
