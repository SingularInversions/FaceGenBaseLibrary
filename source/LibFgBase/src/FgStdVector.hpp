//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
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

template<class T>
using Svec = std::vector<T>;

typedef Svec<bool>              Bools;
typedef Svec<char>              Chars;
typedef Svec<signed char>       Schars;
typedef Svec<unsigned char>     Uchars;
typedef Svec<int>               Ints;
typedef Svec<unsigned int>      Uints;
typedef Svec<size_t>            Sizes;
typedef Svec<float>             Floats;
typedef Svec<double>            Doubles;

typedef Svec<Doubles>           Doubless;
typedef Svec<Floats>            Floatss;
typedef Svec<Ints>              Intss;
typedef Svec<Uints>             Uintss;
typedef Svec<Sizes>             Sizess;

template<class T>
struct Traits<Svec<T> >
{
    typedef typename Traits<T>::Scalar                Scalar;
    typedef Svec<typename Traits<T>::Accumulator>   Accumulator;
    typedef Svec<typename Traits<T>::Floating>      Floating;
};

template<class T>
std::ostream &
operator<<(std::ostream & ss,const Svec<T> & vv)
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
Svec<To>
scast(Svec<From> const & vec)
{
    Svec<To>        ret;
    ret.reserve(vec.size());
    for (auto it=vec.cbegin(); it != vec.cend(); ++it)
        ret.push_back(scast<To,From>(*it));
    return ret;
}

template<class T>
Svec<T>
operator-(
    const Svec<T> &  lhs,
    const Svec<T> &  rhs)
{
    Svec<T>   ret;
    ret.reserve(lhs.size());
    FGASSERT(lhs.size() == rhs.size());
    for (size_t ii=0; ii<lhs.size(); ++ii)
        ret.push_back(lhs[ii] - rhs[ii]);
    return ret;
}

template<class T>
Svec<T>
operator+(
    const Svec<T> &  lhs,
    const Svec<T> &  rhs)
{
    Svec<T>       ret;
    FGASSERT(lhs.size() == rhs.size());
    ret.reserve(lhs.size());
    for (size_t ii=0; ii<lhs.size(); ++ii)
        ret.push_back(lhs[ii] + rhs[ii]);
    return ret;
}

template<class T>
void
operator-=(
    Svec<T> &         lhs,
    const Svec<T> &   rhs)
{
    FGASSERT(lhs.size() == rhs.size());
    for (size_t ii=0; ii<lhs.size(); ++ii)
        lhs[ii] -= rhs[ii];
}

template<class T>
void
operator+=(
    Svec<T> &         lhs,
    const Svec<T> &   rhs)
{
    FGASSERT(lhs.size() == rhs.size());
    for (size_t ii=0; ii<lhs.size(); ++ii)
        lhs[ii] += rhs[ii];
}

template<class T,class U>
Svec<T>
operator*(
    const Svec<T> &   lhs,
    U                   rhs)    // Different type useful for eg. Svec<FgVect> * float
{
    Svec<T>   ret;
    ret.reserve(lhs.size());
    for (size_t ii=0; ii<lhs.size(); ++ii)
        ret.push_back(lhs[ii] * rhs);
    return ret;
}

template<class T,class U>
Svec<T>
operator/(
    const Svec<T> &   lhs,
    U                   rhs)
{
    Svec<T>   ret;
    ret.reserve(lhs.size());
    for (size_t ii=0; ii<lhs.size(); ++ii)
        ret.push_back(lhs[ii] / rhs);
    return ret;
}

template<class T,class U>
void
operator*=(
    Svec<T> &     lhs,
    U               rhs)        // Different type useful for eg. Svec<FgVect> * float
{
    for (size_t ii=0; ii<lhs.size(); ++ii)
        lhs[ii] *= rhs;
}

template<class T,class U>
void
operator/=(
    Svec<T> &     lhs,
    U               rhs)
{
    for (size_t ii=0; ii<lhs.size(); ++ii)
        lhs[ii] /= rhs;
}

// Acts just like bool for use with vector but avoids use of broken
// Svec<bool> specialization:
struct FgBool
{
    uchar   m;
    FgBool() : m(0) {}
    FgBool(bool v) : m(v ? 1 : 0) {}
    operator bool () const {return (m > 0); }
};
typedef Svec<FgBool>  FgBools;

// Construction:

template<class T>
Svec<T>
fgSvec(const T & v1)
{return Svec<T>(1,v1); }

template<class T>
Svec<T>
fgSvec(const T & v1,const T & v2)
{
    Svec<T>   vec;
    vec.reserve(2);
    vec.push_back(v1); vec.push_back(v2);
    return vec;
}
template<class T>
Svec<T>
fgSvec(const T & v1,const T & v2,const T & v3)
{
    Svec<T>   vec;
    vec.reserve(3);
    vec.push_back(v1); vec.push_back(v2); vec.push_back(v3);
    return vec;
}
template<class T>
Svec<T>
fgSvec(const T & v0,const T & v1,const T & v2,const T & v3)
{
    Svec<T>   vec;
    vec.reserve(4);
    vec.push_back(v0); vec.push_back(v1); vec.push_back(v2); vec.push_back(v3);
    return vec;
}

template<class T>
Svec<T>
fgSvec(const T & v0,const T & v1,const T & v2,const T & v3,const T & v4)
{
    Svec<T> vec;
    vec.reserve(5);
    vec.push_back(v0); vec.push_back(v1); vec.push_back(v2); vec.push_back(v3); vec.push_back(v4);
    return vec;
}

template<class T>
Svec<T>
fgSvec(const T & v0,const T & v1,const T & v2,const T & v3,const T & v4,const T & v5)
{
    Svec<T> vec;
    vec.reserve(6);
    vec.push_back(v0); vec.push_back(v1); vec.push_back(v2); vec.push_back(v3); vec.push_back(v4);
    vec.push_back(v5);
    return vec;
}

template<class T>
Svec<T>
fgSvec(const T & v0,const T & v1,const T & v2,const T & v3,const T & v4,const T & v5,const T & v6)
{
    Svec<T> vec;
    vec.reserve(7);
    vec.push_back(v0); vec.push_back(v1); vec.push_back(v2); vec.push_back(v3); vec.push_back(v4);
    vec.push_back(v5); vec.push_back(v6);
    return vec;
}

template<class T>
Svec<T>
fgSvec(const T & v0,const T & v1,const T & v2,const T & v3,const T & v4,const T & v5,
         const T & v6,const T & v7)
{
    Svec<T> vec;
    vec.reserve(8);
    vec.push_back(v0); vec.push_back(v1); vec.push_back(v2); vec.push_back(v3); vec.push_back(v4);
    vec.push_back(v5); vec.push_back(v6); vec.push_back(v7);
    return vec;
}

template<class T>
Svec<T>
fgSvec(const T & v0,const T & v1,const T & v2,const T & v3,const T & v4,const T & v5,
         const T & v6,const T & v7,const T & v8)
{
    Svec<T> vec;
    vec.reserve(9);
    vec.push_back(v0); vec.push_back(v1); vec.push_back(v2); vec.push_back(v3); vec.push_back(v4);
    vec.push_back(v5); vec.push_back(v6); vec.push_back(v7); vec.push_back(v8);
    return vec;
}

template<class T>
Svec<T>
fgSvec(const T & v0,const T & v1,const T & v2,const T & v3,const T & v4,const T & v5,
       const T & v6,const T & v7,const T & v8,const T & v9)
{
    Svec<T> vec;
    vec.reserve(10);
    vec.push_back(v0); vec.push_back(v1); vec.push_back(v2); vec.push_back(v3); vec.push_back(v4);
    vec.push_back(v5); vec.push_back(v6); vec.push_back(v7); vec.push_back(v8); vec.push_back(v9);
    return vec;
}

template<class T>
Svec<T>
fgSvec(const T & v0,const T & v1,const T & v2,const T & v3,const T & v4,const T & v5,
       const T & v6,const T & v7,const T & v8,const T & v9,const T & vA)
{
    Svec<T> vec;
    vec.reserve(11);
    vec.push_back(v0); vec.push_back(v1); vec.push_back(v2); vec.push_back(v3); vec.push_back(v4);
    vec.push_back(v5); vec.push_back(v6); vec.push_back(v7); vec.push_back(v8); vec.push_back(v9);
    vec.push_back(vA);
    return vec;
}

template<class T,size_t N>
Svec<T>
fgSvec(const std::array<T,N> & v)
{return Svec<T>(v.cbegin(),v.cend()); }

// Generate elements using a function (eg. random generator):
template<class T>
Svec<T>
generate(std::function<T()> generator,size_t num)
{
    Svec<T>       ret;
    ret.reserve(num);
    for (size_t ii=0; ii<num; ++ii)
        ret.push_back(generator());
    return ret;
}

template<class From,class To>
Svec<To>
fgMapConvert(const Svec<From> & in)
{
    Svec<To>   ret;
    ret.reserve(in.size());
    for (typename Svec<From>::const_iterator it=in.begin(); it != in.end(); ++it)
        ret.push_back(To(*it));
    return ret;
}

// Linear interpolation between vectors of equal lengths:
template<class T>
void
interpolate_(Svec<T> const & v0,Svec<T> const & v1,float val,Svec<T> & ret)
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
Svec<T>
integrate(const Svec<T> & vec)
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
Svec<T>
cutSubvec(const Svec<T> & vec,size_t start,size_t size)
{
    FGASSERT(start+size <= vec.size());
    return  Svec<T>(vec.begin()+start,vec.begin()+start+size);
}

template<class T>
Svec<T>
cutHead(const Svec<T> & vec,size_t size)
{
    FGASSERT(size <= vec.size());
    return Svec<T>(vec.begin(),vec.begin()+size);
}

template<class T>
Svec<T>
cutRest(const Svec<T> & vec,size_t start=1)
{
    FGASSERT(start <= vec.size());      // Can be size zero
    return Svec<T>(vec.begin()+start,vec.end());
}

template<class T>
Svec<T>
cutTail(const Svec<T> & vec,size_t size)
{
    FGASSERT(size <= vec.size());
    return Svec<T>(vec.end()-size,vec.end());
}

// Append is the functional equivalent of push_back:
template<class T>
Svec<T>
append(const Svec<T> & vec,const T & val)
{
    Svec<T>       ret;
    ret.reserve(vec.size()+1);
    ret.insert(ret.begin(),vec.cbegin(),vec.cend());
    ret.push_back(val);
    return ret;
}

// Functional equivalent of insert at front:
template<class T>
Svec<T>
prepend(const T & val,const Svec<T> & vec)
{
    Svec<T>       ret;
    ret.reserve(vec.size()+1);
    ret.push_back(val);
    ret.insert(ret.end(),vec.begin(),vec.end());
    return ret;
}

// Concatenation in several forms:
template<class T>
inline void
cat_(Svec<T> & base,const Svec<T> & app)
{base.insert(base.end(),app.begin(),app.end()); }

template<class T>
Svec<T>
cat(const Svec<T> & v0,const Svec<T> & v1)
{
    Svec<T>   ret;
    ret.reserve(v0.size()+v1.size());
    ret.insert(ret.end(),v0.begin(),v0.end());
    ret.insert(ret.end(),v1.begin(),v1.end());
    return ret;
}
template<class T>
Svec<T>
cat(const Svec<T> & v0,const Svec<T> & v1,const Svec<T> & v2)
{
    Svec<T>  ret;
    ret.reserve(v0.size()+v1.size()+v2.size());
    ret.insert(ret.end(),v0.begin(),v0.end());
    ret.insert(ret.end(),v1.begin(),v1.end());
    ret.insert(ret.end(),v2.begin(),v2.end());
    return ret;
}
template<class T>
Svec<T>
cat(const Svec<T> & v0,const Svec<T> & v1,const Svec<T> & v2,const Svec<T> & v3)
{
    Svec<T>  ret;
    ret.reserve(v0.size()+v1.size()+v2.size()+v3.size());
    ret.insert(ret.end(),v0.begin(),v0.end());
    ret.insert(ret.end(),v1.begin(),v1.end());
    ret.insert(ret.end(),v2.begin(),v2.end());
    ret.insert(ret.end(),v3.begin(),v3.end());
    return ret;
}

// Functional version of vector::erase for single element:
template<class T>
Svec<T>
fgSnip(const Svec<T> & v,size_t idx)
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
Svec<T>
flatten(const Svec<Svec<T> > & v)
{
    Svec<T>       ret;
    size_t          sz = 0;
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
size_t
findFirstIdx(
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

// Throws if value not found:
template<class T,class U>
const T &
findFirst(
    Svec<T> const &     vec,
    U const &           val)     // Allow for T::operator==(U)
{
    auto const &    it = std::find(vec.begin(),vec.end(),val);
    FGASSERT(it != vec.end());
    return *it;
}

template<class T,class U>
size_t
findLastIdx(const Svec<T> & vec,const U & val)
{
    for (size_t ii=vec.size(); ii!=0; --ii)
        if (vec[ii-1] == val)
            return ii-1;
    return vec.size();
}

template<class T,class U>
bool
contains(const Svec<T> & vec,const U & val)     // Allows for T::operator==(U)
{
    for (size_t ii=0; ii<vec.size(); ++ii)
        if (vec[ii] == val)
            return true;
    return false;
}

template<class T,class U>
bool
containsAny(const Svec<T> & ctr,const Svec<U> & vals)     // Simple and slow: O(ctr * vals)
{
    for (size_t ii=0; ii<vals.size(); ++ii)
        if (contains(ctr,vals[ii]))
            return true;
    return false;
}

template<class T,class U>
bool
containsAll(const Svec<T> & ctr,const Svec<U> & vals)     // Simple and slow: O(ctr * vals)
{
    for (size_t ii=0; ii<vals.size(); ++ii)
        if (!contains(ctr,vals[ii]))
            return false;
    return true;
}

template<class T>
void
fgReplace_(Svec<T> & v,T a,T b)       // Replace each 'a' with 'b'
{
    for (size_t ii=0; ii<v.size(); ++ii)
        if (v[ii] == a)
            v[ii] = b;
}

template<class T>
Svec<T>
replaceAll(const Svec<T> & vec,const T & a,const T & b) // Replace each 'a' with 'b'
{
    Svec<T>       ret;
    ret.reserve(vec.size());
    for (const T & v : vec) {
        if (v == a)
            ret.push_back(b);
        else
            ret.push_back(v);
    }
    return ret;
}

// Returns at least size 1, with 1 additional for each split element:
template<class T>
Svec<Svec<T> >
splitAtChar(const Svec<T> & str,T ch)
{
    Svec<Svec<T> >    ret;
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

template<class T>
void
fgSetSubVec(Svec<T> & mod,size_t pos,const Svec<T> & sub)
{
    FGASSERT(sub.size() + pos <= mod.size());
    for (size_t ii=0; ii<sub.size(); ++ii)
        mod[pos+ii] = sub[ii];
}

template<class T>
bool
beginsWith(const Svec<T> & base,const Svec<T> & pattern)
{
    if (pattern.size() > base.size())
        return false;
    for (size_t ii=0; ii<pattern.size(); ++ii)
        if (pattern[ii] != base[ii])
            return false;
    return true;
}

template<class T>
bool
endsWith(const Svec<T> & base,const Svec<T> & pattern)
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

template<class T,class U>
void
scast_(const Svec<T> & lhs,Svec<U> & rhs)
{
    rhs.resize(lhs.size());
    for (size_t ii=0; ii<lhs.size(); ++ii)
        scast_(lhs[ii],rhs[ii]);
}

// Functional version of std::transform. Requires explicit template args even for 'Out' (don't know why):
template<class Out,class In>
Svec<Out>
mapf(Svec<In> const & in,std::function<Out(In const &)> const & func)
{
    Svec<Out>    ret;
    ret.reserve(in.size());
    for (size_t ii=0; ii<in.size(); ++ii)
        ret.push_back(func(in[ii]));
    return ret;
}

// As above but output is same type as input, which allows template args to be skipped:
template<class T>
Svec<T>
mapft(Svec<T> const & in,std::function<T(T const &)> const & func)
{
    Svec<T>    ret;
    ret.reserve(in.size());
    for (size_t ii=0; ii<in.size(); ++ii)
        ret.push_back(func(in[ii]));
    return ret;
}

// map transform = map using: Out Op::operator*(In). First template arg must be specified:
template<class Out,class In,class Op>
Svec<Out>
mapXf(Svec<In> const & in,Op const & op)
{
    Svec<Out>       ret;
    ret.reserve(in.size());
    for (size_t ii=0; ii<in.size(); ++ii)
        ret.push_back(op * in[ii]);
    return ret;
}

// map transform to same type = map using: T Op::operator*(T). Template types deduced:
template<class T,class Op>
Svec<T>
mapXft(Svec<T> const & in,Op const & op)
{
    Svec<T>     ret;
    ret.reserve(in.size());
    for (size_t ii=0; ii<in.size(); ++ii)
        ret.push_back(op * in[ii]);
    return ret;
}

template<class T,class U,class Op>
void
mapXf_(Svec<T> const & in,Op const & op,Svec<U> & out)
{
    out.resize(in.size());
    for (size_t ii=0; ii<in.size(); ++ii)
        out[ii] = op * in[ii];
}

template<class T,class Op>
void
mapXf_(Svec<T> & data,Op const & op)
{
    for (size_t ii=0; ii<data.size(); ++ii)
        data[ii] = op * data[ii];
}

template<class T>
Svec<T>
mapAbs(Svec<T> const & vec)
{
    Svec<T>     ret;
    ret.reserve(vec.size());
    for (T const & v : vec)
        ret.push_back(std::abs(v));
    return ret;
}

inline void cSum_(const Svec<double> & in,double & out)
{
    double      acc = out;
    for (double i : in)
        acc += i;
    out = acc;
}

// Sum into an existing accumulator:
template<class T>
void
cSum_(const Svec<T> & in,T & out)
{
    for (const T & i : in)
        out += i;
}

// NOTE: The value is accumulated in the templated type. Make a special purpose function
// if the accumulator type must be larger than the templated type:
template<class T>
T
cSum(const Svec<T> & v)
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
T
fgProduct(const Svec<T> & v)
{
    typedef typename Traits<T>::Accumulator Acc;
    Acc         acc(1);
    for (size_t ii=0; ii<v.size(); ++ii)
        acc *= Acc(v[ii]);
    return T(acc);
}

// The value is accumulated in the templated type. Make a special purpose function
// if the accumulator type must be larger than the templated type:
template<class T>
T
cMean(const Svec<T> & v)
{
    typedef typename Traits<T>::Scalar    S;
    static_assert(std::is_floating_point<S>::value,"Multiplication by a reciprocal");
    return cSum(v) * (S(1) / S(v.size()));
}

template<class T>
void
fgSubtract(
    const Svec<T> &  lhs,
    const Svec<T> &  rhs,
    Svec<T> &        res)
{
    FGASSERT(lhs.size() == rhs.size());
    res.resize(lhs.size());
    for (size_t ii=0; ii<lhs.size(); ++ii)
        res[ii] = lhs[ii] - rhs[ii];
}

template<class T>
void
fgMapAddConst_(Svec<T> & vec,const T & val)
{
    for (T & e : vec)
        e += val;
}

template<class T>
void
fgMapSubConst_(Svec<T> & vec,const T & val)
{
    for (T & e : vec)
        e -= val;
}

template<class T>
Svec<T>
fgMapAddConst(const Svec<T> & vec,const T & val)
{
    Svec<T>       ret;
    ret.reserve(vec.size());
    for (const T & v : vec)
        ret.push_back(v + val);
    return ret;
}

template<class T>
Svec<T>
fgMapSubConst(const Svec<T> & vec,const T & val)
{
    Svec<T>       ret;
    ret.reserve(vec.size());
    for (const T & v : vec)
        ret.push_back(v - val);
    return ret;
}

// Element-wise division:
template<class T>
Svec<T>
mapDiv(
    const Svec<T> &   lhs,
    const Svec<T> &   rhs)
{
    Svec<T>       ret;
    FGASSERT(lhs.size() == rhs.size());
    ret.reserve(lhs.size());
    for (size_t ii=0; ii<lhs.size(); ++ii)
        ret.push_back(lhs[ii]/rhs[ii]);
    return ret;
}

template<class T>
Svec<T>
fgMapAbs(const Svec<T> & v)
{
    Svec<T>  ret;
    ret.reserve(v.size());
    for (size_t ii=0; ii<v.size(); ++ii)
        ret.push_back(std::abs(v[ii]));
    return ret;
}

template<class T>
Svec<T>
fgMapSqr(const Svec<T> & v)
{
    Svec<T>  ret;
    ret.reserve(v.size());
    for (size_t ii=0; ii<v.size(); ++ii)
        ret.push_back(v[ii]*v[ii]);
    return ret;
}

template<class T>
size_t
cMinIdx(const Svec<T> & v)
{
    FGASSERT(!v.empty());
    size_t      ret = 0;
    for (size_t ii=1; ii<v.size(); ++ii)
        if (v[ii] < v[ret])
            ret = ii;
    return ret;
}

template<class T>
size_t
cMaxIdx(const Svec<T> & v)
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
size_t
cMaxIdx(const Svec<T> & v,const std::function<bool(const T & lhs,const T & rhs)> & lessThan)
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
void
fgSort_(Svec<T> & v)
{std::sort(v.begin(),v.end()); }

// Functional version of sorting whole vector:
template<class T>
Svec<T>
fgSort(const Svec<T> & v)
{
    Svec<T>  ret(v);
    std::sort(ret.begin(),ret.end());
    return ret;
}

template<class T>
bool
fgSortIndsLt(const T * v,size_t l,size_t r)
{return (v[l] < v[r]); }

// Return a list of the permuted indices instead of the sorted list itself:
template<class T>
Svec<size_t>
fgSortInds(const Svec<T> & v)
{
    Svec<size_t>  inds(v.size());
    for (size_t ii=0; ii<inds.size(); ++ii)
        inds[ii] = ii;
    if (!inds.empty())
        std::sort(inds.begin(),inds.end(),std::bind(fgSortIndsLt<T>,&v[0],std::placeholders::_1,std::placeholders::_2));
    return inds;
}

template<class T>
bool
fgSortIndsGt(const T * v,size_t l,size_t r)
{return (v[l] > v[r]); }

// Make use of a permuted indices list to re-order a list (or subset thereof):
template<class T>
Svec<T>
reorder(const Svec<T> & v,const Svec<uint> & inds)
{
    Svec<T>       ret;
    ret.reserve(inds.size());
    for (size_t ii=0; ii<inds.size(); ++ii)
        ret.push_back(v[inds[ii]]);
    return ret;
}

#ifdef FG_64
// Make use of a permuted indices list to re-order a list (or subset thereof):
template<class T>
Svec<T>
reorder(const Svec<T> & v,const Svec<size_t> & inds)
{
    Svec<T>       ret;
    ret.reserve(inds.size());
    for (size_t ii=0; ii<inds.size(); ++ii)
        ret.push_back(v[inds[ii]]);
    return ret;
}
#endif

template<class T>
bool
containsDuplicates(const Svec<T> & mustBeSorted)
{
    for (size_t ii=1; ii<mustBeSorted.size(); ++ii)
        if (mustBeSorted[ii] == mustBeSorted[ii-1])
            return true;
    return false;
}

// Removes duplicates from a sorted vector (I don't get std::unique):
template<class T>
Svec<T>
cUnique(const Svec<T> & sorted)
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

template<class T,class U>
Svec<U>
sliceMember(const Svec<T> & ts,U T::*m)
{
    Svec<U>       ret;
    ret.reserve(ts.size());
    for (size_t ii=0; ii<ts.size(); ++ii)
        ret.push_back(ts[ii].*m);
    return ret;
}

// Transpose a row-major contiguous array:
template<class T>
Svec<T>
fgTranspose(const Svec<T> & v,size_t wid,size_t hgt)
{
    Svec<T>       ret;
    FGASSERT(v.size() == wid*hgt);
    ret.reserve(v.size());
    for (size_t xx=0; xx<wid; ++xx) {
        size_t      idx = xx;
        for (size_t yy=0; yy<hgt; ++yy) {
            ret.push_back(v[idx]);
            idx += wid;
        }
    }
    return ret;
}

template<class T>
void
fgFill(Svec<T> & vec,T val)
{
    for (size_t ii=0; ii<vec.size(); ++ii)
        vec[ii] = val;
}

// Transpose a vector of vectors just like Python 'zip' on lists.
// All sub-vectors must have the same size():
template<class T>
Svec<Svec<T> >
transpose(const Svec<Svec<T> > & v)
{
    Svec<Svec<T> >  ret;
    if (!v.empty()) {
        size_t      sz = v[0].size();
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

// For this to work with lambdas the template type must be explicitly given with the function call:
template<class T>
Svec<T>
fgFilter(const Svec<T> & vals,const std::function<bool(const T & val)> & fnSelect)
{
    Svec<T>       ret;
    for (auto it=vals.begin(); it!=vals.end(); ++it)
        if (fnSelect(*it))
            ret.push_back(*it);
    return ret;
}

template<class T>
Svec<T>
fgFilter(const Svec<T> & in,const Svec<bool> & accept)
{
    Svec<T>       ret;
    FGASSERT(accept.size() == in.size());
    for (size_t ii=0; ii<in.size(); ++ii)
        if (accept[ii])
            ret.push_back(in[ii]);
    return ret;
}

template<class T>
Svec<size_t>
fgJaggedDims(const Svec<Svec<T> > & v)
{
    Svec<size_t>      ret(v.size());
    for (size_t ii=0; ii<ret.size(); ++ii)
        ret[ii] = v[ii].size();
    return ret;
}

template<class T>
Svec<Svec<T> >
fgJaggedVec(const Svec<size_t> & dims,const T & initVal)
{
    Svec<Svec<T> >      ret(dims.size());
    for (size_t ii=0; ii<ret.size(); ++ii)
        ret[ii].resize(dims[ii],initVal);
    return ret;
}

// Partition a vector into a jagged vector of the given jagged dimensions:
template<class T>
Svec<Svec<T> >
fgJaggedVec(const Svec<size_t> & dims,const Svec<T> & data)
{
    Svec<Svec<T> >      ret(dims.size());
    size_t                  idx = 0;
    for (size_t ii=0; ii<ret.size(); ++ii) {
        FGASSERT(data.size() >= idx + dims[ii]);
        ret[ii] = Svec<T>(data.begin()+idx,data.begin()+idx+dims[ii]);
        idx += dims[ii];
    }
    return ret;
}

template<class T>
Svec<Svec<T> >
fgVecOfVecs(size_t dim0,size_t dim1,const T & initVal)
{
    Svec<Svec<T> >      ret(dim0);
    for (size_t ii=0; ii<ret.size(); ++ii)
        ret[ii].resize(dim1,initVal);
    return ret;
}

template<class T>
Svec<T>
// Each vec in 'vs' must be the same size or one less, with the latter all at the end.
// Ie. the result of interleaving in order:
fgDeinterleave(const Svec<Svec<T> > & vs)
{
    Svec<T>           ret;
    if (!vs.empty()) {
        size_t          sz0 = vs[0].size();
        ret.reserve(vs.size()*sz0);             // Largest possible size of return value
        for (size_t ii=0; ii<sz0; ++ii) {
            size_t jj=0;
            for (; jj<vs.size(); ++jj) {
                if (vs[jj].size() <= ii)
                    break;                      // Don't return here for NRVO
                ret.push_back(vs[jj][ii]);
            }
            if (jj < vs.size())
                break;
        }
    }
    return ret;
}

template<class T>
Svec<const T *>
fgMapToConstPtrs(const Svec<Svec<T> > & v)  // Cannot contain any empty vectors
{
    Svec<const T *>   ret;
    ret.reserve(v.size());
    for (size_t ii=0; ii<v.size(); ++ii) {
        FGASSERT(!v[ii].empty());
        ret.push_back(&v[ii][0]);
    }
    return ret;
}

// Set intersection with vector containers; ignores duplicates returns intersection of uniques:
template<class T>
Svec<T>
fgSetIntersection(const Svec<T> & v0,const Svec<T> & v1)
{
    Svec<T>       ret;
    for (const T & v : v0) {
        if (contains(v1,v) && !contains(ret,v))
            ret.push_back(v);
    }
    return ret;
}

template<class T>
Svec<T>
fgSetIntersection(const Svec<Svec<T> > & vs)
{
    Svec<T>       ret;
    if (!vs.empty()) {
        ret = vs[0];
        for (size_t ii=1; ii<vs.size(); ++ii)
            ret = fgSetIntersection(ret,vs[ii]);
    }
    return ret;
}

// Multiset intersection with vector containers:
template<class T>
Svec<T>
fgIntersection(const Svec<T> & v0,Svec<T> v1)
{
    Svec<T>       ret;
    for (const T & v : v0) {
        auto        it = find(v1.begin(),v1.end(),v);
        if (it != v1.end()) {
            ret.push_back(v);
            v1.erase(it);
        }
    }
    return ret;
}

template<class T>
Svec<T>
fgIntersection(const Svec<Svec<T> > & vs)
{
    Svec<T>       ret;
    if (!vs.empty()) {
        ret = vs[0];
        for (size_t ii=1; ii<vs.size(); ++ii)
            ret = fgIntersection(vs[ii],ret);
    }
    return ret;
}

// Set union on vector containers, retaining lhs order then rhs order:
template<class T>
void
fgSetwiseAdd_(Svec<T> & lhs,const Svec<T> & rhs)
{
    for (const T & r : rhs)
        if (!contains(lhs,r))
            lhs.push_back(r);
}

// Set union on vector containers, retaining lhs order then rhs order:
template<class T>
Svec<T>
fgSetwiseAdd(Svec<T> lhs,const Svec<T> & rhs)
{
    fgSetwiseAdd_(lhs,rhs);
    return lhs;
}

// Set subtraction on vector containers (lhs retains ordering):
template<class T>
Svec<T>
fgSetwiseSubtract(const Svec<T> & lhs,const Svec<T> & rhs)
{
    Svec<T>       ret;
    for (const T & l : lhs) {
        if (!contains(rhs,l))
            ret.push_back(l);
    }
    return ret;
}

// Multiset subtraction on vector containers (lhs retains ordering):
template<class T>
Svec<T>
fgMultisetWiseSubtract(const Svec<T> & lhs,Svec<T> rhs)
{
    Svec<T>       ret;
    for (const T & l : lhs) {
        bool        found = false;
        for (auto it=rhs.begin(); it!=rhs.end(); ++it) {
            if (l == *it) {
                rhs.erase(it);
                found = true;
                break;
            }
        }
        if (!found)
            ret.push_back(l);
    }
    return ret;
}

template<class T>
Svec<T>
fgReverse(Svec<T> v)
{
    std::reverse(v.begin(),v.end());
    return v;
}

}

#endif
