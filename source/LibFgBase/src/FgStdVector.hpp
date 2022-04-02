//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Extensions to std::vector
//

#ifndef FGSTDVECTOR_HPP
#define FGSTDVECTOR_HPP

#include "FgStdLibs.hpp"
#include "FgTypes.hpp"
#include "FgDiagnostics.hpp"
#include "FgOut.hpp"

namespace Fg {

// Frequently used shorthands:

template<class T>   using Svec = std::vector<T>;
template<class T>   using Ptrs = std::vector<T const *>;

typedef Svec<bool>              Bools;
typedef Svec<char>              Chars;
typedef Svec<signed char>       Schars;
typedef Svec<unsigned char>     Uchars;
typedef Svec<int>               Ints;
typedef Svec<uint>              Uints;
typedef Svec<size_t>            Sizes;
typedef Svec<int64>             Int64s;
typedef Svec<uint64>            Uint64s;
typedef Svec<float>             Floats;
typedef Svec<double>            Doubles;

typedef Svec<Bools>             Boolss;
typedef Svec<Doubles>           Doubless;
typedef Svec<Floats>            Floatss;
typedef Svec<Ints>              Intss;
typedef Svec<Uints>             Uintss;
typedef Svec<Sizes>             Sizess;

template<class T>
struct Traits<Svec<T>>
{
    typedef typename Traits<T>::Scalar                Scalar;
    typedef Svec<typename Traits<T>::Accumulator>   Accumulator;
    typedef Svec<typename Traits<T>::Floating>      Floating;
};

// Element list construction when type must be explicit and {} cannot be used:

template<class T>
inline Svec<T>      svec(T const & v0)
{return Svec<T>{v0}; }

template<class T>
inline Svec<T>      svec(T const & v0,T const & v1)
{return Svec<T>{v0,v1}; }

template<class T>
inline Svec<T>      svec(T const & v0,T const & v1,T const & v2)
{return Svec<T>{v0,v1,v2}; }

template<class T>
inline Svec<T>      svec(T const & v0,T const & v1,T const & v2,T const & v3)
{return Svec<T>{v0,v1,v2,v3}; }

template<class T>
inline Svec<T>      svec(T const & v0,T const & v1,T const & v2,T const & v3,T const & v4)
{return Svec<T>{v0,v1,v2,v3,v4}; }

template<class T>
inline Svec<T>      svec(T const & v0,T const & v1,T const & v2,T const & v3,T const & v4,T const & v5)
{return Svec<T>{v0,v1,v2,v3,v4,v5}; }

template<class T>
inline Svec<T>
svec(T const & v0,T const & v1,T const & v2,T const & v3,T const & v4,T const & v5,T const & v6)
{return Svec<T>{v0,v1,v2,v3,v4,v5,v6}; }

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

template<typename To,typename From>
Svec<To>            mapCast(Svec<From> const & vec)
{
    Svec<To>        ret; ret.reserve(vec.size());
    for (From const & v : vec)
        ret.push_back(static_cast<To>(v));
    return ret;
}

template<typename To,typename From>
void                deepCast_(Svec<To> const & from,Svec<From> & to)
{
    to.resize(from.size());
    for (size_t ii=0; ii<to.size(); ++ii)
        deepCast_(from[ii],to[ii]);
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
Svec<T>             generateSvec(size_t num,C const & callable)
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
void                interpolate_(Svec<T> const & v0,Svec<T> const & v1,float val,Svec<T> & ret)
{
    size_t              sz = v0.size();
    FGASSERT(v1.size() == sz);
    ret.resize(sz);
    float               omv = 1.0f - val;
    for (size_t ii=0; ii<sz; ++ii)
        ret[ii] = round<T>(scast<float>(v0[ii]) * omv + scast<float>(v1[ii]) * val);
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
// Concatenation in several forms:
template<class T>
inline void         cat_(Svec<T> & base,Svec<T> const & app) {base.insert(base.end(),app.begin(),app.end()); }
// see 'flatten' instead of cat(Svec<Svec<T>>)
template<class T>
Svec<T>             cat(Svec<T> const & v0,Svec<T> const & v1)
{
    Svec<T>   ret;
    ret.reserve(v0.size()+v1.size());
    ret.insert(ret.end(),v0.begin(),v0.end());
    ret.insert(ret.end(),v1.begin(),v1.end());
    return ret;
}
template<class T>
Svec<T>             cat(Svec<T> const & v0,Svec<T> const & v1,Svec<T> const & v2)
{
    Svec<T>  ret;
    ret.reserve(v0.size()+v1.size()+v2.size());
    ret.insert(ret.end(),v0.begin(),v0.end());
    ret.insert(ret.end(),v1.begin(),v1.end());
    ret.insert(ret.end(),v2.begin(),v2.end());
    return ret;
}
template<class T>
Svec<T>             cat(Svec<T> const & v0,Svec<T> const & v1,Svec<T> const & v2,Svec<T> const & v3)
{
    Svec<T>  ret;
    ret.reserve(v0.size()+v1.size()+v2.size()+v3.size());
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
double              cMag(Svec<T> const & v)              // Sum of squared magnitude values:
{
    double      ret(0);
    for (size_t ii=0; ii<v.size(); ++ii)
        ret += cMag(v[ii]);
    return ret;
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
void                cSort_(Svec<T> & v) {std::sort(v.begin(),v.end()); }

// Functional version of sorting whole vector:
template<class T>
Svec<T>             cSort(Svec<T> const & v)
{
    Svec<T>             ret(v);
    std::sort(ret.begin(),ret.end());
    return ret;
}
// ... and with predicate:
template<class T,class P>
Svec<T>             cSort(Svec<T> const & v,P const & pred)
{
    Svec<T>             ret(v);
    std::sort(ret.begin(),ret.end(),pred);
    return ret;
}
// Return array of indices such that the input elements are sorted:
// (ie. A mapping from the NEW order to the ORIGINAL order):
template<class T>
Sizes               sortInds(Svec<T> const & v)
{
    Sizes       inds(v.size());
    for (size_t ii=0; ii<inds.size(); ++ii)
        inds[ii] = ii;
    std::sort(inds.begin(),inds.end(),[&](size_t l,size_t r){return (v[l]<v[r]); });
    return inds;
}
// Permute / select / reorder / re-order a list or subset thereof in the order of the given indices.
template<class T,class U,
    // Can be uint or uint64, and on some platforms size_t is different from these:
    FG_ENABLE_IF(U,is_unsigned),
    FG_ENABLE_IF(U,is_integral)
>
Svec<T>             permute(Svec<T> const & v,Svec<U> const & inds)
{
    Svec<T>             ret; ret.reserve(inds.size());
    for (size_t ii=0; ii<inds.size(); ++ii)
        ret.push_back(v[inds[ii]]);
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
// Transpose a vector of vectors just like Python 'zip' on lists.
// All sub-vectors must have the same size():
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

}

#endif
