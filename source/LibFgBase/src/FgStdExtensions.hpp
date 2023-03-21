//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGSTDEXTENSIONS_HPP
#define FGSTDEXTENSIONS_HPP

#include "FgDiagnostics.hpp"

namespace Fg {

// STD CONTAINERS TO OUTPUT STREAMS:

template<class T,size_t S>
std::ostream &      operator<<(std::ostream & os,Arr<T,S> const & arr)
{
    os << "[";
    for (T const & e : arr)
        os << Traits<T>::Printable(e) << " ";
    return os << "]";
}
template<class T>
std::ostream &      operator<<(std::ostream & ss,Svec<T> const & vv)
{
    std::ios::fmtflags       oldFlag = ss.setf(
        std::ios::fixed |
        std::ios::showpos |
        std::ios::right);
    std::streamsize          oldPrec = ss.precision(6);
    ss << "[" << Fg::fgpush;
    if (vv.size() > 0)
        ss << vv[0];
    for (size_t ii=1; ii<vv.size(); ii++)
        ss << "," << vv[ii];
    ss << Fg::fgpop << "]";
    ss.flags(oldFlag);
    ss.precision(oldPrec);
    return ss;
}
template<class T>
std::ostream &      operator<<(std::ostream & ss,std::shared_ptr<T> const & p)
{
    if (p)
        return ss << *p;
    else
        return ss << "NULL";
}

template<class T,class U>
std::ostream &
operator<<(std::ostream & ss,Pair<T,U> const & pp)
{
    return ss << "(" << pp.first << "," << pp.second << ")";
}

// STD CONTAINTER CONSTRUCTION FUNCTIONS:

template<class T,size_t S>
Arr<T,S>            cArr(T fillVal)
{
    Arr<T,S>        ret;
    ret.fill(fillVal);
    return ret;
}
// DEPRECATED: Element list construction when type must be explicit and {} cannot be used:
template<class T> inline Svec<T> svec(T const & v0) {return Svec<T>{v0}; }
template<class T> inline Svec<T> svec(T const & v0,T const & v1) {return Svec<T>{v0,v1}; }
template<class T> inline Svec<T> svec(T const & v0,T const & v1,T const & v2) {return Svec<T>{v0,v1,v2}; }


// Like C++17 std::data() but better named:
template <class _Elem>
static constexpr const _Elem* dataPtr(std::initializer_list<_Elem> _Ilist) noexcept
{return _Ilist.begin(); }


// FUNCTIONAL-STYLE (NOT ITERATOR) ALGORITHMS ON STD CONTAINERS:

// combine rounding with conversion to integer types. No bounds checking:
template<class T,class F,FG_ENABLE_IF(T,is_signed),FG_ENABLE_IF(T,is_integral),FG_ENABLE_IF(F,is_floating_point)>
inline T            roundT(F v) {return scast<T>(std::round(v)); }
template<class T,class F,FG_ENABLE_IF(T,is_unsigned),FG_ENABLE_IF(T,is_integral),FG_ENABLE_IF(F,is_floating_point)>
inline T            roundT(F v) {return scast<T>(v+F(0.5)); }
template<class T,class F,FG_ENABLE_IF(T,is_floating_point)>
inline T            roundT(F v) {return scast<T>(v); }
template<class T,class F>
void                mapRound_(F from,T & to) {to = roundT<T,F>(from); }

template<class T,FG_ENABLE_IF(T,is_floating_point)>
T                   interpolate(T v0,T v1,T val)    // returns v0 when val==0, v1 when val==1
{
    return v0 * (1-val) + v1 * val;
}

// cMag: Squared magnitude. Always returns double:
inline double       cMag(double v) {return v*v; }
inline double       cMag(std::complex<double> v) {return std::norm(v); }
template<class T> double  cMag(Svec<T> const & v);          // forward declare to handle Arr<Svec<...>>
template<class T,size_t S>
double              cMag(Arr<T,S> const & arr)
{
    double          acc {0};
    for (T const & e : arr)
        acc += cMag(e);
    return acc;
}
template<class T>
double              cMag(Svec<T> const & arr)
{
    double          acc {0};
    for (T const & e : arr)
        acc += cMag(e);
    return acc;
}

template<class T,size_t S,size_t R>
Arr<T,S+R>          cat(Arr<T,S> const & lhs,Arr<T,R> const & rhs)
{
    Arr<T,S+R>              ret;
    size_t                  cnt {0};
    for (T const & l : lhs)
        ret[cnt++] = l;
    for (T const & r : rhs)
        ret[cnt++] = r;
    return ret;
}

template<class T,size_t S>
Arr<size_t,S>       cSizes(Arr<std::vector<T>,S> const & vss)
{
    Arr<size_t,S>           ret;
    for (size_t ii=0; ii<S; ++ii)
        ret[ii] = vss[ii].size();
    return ret;
}

// map type-preserving unary callable:
template<class T,size_t S,class C>
Arr<T,S>            mapCall(Arr<T,S> const & arr,C call)
{
    Arr<T,S>            ret;
    for (size_t ii=0; ii<S; ++ii)
        ret[ii] = call(arr[ii]);
    return ret;
}
// map type-preserving binary callable:
template<class T,size_t S,class C>
Arr<T,S>            mapCall(Arr<T,S> const & lhs,Arr<T,S> const & rhs,C call)
{
    Arr<T,S>            ret;
    for (size_t ii=0; ii<S; ++ii)
        ret[ii] = call(lhs[ii],rhs[ii]);
    return ret;
}
// map type-preserving binary on aggregate/scalar pair callable:
template<class T,size_t S,class C>
Arr<T,S>            mapCall(Arr<T,S> const & lhs,T rhs,C call)
{
    Arr<T,S>            ret;
    for (size_t ii=0; ii<S; ++ii)
        ret[ii] = call(lhs[ii],rhs);
    return ret;
}

// forward declarations to handle arr<svec<T>>:
template<class T> Svec<T> operator+(Svec<T> const &  l,Svec<T> const &  r);
template<class T> Svec<T> operator-(Svec<T> const &  l,Svec<T> const &  r);

// addition and subtraction operators between 2 arrays are by convention element-wise
// note that we can't use std::plus / std::minus since these do not support overloads of operator+/-:
template<class T,size_t S>
Arr<T,S>        operator+(Arr<T,S> const & l,Arr<T,S> const & r) {return mapCall(l,r,[](T const & l,T const & r){return l+r; }); }
template<class T,size_t S>
Arr<T,S>        operator-(Arr<T,S> const & l,Arr<T,S> const & r) {return mapCall(l,r,[](T const & l,T const & r){return l-r; }); }

// addition and subtraction between an array and an element is explicit, not operator-overloaded
template<class T,size_t S>
Arr<T,S>            mapAdd(Arr<T,S> const & l,T r) {return mapCall(l,r,std::plus<T>{}); }
template<class T,size_t S>
Arr<T,S>            mapSub(Arr<T,S> const & l,T r) {return mapCall(l,r,std::minus<T>{}); }

// multiplication and division operators between an array and a scalar are by convention element-wise
template<class T,size_t S>
Arr<T,S>            operator*(Arr<T,S> const & l,T r) {return mapCall(l,r,std::multiplies<T>{}); }
template<class T,size_t S>
Arr<T,S>            operator/(Arr<T,S> const & l,T r) {return mapCall(l,r,std::divides<T>{}); }

// map type-preserving binary in-place modifying (too simple to make generic):
template<class T,size_t S>
void            operator+=(Arr<T,S> & l,Arr<T,S> const & r) {for (size_t ii=0; ii<S; ++ii) l[ii] += r[ii]; }
template<class T,size_t S>
void            operator-=(Arr<T,S> & l,Arr<T,S> const & r) {for (size_t ii=0; ii<S; ++ii) l[ii] -= r[ii]; }
template<class T,size_t S>
void            operator*=(Arr<T,S> & l,T r) {for (size_t ii=0; ii<S; ++ii) l[ii] *= r; }
template<class T,size_t S>
void            operator/=(Arr<T,S> & l,T r) {for (size_t ii=0; ii<S; ++ii) l[ii] /= r; }

// element-wise mutiplication and division between 2 arrays is explicit, not operator-overloaded:
template<class T,size_t S>
Arr<T,S>        mapMul(Arr<T,S> const & l,Arr<T,S> const & r) {return mapCall(l,r,std::multiplies<T>{}); }
template<class T,size_t S>
Arr<T,S>        mapDiv(Arr<T,S> const & l,Arr<T,S> const & r) {return mapCall(l,r,std::divides<T>{}); }

// multiply-typed version:
template<class T,class U,size_t S>
Arr<U,S>            mapMul(T const & lhs,Arr<U,S> const & rhs)
{
    Arr<U,S>            ret;
    for (size_t ii=0; ii<S; ++ii)
        ret[ii] = lhs * rhs[ii];
    return ret;
}

template<class T,size_t S>
Arr<T,S>            mapMin(Arr<T,S> const & lhs,Arr<T,S> const & rhs)
{
    struct F {T operator()(T l,T r){return std::min(l,r); } };
    return mapCall(lhs,rhs,F{});
}

template<class T,class F,size_t S>
Arr<T,S>           mapCast(Arr<F,S> const & arr)
{
    Arr<T,S>            ret;
    for (size_t ii=0; ii<S; ++ii)
        ret[ii] = scast<T>(arr[ii]);
    return ret;
}
template<class T,class F,size_t S>
void            mapCast_(Arr<F,S> const & from,Arr<T,S> & to)
{
    for (size_t ii=0; ii<S; ++ii)
        to[ii] = scast<T>(from[ii]);
}
template<class T,size_t S>
Arr<T,S>        mapSqr(Arr<T,S> const & arr) {return mapCall(arr,[](T v){return v*v;}); }
template<class To,class From,size_t S>
void            mapRound_(Arr<From,S> const & from,Arr<To,S> & to)
{
    for (size_t ii=0; ii<S; ++ii)
        mapRound_(from[ii],to[ii]);
}
template<class T,size_t S>
T               cSum(Arr<T,S> const & a) {return std::accumulate(cbegin(a),cend(a),T{0}); }
template<class T,size_t S>
T               cMean(Arr<T,S> const & a) {return cSum(a)/a.size(); }
template<class T,size_t S>
T               cProd(Arr<T,S> const & a) {return std::accumulate(cbegin(a)+1,cend(a),a[0],std::multiplies<T>{}); }

template<class T,size_t S>
T               cDot(Arr<T,S> const & lhs,Arr<T,S> const & rhs)
{
    T               ret {0};
    for (size_t ii=0; ii<S; ++ii)
        ret += lhs[ii] * rhs[ii];
    return ret;
}
template<class T,size_t S>
Arr<T,S>        normalize(Arr<T,S> const & arr)
{
    // can't use cMag() since we don't want double type if T is float:
    T               acc {0};
    for (size_t ii=0; ii<S; ++ii)
        acc += arr[ii] * arr[ii];
    return arr / std::sqrt(acc);
}
template<class T,size_t S>
size_t              cMinIdx(const Arr<T,S> & v)
{
    size_t      ret = 0;
    for (size_t ii=1; ii<v.size(); ++ii)
        if (v[ii] < v[ret])
            ret = ii;
    return ret;
}
template<class T,size_t S>
size_t              cMaxIdx(Arr<T,S> const & v)
{
    size_t      ret = 0;
    for (size_t ii=1; ii<v.size(); ++ii)
        if (v[ii] > v[ret])
            ret = ii;
    return ret;
}
template<size_t N,class T,size_t M>
Arr<T,N>            cHead(Arr<T,M> const & v)
{
    static_assert(N<M,"Not a proper subset");
    Arr<T,N>        ret;
    for (size_t ii=0; ii<N; ++ii)
        ret[ii] = v[ii];
    return ret;
}
template<class T,size_t S,class U>
bool                contains(Arr<T,S> const & arr,U const & val)
{
    for (T const & e : arr)
        if (e == val)
            return true;
    return false;
}
// if not found, returns the size of the array:
template<class T,size_t S,class U>
size_t              findFirstIdx(Arr<T,S> const & arr,U const & val)
{
    for (size_t ii=0; ii<S; ++ii)
        if (arr[ii] == val)
            return ii;
    return S;
}
template<class T,size_t S>
Arr<T,S>            reverseOrder(Arr<T,S> const & a)
{
    Arr<T,S>            ret;
    for (size_t ii=0; ii<S; ++ii)
        ret[S-ii-1] = a[ii];
    return ret;
}

template<typename To,typename From>
Svec<To>            mapCast(Svec<From> const & vec)
{
    Svec<To>        ret; ret.reserve(vec.size());
    for (From const & v : vec)
        ret.push_back(static_cast<To>(v));
    return ret;
}

template<typename To,typename From>
void                scast_(Svec<To> const & from,Svec<From> & to)
{
    to.resize(from.size());
    for (size_t ii=0; ii<to.size(); ++ii)
        scast_(from[ii],to[ii]);
}

// It's probably more correct to do the element-wise add/sub operations with 'map*' functions
// but this is convenient and widely used:
template<class T>
Svec<T>             operator-(Svec<T> const &  l,Svec<T> const &  r)
{
    Svec<T>             ret; ret.reserve(l.size());
    FGASSERT(l.size() == r.size());
    for (size_t ii=0; ii<l.size(); ++ii)
        ret.push_back(l[ii] - r[ii]);
    return ret;
}
template<class T>
Svec<T>             operator+(Svec<T> const &  l,Svec<T> const &  r)
{
    Svec<T>             ret; ret.reserve(l.size());
    FGASSERT(l.size() == r.size());
    for (size_t ii=0; ii<l.size(); ++ii)
        ret.push_back(l[ii] + r[ii]);
    return ret;
}
template<class T>
void                operator-=(Svec<T> & l,Svec<T> const & r)
{
    FGASSERT(l.size() == r.size());
    for (size_t ii=0; ii<l.size(); ++ii)
        l[ii] -= r[ii];
}
template<class T>
void                operator+=(Svec<T> & l,Svec<T> const & r)
{
    FGASSERT(l.size() == r.size());
    for (size_t ii=0; ii<l.size(); ++ii)
        l[ii] += r[ii];
}
// mul/div are overloaded to operators when one argument is a constant value.
// Types are different, for example, when T is a 3-vec and U is a float.
// The output type must be the same as the input, otherwise we need a named function.
// Element-wise mul/div is achieved using 'map*'.
template<class T,class U>
Svec<T>             operator*(Svec<T> const & lhs,U rhs)
{
    Svec<T>             ret; ret.reserve(lhs.size());
    for (size_t ii=0; ii<lhs.size(); ++ii)
        ret.push_back(lhs[ii] * rhs);
    return ret;
}
template<class T,class U>
Svec<T>             operator/(Svec<T> const & lhs,U rhs)
{
    Svec<T>             ret; ret.reserve(lhs.size());
    for (size_t ii=0; ii<lhs.size(); ++ii)
        ret.push_back(lhs[ii] / rhs);
    return ret;
}
template<class T,class U>
void                operator*=(Svec<T> & lhs,U rhs)
{
    for (T & l : lhs)
        l *= rhs;
}
template<class T,class U>
void                operator/=(Svec<T> & lhs,U rhs)
{
    for (T & l : lhs)
        l /= rhs;
}

// Acts just like bool for use with vector but retains a memory address for use with references
// and pointers (unlike std::vector<bool> specialization which is a bit field):
struct FatBool
{
    bool            m;
    FatBool() {}
    FatBool(bool v) : m{v} {}
    operator bool () const {return m; }
};
typedef Svec<FatBool>   FatBools;

template<class T>
void                mapAsgn_(Svec<T> & vec,T val) {std::fill(vec.begin(),vec.end(),val); }
// Generate elements using a callable type that accepts a 'size_t' argument and returns a T:
// NOTE: T must be explicitly specified:
template<class T,class C>
Svec<T>             genSvec(size_t num,C const & callable)
{
    Svec<T>             ret; ret.reserve(num);
    for (size_t ii=0; ii<num; ++ii)
        ret.push_back(callable(ii));
    return ret;
}
// multithreaded version is structurally different so different function rather than boolean arg:
template<class T,class C>
Svec<T>             generateSvecMt(size_t num,C const & callable)
{
    Svec<T>             ret(num);           // default-constructable required
    size_t              nt = std::min(size_t(std::thread::hardware_concurrency()),num);
    auto                fn = [&callable,&ret](size_t lo,size_t eub)
    {
        for (size_t ii=lo; ii<eub; ++ii)
            ret[ii] = callable(ii);
    };
    Svec<std::thread>   threads; threads.reserve(nt);
    for (size_t tt=0; tt<nt; ++tt) {
        size_t              lo = (tt*num)/nt,
                            hi = ((tt+1)*num)/nt;
        threads.emplace_back(fn,lo,hi);
    }
    for (std::thread & thread : threads)
        thread.join();
    return ret;
}
template<class T,class U>
Svec<T>             mapConstruct(Svec<U> const & us)
{
    Svec<T>         ret;
    ret.reserve(us.size());
    for (U const & u : us)
        ret.emplace_back(u);
    return ret;
}
template<class From,class To>
Svec<To>            mapConvert(const Svec<From> & in)
{
    Svec<To>   ret;
    ret.reserve(in.size());
    for (typename Svec<From>::const_iterator it=in.begin(); it != in.end(); ++it)
        ret.push_back(To(*it));
    return ret;
}
// Linear interpolation between vectors of equal lengths:
template<class T>
Svec<T>             interpolate(Svec<T> const & v0,Svec<T> const & v1,T val)
{
    T                   omv = 1 - val;
    return mapCall(v0,v1,[val,omv](T l,T r){return l*omv + r*val; });
}
// Returns a vector one larger in size than 'vec', where each element is the integral of all elements of 'vec'
// up to but not including the current one:
template<class T>
Svec<T>             integrate(Svec<T> const & vec)
{
    Svec<T>       ret;
    ret.reserve(vec.size()+1);
    T               acc(0);
    ret.push_back(acc);
    for (T v : vec) {
        acc += v;
        ret.push_back(acc);
    }
    return ret;
}

// Structural:

template<class T>
Svec<T>             cSubvec(Svec<T> const & vec,size_t start,size_t size)
{
    FGASSERT(start+size <= vec.size());
    return  Svec<T>(vec.begin()+start,vec.begin()+start+size);
}
// Truncate after the first elements of 'vec' up to 'size', if present:
template<class T>
void                cHead_(Svec<T> & vec,size_t size)
{
    size_t              sz = std::min(vec.size(),size);
    vec.resize(sz);
}
// Return the first elements of 'vec' up to 'size', if present:
template<class T>
Svec<T>             cHead(Svec<T> const & vec,size_t size)
{
    size_t              sz = std::min(vec.size(),size);
    return Svec<T>(vec.begin(),vec.begin()+sz);
}
template<class T>
Svec<T>             cRest(Svec<T> const & vec,size_t start=1)
{
    FGASSERT(start <= vec.size());      // Can be size zero
    return Svec<T>(vec.begin()+start,vec.end());
}
template<class T>
Svec<T>             cTail(Svec<T> const & vec,size_t size)
{
    FGASSERT(size <= vec.size());
    return Svec<T>(vec.end()-size,vec.end());
}
// Append is the functional equivalent of push_back:
template<class T>
Svec<T>             append(Svec<T> const & vec,T const & val)
{
    Svec<T>       ret;
    ret.reserve(vec.size()+1);
    ret.insert(ret.begin(),vec.cbegin(),vec.cend());
    ret.push_back(val);
    return ret;
}
// Functional equivalent of insert at front:
template<class T>
Svec<T>             prepend(T const & val,Svec<T> const & vec)
{
    Svec<T>       ret;
    ret.reserve(vec.size()+1);
    ret.push_back(val);
    ret.insert(ret.end(),vec.begin(),vec.end());
    return ret;
}
// append from pointer and size:
template<class T>
void                append_(Svec<T> & vec,T const * vals,size_t num)
{
    vec.reserve(vec.size()+num);
    for (size_t ii=0; ii<num; ++ii)
        vec.push_back(vals[ii]);
}
// concatenation of 2 or 3 std::vectors (concatenation of a vector of vectors is called 'flatten'):
template<class T>
inline void         cat_(Svec<T> & base,Svec<T> const & app) {base.insert(base.end(),app.begin(),app.end()); }
// see 'flatten' instead of cat(Svec<Svec<T>>)
template<class T>
Svec<T>             cat(Svec<T> const & v0,Svec<T> const & v1)
{
    Svec<T>             ret; ret.reserve(v0.size()+v1.size());
    ret.insert(ret.end(),v0.begin(),v0.end());
    ret.insert(ret.end(),v1.begin(),v1.end());
    return ret;
}
template<class T>
Svec<T>             cat(Svec<T> const & v0,Svec<T> const & v1,Svec<T> const & v2)
{
    Svec<T>             ret; ret.reserve(v0.size()+v1.size()+v2.size());
    ret.insert(ret.end(),v0.begin(),v0.end());
    ret.insert(ret.end(),v1.begin(),v1.end());
    ret.insert(ret.end(),v2.begin(),v2.end());
    return ret;
}
template<class T>
Svec<T>             cat(Svec<T> const & v0,Svec<T> const & v1,Svec<T> const & v2,Svec<T> const & v3)
{
    Svec<T>             ret; ret.reserve(v0.size()+v1.size()+v2.size()+v3.size());
    ret.insert(ret.end(),v0.begin(),v0.end());
    ret.insert(ret.end(),v1.begin(),v1.end());
    ret.insert(ret.end(),v2.begin(),v2.end());
    ret.insert(ret.end(),v3.begin(),v3.end());
    return ret;
}
template<class T>
Svec<T>             catDeref(Svec<Svec<T> const *> const & tsPtrs)
{
    Svec<T>             ret;
    size_t              sz {0};
    for (Svec<T> const * tsPtr : tsPtrs)
        sz += tsPtr->size();
    ret.reserve(sz);
    for (Svec<T> const * tsPtr : tsPtrs)
        cat_(ret,*tsPtr);
    return ret;
}
// Avoid re-typeing argument for vector::erase of a single element:
template<class T>
void                snip_(Svec<T> & v,size_t idx) {v.erase(v.begin()+idx); }
// Functional version of vector::erase for single element:
template<class T>
Svec<T>             snip(Svec<T> const & v,size_t idx)
{
    FGASSERT(idx < v.size());
    Svec<T>       ret;
    ret.reserve(v.size()-1);
    for (size_t ii=0; ii<idx; ++ii)
        ret.push_back(v[ii]);
    for (size_t ii=idx+1; ii<v.size(); ++ii)
        ret.push_back(v[ii]);
    return ret;
}
// Not recursive; only flattens vector<vector> to vector<>:
template<class T>
Svec<T>             flatten(Svec<Svec<T>> const & v)
{
    Svec<T>             ret;
    size_t              sz = 0;
    for (size_t ii=0; ii<v.size(); ++ii)
        sz += v[ii].size();
    ret.reserve(sz);
    for (size_t ii=0; ii<v.size(); ++ii)
        ret.insert(ret.end(),v[ii].begin(),v[ii].end());
    return ret;       
}
// Like std::find except it returns index rather than iterator of first occurance.
// If not found, returns v.size() or throws:
template<class T,class U>
size_t              findFirstIdx(
    Svec<T> const &     vec,
    U const &           val,     // Allow for T::operator==(U)
    bool                throwOnFail=false)
{
    for (size_t ii=0; ii<vec.size(); ++ii)
        if (vec[ii] == val)
            return ii;
    if (throwOnFail)
        FGASSERT_FALSE;
    return vec.size();
}
// Functional specialization of std::find, throws if not found:
template<class T,class U>
T const &           findFirst(Svec<T> const & vec,U const & rhs)        // Allow for T::operator==(U)
{
    auto            it = std::find(vec.cbegin(),vec.cend(),rhs);
    FGASSERT(it != vec.end());
    return *it;
}
// non-const version:
template<class T,class U>
T &                 findFirst(Svec<T> & vec,U const & rhs)              // Allow for T::operator==(U)
{
    auto            it = std::find(vec.begin(),vec.end(),rhs);
    FGASSERT(it != vec.end());
    return *it;
}
// Functional specialization of std::find_if, throws if not found:
template<class T,class U>
T const &           findFirstIf(
    Svec<T> const &     vec,
    U const &           func)       // Allow for std::function
{
    auto            it = std::find_if(vec.begin(),vec.end(),func);
    FGASSERT(it != vec.end());
    return *it;
}
template<class T,class U>
size_t              findFirstIdxIf(
    Svec<T> const &     vec,
    U const &           func,       // Allow for std::function
    bool                throwOnFail=true)
{
    for (size_t ii=0; ii<vec.size(); ++ii)
        if (func(vec[ii]))
            return ii;
    if (throwOnFail)
        FGASSERT_FALSE;
    return vec.size();
}
template<class T,class U>
size_t              findLastIdx(Svec<T> const & vec,const U & val)
{
    for (size_t ii=vec.size(); ii!=0; --ii)
        if (vec[ii-1] == val)
            return ii-1;
    return vec.size();
}
template<class T>
Svec<T>             filter(Svec<T> const & vals,Bools const & keepFlags)
{
    size_t              V = vals.size();
    FGASSERT(keepFlags.size() == V);
    size_t              S {0};
    for (bool f : keepFlags)
        if (f) ++S;
    Svec<T>             ret; ret.reserve(S);
    for (size_t vv=0; vv<V; ++vv)
        if (keepFlags[vv])
            ret.push_back(vals[vv]);
    return ret;
}
// functional version of std::copy_if over entire vector (like a filter):
template<class T,class P>
Svec<T>             findAll(Svec<T> const & vals,P const & pred)    // pred() must take one T argument
{
    Svec<T>             ret;
    for (T const & v : vals)
        if (pred(v))
            ret.push_back(v);
    return ret;
}
template<class T>
Svec<T>             findAll(Svec<T> const & in,Bools const & accept)
{
    Svec<T>             ret;
    FGASSERT(accept.size() == in.size());
    for (size_t ii=0; ii<in.size(); ++ii)
        if (accept[ii])
            ret.push_back(in[ii]);
    return ret;
}
// Returns index of first instance whose given member matches 'val', or vec.size() if none found:
template<class T,class U>
size_t              findFirstIdxByMember(Svec<T> const & vec,U T::*mbr,U const & val)
{
    for (size_t ii=0; ii<vec.size(); ++ii)
        if (vec[ii].*mbr == val)
            return ii;
    return vec.size();
}
// Returns all instances whose given member matches 'val':
template<class T,class U>
Svec<T>             findAllByMember(Svec<T> const & vec,U T::*mbr,U const & val)
{
    Svec<T>                 ret;
    for (T const & elm : vec)
        if (elm.*mbr == val)
            ret.push_back(elm);
    return ret;
}
// Returns first instance whose given member matches 'val'. Throws if none.
template<class T,class U>
T const &           findFirstByMember(Svec<T> const & svec,U T::*mbr,U const & val)
{
    size_t          ii = 0;
    for (; ii<svec.size(); ++ii)
        if (svec[ii].*mbr == val)
            break;
    if (ii >= svec.size())
        fgThrow("findFirstByMember (const) not found");
    return svec[ii];
}
// non-const version:
template<class T,class U>
T &                 findFirstByMember(Svec<T> & svec,U T::*mbr,U const & val)
{
    size_t          ii = 0;
    for (; ii<svec.size(); ++ii)
        if (svec[ii].*mbr == val)
            break;
    if (ii >= svec.size())
        fgThrow("findFirstByMember not found");
    return svec[ii];
}
template<class T,class U>
bool                contains(Svec<T> const & vec,const U & val)     // Allows for T::operator==(U)
{
    for (size_t ii=0; ii<vec.size(); ++ii)
        if (vec[ii] == val)
            return true;
    return false;
}
template<class T,class U>
bool                containsAny(Svec<T> const & ctr,const Svec<U> & vals)     // Simple and slow: O(ctr * vals)
{
    for (size_t ii=0; ii<vals.size(); ++ii)
        if (contains(ctr,vals[ii]))
            return true;
    return false;
}
template<class T,class U>
bool                containsAll(Svec<T> const & ctr,const Svec<U> & vals)     // Simple and slow: O(ctr * vals)
{
    for (size_t ii=0; ii<vals.size(); ++ii)
        if (!contains(ctr,vals[ii]))
            return false;
    return true;
}
template<class T,class U>
bool                containsMember(Svec<T> const & vec,U T::*mbr,U const & val)
{
    for (T const & v : vec)
        if (v.*mbr == val)
            return true;
    return false;
}
template<class T>
void                replaceAll_(Svec<T> & v,T a,T b)       // Replace each 'a' with 'b'
{
    for (size_t ii=0; ii<v.size(); ++ii)
        if (v[ii] == a)
            v[ii] = b;
}
template<class T>
Svec<T>             replaceAll(Svec<T> const & vec,T const & a,T const & b) // Replace each 'a' with 'b'
{
    Svec<T>       ret;
    ret.reserve(vec.size());
    for (T const & v : vec) {
        if (v == a)
            ret.push_back(b);
        else
            ret.push_back(v);
    }
    return ret;
}
// Returns at least size 1, with 1 additional for each split element:
template<class T>
Svec<Svec<T>>       splitAtChar(Svec<T> const & str,T ch)
{
    Svec<Svec<T>>    ret;
    Svec<T>                  ss;
    for(size_t ii=0; ii<str.size(); ++ii) {
        if (str[ii] == ch) {
            ret.push_back(ss);
            ss.clear();
        }
        else
            ss.push_back(str[ii]);
    }
    ret.push_back(ss);
    return ret;
}
// Overwrite the subset of 'target' starting at 'startPos' with the values from 'data':
template<class T>
void                inject_(Svec<T> const & data,size_t startPos,Svec<T> & target)
{
    FGASSERT(data.size() + startPos <= target.size());
    copy(data.begin(),data.end(),target.begin()+startPos);
}
// Returns the result of overwriting the subset of 'target' starting at 'startPos' with the values from 'data':
template<class T>
Svec<T>             inject(Svec<T> const & data,size_t startPos,Svec<T> const & target)
{
    FGASSERT(data.size() + startPos <= target.size());
    Svec<T>                 ret = target;
    copy(data.begin(),data.end(),ret.begin()+startPos);
    return ret;
}
template<class T>
bool                beginsWith(Svec<T> const & base,Svec<T> const & pattern)
{
    if (pattern.size() > base.size())
        return false;
    for (size_t ii=0; ii<pattern.size(); ++ii)
        if (pattern[ii] != base[ii])
            return false;
    return true;
}
template<class T>
bool                endsWith(Svec<T> const & base,Svec<T> const & pattern)
{
    if (pattern.size() > base.size())
        return false;
    size_t      offset = base.size() - pattern.size();
    for (size_t ii=0; ii<pattern.size(); ++ii)
        if (pattern[ii] != base[ii+offset])
            return false;
    return true;
}

// Numerical:

// Sum into an existing accumulator:
template<class T>
void                cSum_(Svec<T> const & in,T & out)
{
    for (T const & i : in)
        out += i;
}
// NOTE: The value is accumulated in the templated type. Make a special purpose function
// if the accumulator type must be larger than the templated type:
template<class T>
T                   cSum(Svec<T> const & v)
{
    // We don't use std::accumulate since we want to properly handle the case where T itself
    // is a container type that supports operator+=()
    T           ret(0);
    if (!v.empty()) {
        ret = v[0];
        for (size_t ii=1; ii<v.size(); ++ii)
            ret += v[ii];
    }
    return ret;
}
template<class T>
T                   cProduct(Svec<T> const & v)
{
    typedef typename Traits<T>::Accumulator Acc;
    Acc         acc(1);
    for (size_t ii=0; ii<v.size(); ++ii)
        acc *= Acc(v[ii]);
    return T(acc);
}
// The value is accumulated in the templated type:
template<class T>
T                   cMean(Svec<T> const & v)
{
    typedef typename Traits<T>::Scalar    S;
    return cSum(v) / static_cast<S>(v.size());
}
template<class T>
double              cLen(Svec<T> const & v) {return std::sqrt(cMag(v)); }
template<class T>
Svec<T>             normalize(Svec<T> const & v)
{
    T           len = scast<T>(cLen(v));
    FGASSERT(len > 0.0f);
    return v * (1.0f/len);
}
// Map callable unary operation to same type
// (functional version of std::transform where output type is same as input type):
template<class T,class C>
Svec<T>             mapCall(Svec<T> const & in,C func)
{
    Svec<T>             ret; ret.reserve(in.size());
    for (T const & p : in)
        ret.push_back(func(p));
    return ret;
}
// Map callable unary operation to different type (must be specified):
template<class T,class U,class C>
Svec<T>             mapCallT(Svec<U> const & in,C fn)
{
    Svec<T>           ret; ret.reserve(in.size());
    for (U const & p : in)
        ret.push_back(fn(p));
    return ret;
}
// Map callable binary operation to same type
template<class T,class C>
Svec<T>             mapCall(Svec<T> const & l,Svec<T> const & r,C func)
{
    FGASSERT(l.size() == r.size());
    Svec<T>             ret; ret.reserve(l.size());
    for (size_t ii=0; ii<l.size(); ++ii)
        ret.push_back(func(l[ii],r[ii]));
    return ret;
}
// Map callable binary operation to different type (must be specified):
template<class T,class U,class V,class C>
Svec<T>             mapCallT(Svec<U> const & in0,Svec<V> const & in1,C fn)
{
    size_t              S = in0.size();
    FGASSERT(in1.size() == S);
    Svec<T>             ret; ret.reserve(S);
    for (size_t ii=0; ii<S; ++ii)
        ret.push_back(fn(in0[ii],in1[ii]));
    return ret;
}
template<class T,class U,class V,class W,class C>
Svec<T>             mapCallT(Svec<U> const & in0,Svec<V> const & in1,Svec<W> const & in2,C fn)
{
    size_t              S = in0.size();
    FGASSERT(in1.size() == S);
    Svec<T>             ret; ret.reserve(S);
    for (size_t ii=0; ii<S; ++ii)
        ret.push_back(fn(in0[ii],in1[ii],in2[ii]));
    return ret;
}
template<class T>
Svec<T const *>     mapAddr(Svec<T> const & v)
{
    Svec<T const *>         ret; ret.reserve(v.size());
    for (T const & e : v)
        ret.push_back(&e);
    return ret;
}

template<class T>
Svec<T>             mapMul(Svec<T> const & l,Svec<T> const & r)
{
    size_t              S = l.size();
    FGASSERT(r.size() == S);
    Svec<T>             ret; ret.reserve(S);
    for (size_t ss=0; ss<S; ++ss)
        ret.push_back(l[ss]*r[ss]);
    return ret;
}

// Map a LHS constant multiplication to same type as RHS:  L * R -> R
// Output type matches input so explicit template args not necessary.
template<class L,class R>
Svec<R>             mapMul(L lhs,Svec<R> const & rhs)
{
    Svec<R>         ret; ret.reserve(rhs.size());
    for (size_t ii=0; ii<rhs.size(); ++ii)
        ret.push_back(lhs * rhs[ii]);
    return ret;
}
// Map a LHS constant multiplication to new type:  Op * T -> U
// Output type explicit template arg required.
template<class Out,class In,class Op>
Svec<Out>           mapMulT(Op const & op,Svec<In> const & in)
{
    Svec<Out>           ret; ret.reserve(in.size());
    for (size_t ii=0; ii<in.size(); ++ii)
        ret.push_back(op * in[ii]);
    return ret;
}
// Non-functional version:
template<class T,class U,class Op>
void                mapMul_(Op const & op,Svec<T> const & in,Svec<U> & out)
{
    out.resize(in.size());
    for (size_t ii=0; ii<in.size(); ++ii)
        out[ii] = op * in[ii];
}
// Non-functional in-place version:
template<class T,class Op>
void                mapMul_(Op const & op,Svec<T> & data)
{
    for (size_t ii=0; ii<data.size(); ++ii)
        data[ii] = op * data[ii];
}
template<class T>
Svec<T>             mapAbs(Svec<T> const & vec)
{
    Svec<T>     ret;
    ret.reserve(vec.size());
    for (T const & v : vec)
        ret.push_back(std::abs(v));
    return ret;
}
template<class T>
Svec<T>             mapAdd(Svec<T> const & vec,T const & val)
{
    Svec<T>       ret;
    ret.reserve(vec.size());
    for (T const & v : vec)
        ret.push_back(v + val);
    return ret;
}
template<class T>
Svec<T>             mapSub(T const & val,Svec<T> const & vec)
{
    Svec<T>       ret;
    ret.reserve(vec.size());
    for (T const & v : vec)
        ret.push_back(val - v);
    return ret;
}
template<class T>
Svec<T>             mapSub(Svec<T> const & vec,T const & val)
{
    Svec<T>       ret;
    ret.reserve(vec.size());
    for (T const & v : vec)
        ret.push_back(v - val);
    return ret;
}
// Element-wise division:
template<class T>
Svec<T>             mapDiv(Svec<T> const & lhs,Svec<T> const & rhs)
{
    Svec<T>         ret;
    FGASSERT(lhs.size() == rhs.size());
    ret.reserve(lhs.size());
    for (size_t ii=0; ii<lhs.size(); ++ii)
        ret.push_back(lhs[ii]/rhs[ii]);
    return ret;
}
template<class T>
Svec<T>             mapSqr(Svec<T> const & v)
{
    Svec<T>         ret;
    ret.reserve(v.size());
    for (size_t ii=0; ii<v.size(); ++ii)
        ret.push_back(v[ii]*v[ii]);
    return ret;
}
// Add a weighted vector of values to an existing vector of values: acc += val * vec
// aka multiply-accumulate loop:
template<class T,class U,class V>
void                mapMulAcc_(Svec<T> const & vec,U val,Svec<V> & acc)
{
    FGASSERT(vec.size() == acc.size());
    for (size_t ii=0; ii<acc.size(); ++ii)
        acc[ii] += vec[ii] * val;
}
template<class T,class U,class V>
Svec<V>             mapMulAcc(Svec<T> const & base,U val,Svec<V> const & vec)
{
    FGASSERT(base.size() == vec.size());
    Svec<T>                 ret = base;
    for (size_t ii=0; ii<ret.size(); ++ii)
        ret[ii] += vec[ii] * val;
    return ret;
}

template<class R,class U,class V,size_t S,class Callable>
R                   reduceSum(Arr<U,S> const & l,Arr<V,S> const & r,Callable const & fn)
{
    R                   acc {0};
    for (size_t ii=0; ii<S; ++ii)
        acc += fn(l[ii],r[ii]);
    return acc;
}
template<class R,class U,class V,class W,size_t S,class Callable>
R                   reduceSum(Arr<U,S> const & a0,Arr<V,S> const & a1,Arr<W,S> const & a2,Callable const & fn)
{
    R                   acc {0};
    for (size_t ii=0; ii<S; ++ii)
        acc += fn(a0[ii],a1[ii],a2[ii]);
    return acc;
}

template<class R,class U,class V,class Callable>
R                   reduceSum(Svec<U> const & l,Svec<V> const & r,Callable const & fn)
{
    size_t              S = l.size();
    FGASSERT(r.size() == S);
    R                   acc {0};
    for (size_t ii=0; ii<S; ++ii)
        acc += fn(l[ii],r[ii]);
    return acc;
}

template<class T>
size_t              cMinIdx(Svec<T> const & v)
{
    FGASSERT(!v.empty());
    size_t      ret = 0;
    for (size_t ii=1; ii<v.size(); ++ii)
        if (v[ii] < v[ret])
            ret = ii;
    return ret;
}
template<class T>
size_t              cMaxIdx(Svec<T> const & v)
{
    FGASSERT(!v.empty());
    size_t      ret = 0;
    for (size_t ii=1; ii<v.size(); ++ii)
        if (v[ii] > v[ret])
            ret = ii;
    return ret;
}
// To get a min version, just use greater than for 'lessThan':
template<class T>
size_t              cMaxIdx(Svec<T> const & v,const std::function<bool(T const & lhs,T const & rhs)> & lessThan)
{
    FGASSERT(!v.empty());
    size_t      ret = 0;
    for (size_t ii=1; ii<v.size(); ++ii)
        if (lessThan(v[ret],v[ii]))
            ret = ii;
    return ret;    
}
// Common case where we want to sort the whole vector:
template<class T>
inline void         cSort_(Svec<T> & v) {std::sort(v.begin(),v.end()); }

// Functional version of sorting whole vector:
template<class T>
Svec<T>             cSort(Svec<T> v) {std::sort(v.begin(),v.end()); return v; }
// ... and with predicate (stdlib does this separately so we do too):
template<class T,class P>
Svec<T>             cSort(Svec<T> const & v,P const & pred)
{
    Svec<T>             ret(v);
    std::sort(ret.begin(),ret.end(),pred);
    return ret;
}
// Return array of indices into the given array such that the input elements are sorted,
// ie. A mapping from the SORTED order to the ORIGINAL order:
template<class T>
Sizes               sortInds(Svec<T> const & v)
{
    Sizes               inds; inds.reserve(v.size());
    for (size_t ii=0; ii<v.size(); ++ii)
        inds.push_back(ii);
    std::sort(inds.begin(),inds.end(),[&v](size_t l,size_t r){return (v[l]<v[r]); });
    return inds;
}
// reverse order of above:
template<class T>
Sizes               sortIndsR(Svec<T> const & v)
{
    Sizes               inds; inds.reserve(v.size());
    for (size_t ii=0; ii<v.size(); ++ii)
        inds.push_back(ii);
    std::sort(inds.begin(),inds.end(),[&v](size_t l,size_t r){return (v[r]<v[l]); });
    return inds;
}
// create a new list of values from 'vals' given by the order and indices in 'indices'
// permute / reorder (re-order) is a special case when 'indices' is a 1-1 map with 'in'.
template<class T,class U,
    // Can be uint or uint64, and on some platforms size_t is different from these:
    FG_ENABLE_IF(U,is_unsigned),
    FG_ENABLE_IF(U,is_integral)
>
Svec<T>             select(Svec<T> const & vals,Svec<U> const & indices)
{
    Svec<T>             ret; ret.reserve(indices.size());
    for (size_t ii=0; ii<indices.size(); ++ii)
        ret.push_back(vals[indices[ii]]);
    return ret;
}
template<class T,class U,size_t S,
    // Can be uint or uint64, and on some platforms size_t is different from these:
    FG_ENABLE_IF(U,is_unsigned),
    FG_ENABLE_IF(U,is_integral)
>
Arr<T,S>            select(Svec<T> const & vals,Arr<U,S> indices)
{
    Arr<T,S>            ret;
    for (size_t ii=0; ii<S; ++ii)
        ret[ii] = vals[indices[ii]];
    return ret;
}
template<class T>
bool                containsDuplicates(Svec<T> const & mustBeSorted)
{
    for (size_t ii=1; ii<mustBeSorted.size(); ++ii)
        if (mustBeSorted[ii] == mustBeSorted[ii-1])
            return true;
    return false;
}
// Removes duplicates from a sorted vector (I don't get std::unique):
template<class T>
Svec<T>             getUniqueSorted(Svec<T> const & sorted)
{
    Svec<T>       ret;
    if (!sorted.empty()) {
        ret.push_back(sorted[0]);
        for (size_t ii=1; ii<sorted.size(); ++ii)
            if (!(sorted[ii] == ret.back()))     // In case type only defines operator==
                ret.push_back(sorted[ii]);
    }
    return ret;
}
// Removes duplicates from an unsorted vector and returns uniques in order of appearance.
// Not designed for large return lists as uniqueness test is linear search:
template<class T>
Svec<T>             getUniqueUnsorted(Svec<T> const & vs)
{
    Svec<T>       ret;
    for (T const & v : vs)
        if (!contains(ret,v))
            ret.push_back(v);
    return ret;
}
template<class T,class U>
Svec<U>             sliceMember(Svec<T> const & vs,U T::*m)
{
    Svec<U>             ret; ret.reserve(vs.size());
    for (T const & v : vs)
        ret.push_back(v.*m);
    return ret;
}
// pointers to pointers member slice:
template<class T,class U>
Ptrs<U>             sliceMemberPP(Ptrs<T> const & ps,U T::*m)
{
    Ptrs<U>             ret; ret.reserve(ps.size());
    for (T const * p : ps)
        ret.push_back(&(p->*m));
    return ret;
}
// pointers to values member slice:
template<class T,class U>
Svec<U>             sliceMemberPV(Ptrs<T> const & ps,U T::*m)
{
    Svec<U>             ret; ret.reserve(ps.size());
    for (T const * p : ps)
        ret.push_back(p->*m);
    return ret;
}
// values to pointers member slice:
template<class T,class U>
Ptrs<U>             sliceMemberVP(Svec<T> const & vs,U T::*m)
{
    Ptrs<U>             ret; ret.reserve(vs.size());
    for (T const & v : vs)
        ret.push_back(&(v.*m));
    return ret;
}
// must have explicit member constructor to work since it uses emplace_back:
template<class T,class U,class V>
Svec<T>             zipMembers(Svec<U> const & m0s,Svec<V> const & m1s)
{
    size_t              S = m0s.size();
    FGASSERT(m1s.size() == S);
    Svec<T>             ret; ret.reserve(S);
    for (size_t ii=0; ii<S; ++ii)
        ret.emplace_back(m0s[ii],m1s[ii]);
    return ret;
}
// Transpose a vector of vectors just like Python 'zip' on lists.
// All sub-vectors must have the same size().
// This function is an involution when both sizes are non-zero.
template<class T>
Svec<Svec<T>>       transpose(Svec<Svec<T>> const & v)
{
    Svec<Svec<T>>       ret;
    if (!v.empty()) {
        size_t          sz = v[0].size();
        for (size_t jj=1; jj<v.size(); ++jj)
            FGASSERT(v[jj].size() == sz);
        ret.resize(sz);
        for (size_t ii=0; ii<sz; ++ii) {
            ret[ii].reserve(v.size());
            for (size_t jj=0; jj<v.size(); ++jj)
                ret[ii].push_back(v[jj][ii]);
        }
    }
    return ret;
}
// Inject all elements of domain 'src' into codomain 'dst' in order at true values of 'where'.
// REQUIRED: where.size() == dst.size()
// REQUIRED: number of 'true' values in where == src.size()
template<class T>
Svec<T>             inject(Svec<T> const & src,Svec<T> const & dst,Bools const & where)
{
    FGASSERT(dst.size() == where.size());
    Svec<T>             ret;
    ret.reserve(dst.size());
    size_t              cnt {0};
    for (size_t ii=0; ii<dst.size(); ++ii) {
        if (where[ii]) {
            FGASSERT(cnt < src.size());
            ret.push_back(src[cnt++]);
        }
        else
            ret.push_back(dst[ii]);
    }
    FGASSERT(cnt == src.size());
    return ret;
}
template<class T>
Sizes               cSizes(Svec<Svec<T>> const & vss)
{
    Sizes               ret; ret.reserve(vss.size());
    for (Svec<T> const & vs : vss)
        ret.push_back(vs.size());
    return ret;
}
// Overload for vector of pointers to vector:
template<class T>
Sizes               cSizes(Svec<Svec<T> const *> const & vsPtrs)
{
    Sizes               ret; ret.reserve(vsPtrs.size());
    for (Svec<T> const * vsPtr : vsPtrs)
        ret.push_back(vsPtr->size());
    return ret;
}
template<class T>
size_t              sumSizes(Svec<Svec<T>> const & vss)
{
    size_t              ret {0};
    for (Svec<T> const & vs : vss)
        ret += vs.size();
    return ret;
}
// Set intersection with vector containers; ignores duplicates returns intersection of uniques:
template<class T>
Svec<T>             setwiseIntersect(Svec<T> const & v0,Svec<T> const & v1)
{
    Svec<T>       ret;
    for (T const & v : v0) {
        if (contains(v1,v) && !contains(ret,v))
            ret.push_back(v);
    }
    return ret;
}
template<class T>
Svec<T>             setwiseIntersect(Svec<Svec<T>> const & vs)
{
    Svec<T>       ret;
    if (!vs.empty()) {
        ret = vs[0];
        for (size_t ii=1; ii<vs.size(); ++ii)
            ret = setwiseIntersect(ret,vs[ii]);
    }
    return ret;
}
// Multiset intersection with vector containers:
template<class T>
Svec<T>             multisetIntersect(Svec<T> const & v0,Svec<T> v1)
{
    Svec<T>         ret;
    for (T const & v : v0) {
        auto            it = find(v1.cbegin(),v1.cend(),v);
        if (it != v1.end()) {
            ret.push_back(v);
            v1.erase(it);
        }
    }
    return ret;
}
// Set union on vector containers, retaining lhs order then rhs order:
template<class T>
void                setwiseAdd_(Svec<T> & lhs,Svec<T> const & rhs)
{
    for (T const & r : rhs)
        if (!contains(lhs,r))
            lhs.push_back(r);
}
// Set union on vector containers, retaining lhs order then rhs order:
template<class T>
Svec<T>             setwiseAdd(Svec<T> lhs,Svec<T> const & rhs)
{
    setwiseAdd_(lhs,rhs);
    return lhs;
}
// Set subtraction on vector containers (lhs retains ordering):
template<class T>
Svec<T>             setwiseSubtract(Svec<T> const & lhs,Svec<T> const & rhs)
{
    Svec<T>       ret;
    for (T const & l : lhs) {
        if (!contains(rhs,l))
            ret.push_back(l);
    }
    return ret;
}
template<class T>
Svec<T>             cReverse(Svec<T> v)
{
    std::reverse(v.begin(),v.end());
    return v;
}
// 1D closed manifold subdivision of sample values returns a vector of twice the size:
template<class T>
Svec<T>             subdivide(Svec<T> const & v)
{
    size_t              S = v.size();
    Svec<T>             ret; ret.reserve(S*2);
    for (size_t ii=0; ii<S; ++ii) {
        T const &           e = v[ii];
        ret.push_back(e);
        ret.push_back((e + v[(ii+1)%S]) / 2);
    }
    return ret;
}

template<class T,class U>
Svec<T>             lookupRs(Svec<std::pair<T,U>> const & table,U const & val)
{
    Svec<T>                 ret;
    for (auto const & row : table)
        if (row.second == val)
            ret.push_back(row.first);
    return ret;
}
template<class T,class U>
Sizes               lookupIndsL(Svec<std::pair<T,U>> const & table,T const & val)
{
    Sizes               ret;
    for (size_t ii=0; ii<table.size(); ++ii)
        if (table[ii].first == val)
            ret.push_back(ii);
    return ret;
}
template<class T,class U>
Sizes               lookupIndsR(Svec<std::pair<T,U>> const & table,U const & val)
{
    Sizes               ret;
    for (size_t ii=0; ii<table.size(); ++ii)
        if (table[ii].second == val)
            ret.push_back(ii);
    return ret;
}

// Throws if not found:
template<class T,class U>
U const &           lookupFirstL(Svec<std::pair<T,U>> const & table,T const & val)
{
    auto        it=table.begin();
    for (; it!=table.end(); ++it)
        if (it->first == val)
            return it->second;
    FGASSERT_FALSE;
    return it->second;          // avoid warning
}
template<class T,class U>
T const &           lookupFirstR(Svec<std::pair<T,U>> const & table,U const & val)
{
    auto        it=table.begin();
    for (; it!=table.end(); ++it)
        if (it->second == val)
            return it->first;
    FGASSERT_FALSE;
    return it->first;           // avoid warning
}

// BASIC_STRING:

template<typename T>
bool                contains(std::basic_string<T> const & str,T ch)
{
    return (str.find(ch) != std::basic_string<T>::npos);
}

template<typename T>
bool                containsSubstr(std::basic_string<T> const & str,std::basic_string<T> const & pattern)
{
    return (str.find(pattern) != std::basic_string<T>::npos);
}

template<typename T>
bool                containsSubstr(std::basic_string<T> const & str,T const * pattern_c_str)
{
    return containsSubstr(str,std::basic_string<T>(pattern_c_str));
}

template<typename T>
std::basic_string<T> cHead(std::basic_string<T> const & str,size_t size)
{
    FGASSERT(size <= str.size());
    return std::basic_string<T>(str.begin(),str.begin()+size);
}

template<typename T>
std::basic_string<T> cTail(std::basic_string<T> const & str,size_t size)
{
    FGASSERT(size <= str.size());
    return std::basic_string<T>(str.end()-size,str.end());
}

template<class T>
std::basic_string<T> cSubstr(std::basic_string<T> const & str,size_t start,size_t size)
{
    FGASSERT(start+size <= str.size());
    return  std::basic_string<T>(str.begin()+start,str.begin()+start+size);
}

// Returns at least size 1, with 1 additional for each split element:
template<class T>
Svec<std::basic_string<T> > splitAtChar(std::basic_string<T> const & str,T ch)
{
    Svec<std::basic_string<T> >  ret;
    std::basic_string<T>                ss;
    for(T c : str) {
        if (c == ch) {
            ret.push_back(ss);
            ss.clear();
        }
        else
            ss.push_back(c);
    }
    ret.push_back(ss);
    return ret;
}

template<class T>
bool                beginsWith(std::basic_string<T> const & base,std::basic_string<T> const & pattern)
{
    return (base.rfind(pattern,0) != std::basic_string<T>::npos);
}

template<class T>
bool                beginsWith(std::basic_string<T> const & base,T const * pattern_c_str)
{
    return (base.rfind(pattern_c_str,0) != std::basic_string<T>::npos);
}

template<class T>
bool                endsWith(std::basic_string<T> const & str,std::basic_string<T> const & pattern)
{
    if (pattern.size() > str.size())
        return false;
    return (str.find(pattern,str.size()-pattern.size()) != std::basic_string<T>::npos);
}

template<class T>
bool                endsWith(std::basic_string<T> const & str,T const * pattern_c_str)
{
    return endsWith(str,std::basic_string<T>(pattern_c_str));
}

// STRING

// std::to_string can cause ambiguous call errors and doesn't let you adjust precision:
template<class T>
String              toStr(T const & val)
{
    std::ostringstream   msg;
    msg << val;
    return msg.str();
}

template<>
inline String       toStr(String const & str) {return str; }

// ostringstream defaults to stupidly little precision. Default to full:
template<>
inline String       toStr(float const & val)
{
    std::ostringstream   msg;
    msg << std::setprecision(7) << val;
    return msg.str();
}
template<>
inline String       toStr(double const & val)
{
    std::ostringstream   msg;
    msg << std::setprecision(17) << val;
    return msg.str();
}

template<class T>
Strings             toStrs(Svec<T> const & v)
{
    Strings         ret;
    ret.reserve(v.size());
    for (T const & e : v)
        ret.push_back(toStr(e));
    return ret;
}
template<class T,class U>
Strings             toStrs(T const & t,U const & u) {return {toStr(t),toStr(u)}; }
template<class T,class U,class V>
Strings             toStrs(T const & t,U const & u,V const & v) {return {toStr(t),toStr(u),toStr(v)}; }
template<class T,class U,class V,class W>
Strings             toStrs(T const & t,U const & u,V const & v,W const & w) {return {toStr(t),toStr(u),toStr(v),toStr(w)}; }

// THREAD DISPATCHER

typedef Svec<std::thread>   Threads;

// Simple blocking thread dispatcher - limits running threads to hardware capacity.
struct      ThreadDispatcher
{
    ThreadDispatcher()
    {
        threads.reserve(std::thread::hardware_concurrency());
        dones.reserve(std::thread::hardware_concurrency());
    }
    ~ThreadDispatcher() {finish(); }      // thread terminates if destructed before join()

    void            dispatch(std::function<void()> const & fn);
    void            finish();

private:
    Threads         threads;
    // thread provides no non-blocking way if testing if it's done so use flags.
    // vector requires copyable which atomic is not so use shared pointer to flags.
    Svec<Sptr<std::atomic<bool>>>   dones;      // 1-1 with above

    void            worker(Sfun<void()> const & fn,Sptr<std::atomic<bool> > done);
};

template<class T>
std::ostream &
operator<<(std::ostream & os,const std::set<T> & v)
{
    os << "{";
    for (typename std::set<T>::const_iterator it=v.begin(); it != v.end(); ++it)
        os << *it << ",";
    return os << "}";
}

// Useful in functional contexts where 's' is already an expression:
template<class T,class U>
inline
std::vector<T>
setToSvec(const std::set<T,U> & s)
{return std::vector<T>(s.begin(),s.end()); }

template<class T>
inline
std::set<T>
svecToSet(const std::vector<T> & v)
{return std::set<T>(v.begin(),v.end()); }

template<class T,class Lt>
inline
bool
contains(const std::set<T,Lt> & s,T const & v)
{return (s.find(v) != s.end()); }

// Returns true if the intersect of s0 and s1 is non-empty. Loop is through s1 so prefer s0 for the larger set.
template<class T>
bool
containsAny(const std::set<T> & s0,const std::set<T> & s1)
{
    for (auto it=s1.begin(); it != s1.end(); ++it)
        if (contains(s0,*it))
            return true;
    return false;
}

// Returns true if s0 contains all elements of s1. Loop is through s1 so prefer s0 for the larger set.
template<class T>
bool
containsAll(const std::set<T> & s0,const std::set<T> & s1)
{
    for (auto it=s1.begin(); it != s1.end(); ++it)
        if (!contains(s0,*it))
            return false;
    return true;
}

template<class T>
void
cUnion_(std::set<T> & s0,const std::set<T> & s1)
{s0.insert(s1.begin(),s1.end()); }

template<class T>
void
cUnion_(std::set<T> & s0,const std::vector<T> & s1)
{s0.insert(s1.begin(),s1.end()); }

template<class T>
std::set<T>
cUnion(const std::set<T> & s0,const std::set<T> & s1)
{
    // WTF is with set_union ...
    std::set<T>     ret = s0;
    ret.insert(s1.begin(),s1.end());
    return ret;
}

// std::set_intersection is stupidly complex.
// Loop is through s1 so prefer s0 for the larger set.
template<class T>
std::set<T>
cIntersect(const std::set<T> & s0,const std::set<T> & s1)
{
    std::set<T>         ret;
    for (T const & s : s1)
        if (contains(s0,s))
            ret.insert(s);
    return ret;
}

template<class T>
std::set<T>
operator+(std::set<T> lhs,const std::set<T> & rhs)
{
    lhs.insert(rhs.begin(),rhs.end());
    return lhs;
}

// set_difference is ridiculously verbose:
template<class T>
std::set<T>
operator-(const std::set<T> & lhs,const std::set<T> & rhs)
{
    std::set<T>         ret;
    for (T const & l : lhs)
        if (!contains(rhs,l))
            ret.insert(l);
    return ret;
}

// Arithmetic notation is a nice short-hand for union:
template<class T>
void
operator+=(std::set<T> & l,const std::set<T> & r)
{l.insert(r.begin(),r.end()); }

template<class T>
void
operator+=(std::set<T> & l,const std::vector<T> & r)
{l.insert(r.begin(),r.end()); }

// In case you prefer arithmetic notation for all:
template<class T>
void
operator+=(std::set<T> & l,T const & r)
{l.insert(r); }

template<class T>
void
operator-=(std::set<T> & lhs,const std::set<T> & rhs)
{
    for (T const & r : rhs) {
        auto it = lhs.find(r);
        if (it != lhs.end())
            lhs.erase(it);
    }
}

template<class T>
void
operator-=(std::set<T> & lhs,T const & rhs)
{
    auto it = lhs.find(rhs);
    if (it != lhs.end())
        lhs.erase(it);
}

template<class T>
void
operator-=(std::set<T> & lhs,const std::vector<T> & rhs)
{
    for (T const & r : rhs) {
        auto it = lhs.find(r);
        if (it != lhs.end())
            lhs.erase(it);
    }
}

template<class T>
std::set<T>
fgInsert(const std::set<T> & s,T const & v)
{
    std::set<T>         ret = s;
    ret.insert(v);
    return ret;
}

template<class T>
bool
contains(const std::unordered_set<T> & s,T const & v)
{
    return (s.find(v) != s.end());
}

template<class Key,class Val>
std::map<Key,Val>   operator+(std::map<Key,Val> const & lhs,std::map<Key,Val> const & rhs)
{
    std::map<Key,Val>    ret = lhs;
    ret.insert(rhs.begin(),rhs.end());
    return ret;
}

template<class Key,class Val>
void                operator+=(std::map<Key,Val> & lhs,std::map<Key,Val> const & rhs)
{lhs.insert(rhs.begin(),rhs.end()); }

template<class Key,class Val,class Lt>
bool                contains(const std::map<Key,Val,Lt> & map,const Key & key)
{return (map.find(key) != map.end()); }

// Accumulate weights of type W for different values of type T in map<T,W>.
// Useful for frequency maps or weight maps:
template<class T,class W>
void                mapAccWeight_(std::map<T,W> & map,T const & val,W weight=W(1))
{
    auto    it = map.find(val);
    if (it == map.end())
        map[val] = weight;
    else
        it->second += weight;
}

// As above but accumulate all weights in 'rhs' to 'lhs':
template<class T,class W>
void                mapAccWeight_(std::map<T,W> & lhs,const std::map<T,W> & rhs)
{
    for (auto const & it : rhs)
        mapAccWeight_(lhs,it.first,it.second);
}

}

#endif
