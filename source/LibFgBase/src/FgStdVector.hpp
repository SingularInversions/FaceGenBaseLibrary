//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
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
operator<<(std::ostream & ss,Svec<T> const & vv)
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
mapCast(Svec<From> const & vec)
{
    Svec<To>        ret;
    ret.reserve(vec.size());
    for (From const & v : vec)
        ret.push_back(scast<To,From>(v));
    return ret;
}

template<typename To,typename From>
void
deepCast_(Svec<To> const & from,Svec<From> & to)
{
    to.resize(from.size());
    for (size_t ii=0; ii<to.size(); ++ii)
        deepCast_(from[ii],to[ii]);
}

template<class T>
Svec<T>
operator-(
    Svec<T> const &  lhs,
    Svec<T> const &  rhs)
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
    Svec<T> const &  lhs,
    Svec<T> const &  rhs)
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
    Svec<T> const &   rhs)
{
    FGASSERT(lhs.size() == rhs.size());
    for (size_t ii=0; ii<lhs.size(); ++ii)
        lhs[ii] -= rhs[ii];
}

template<class T>
void
operator+=(
    Svec<T> &         lhs,
    Svec<T> const &   rhs)
{
    FGASSERT(lhs.size() == rhs.size());
    for (size_t ii=0; ii<lhs.size(); ++ii)
        lhs[ii] += rhs[ii];
}

template<class T,class U>
Svec<T>
operator*(
    Svec<T> const &   lhs,
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
    Svec<T> const &   lhs,
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

// Construction:

template<class T>
Svec<T>
svec(T const & v1)
{return Svec<T>(1,v1); }

template<class T>
Svec<T>
svec(T const & v1,T const & v2)
{
    Svec<T>   vec;
    vec.reserve(2);
    vec.push_back(v1); vec.push_back(v2);
    return vec;
}
template<class T>
Svec<T>
svec(T const & v1,T const & v2,T const & v3)
{
    Svec<T>   vec;
    vec.reserve(3);
    vec.push_back(v1); vec.push_back(v2); vec.push_back(v3);
    return vec;
}
template<class T>
Svec<T>
svec(T const & v0,T const & v1,T const & v2,T const & v3)
{
    Svec<T>   vec;
    vec.reserve(4);
    vec.push_back(v0); vec.push_back(v1); vec.push_back(v2); vec.push_back(v3);
    return vec;
}

template<class T>
Svec<T>
svec(T const & v0,T const & v1,T const & v2,T const & v3,T const & v4)
{
    Svec<T> vec;
    vec.reserve(5);
    vec.push_back(v0); vec.push_back(v1); vec.push_back(v2); vec.push_back(v3); vec.push_back(v4);
    return vec;
}

template<class T>
Svec<T>
svec(T const & v0,T const & v1,T const & v2,T const & v3,T const & v4,T const & v5)
{
    Svec<T> vec;
    vec.reserve(6);
    vec.push_back(v0); vec.push_back(v1); vec.push_back(v2); vec.push_back(v3); vec.push_back(v4);
    vec.push_back(v5);
    return vec;
}

template<class T>
Svec<T>
svec(T const & v0,T const & v1,T const & v2,T const & v3,T const & v4,T const & v5,T const & v6)
{
    Svec<T> vec;
    vec.reserve(7);
    vec.push_back(v0); vec.push_back(v1); vec.push_back(v2); vec.push_back(v3); vec.push_back(v4);
    vec.push_back(v5); vec.push_back(v6);
    return vec;
}

template<class T>
Svec<T>
svec(T const & v0,T const & v1,T const & v2,T const & v3,T const & v4,T const & v5,
         T const & v6,T const & v7)
{
    Svec<T> vec;
    vec.reserve(8);
    vec.push_back(v0); vec.push_back(v1); vec.push_back(v2); vec.push_back(v3); vec.push_back(v4);
    vec.push_back(v5); vec.push_back(v6); vec.push_back(v7);
    return vec;
}

template<class T>
void
mapAsgn_(Svec<T> & vec,T val)
{std::fill(vec.begin(),vec.end(),val); }

// Generate elements using a function (eg. random generator):
template<class T>
Svec<T>
generate(size_t num,std::function<T()> const & generator)
{
    Svec<T>       ret;
    ret.reserve(num);
    for (size_t ii=0; ii<num; ++ii)
        ret.push_back(generator());
    return ret;
}

// Generate elements using a function that takes the index as an argument:
template<class T>
Svec<T>
generateIdx(size_t num,std::function<T(size_t)> const & generator)
{
    Svec<T>       ret;
    ret.reserve(num);
    for (size_t ii=0; ii<num; ++ii)
        ret.push_back(generator(ii));
    return ret;
}

template<class T,class U>
Svec<T>
mapConstruct(Svec<U> const & us)
{
    Svec<T>         ret;
    ret.reserve(us.size());
    for (U const & u : us)
        ret.emplace_back(u);
    return ret;
}

template<class From,class To>
Svec<To>
mapConvert(const Svec<From> & in)
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
integrate(Svec<T> const & vec)
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
cSubvec(Svec<T> const & vec,size_t start,size_t size)
{
    FGASSERT(start+size <= vec.size());
    return  Svec<T>(vec.begin()+start,vec.begin()+start+size);
}

// Return the first elements of 'vec' up to 'size', if present:
template<class T>
Svec<T>
cHead(Svec<T> const & vec,size_t size)
{
    size_t      sz = std::min(vec.size(),size);
    return Svec<T>(vec.begin(),vec.begin()+sz);
}

template<class T>
Svec<T>
cRest(Svec<T> const & vec,size_t start=1)
{
    FGASSERT(start <= vec.size());      // Can be size zero
    return Svec<T>(vec.begin()+start,vec.end());
}

template<class T>
Svec<T>
cTail(Svec<T> const & vec,size_t size)
{
    FGASSERT(size <= vec.size());
    return Svec<T>(vec.end()-size,vec.end());
}

// Append is the functional equivalent of push_back:
template<class T>
Svec<T>
append(Svec<T> const & vec,T const & val)
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
prepend(T const & val,Svec<T> const & vec)
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
cat_(Svec<T> & base,Svec<T> const & app)
{base.insert(base.end(),app.begin(),app.end()); }

template<class T>
Svec<T>
cat(Svec<T> const & v0,Svec<T> const & v1)
{
    Svec<T>   ret;
    ret.reserve(v0.size()+v1.size());
    ret.insert(ret.end(),v0.begin(),v0.end());
    ret.insert(ret.end(),v1.begin(),v1.end());
    return ret;
}
template<class T>
Svec<T>
cat(Svec<T> const & v0,Svec<T> const & v1,Svec<T> const & v2)
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
cat(Svec<T> const & v0,Svec<T> const & v1,Svec<T> const & v2,Svec<T> const & v3)
{
    Svec<T>  ret;
    ret.reserve(v0.size()+v1.size()+v2.size()+v3.size());
    ret.insert(ret.end(),v0.begin(),v0.end());
    ret.insert(ret.end(),v1.begin(),v1.end());
    ret.insert(ret.end(),v2.begin(),v2.end());
    ret.insert(ret.end(),v3.begin(),v3.end());
    return ret;
}

// Avoid re-typeing argument for vector::erase of a single element:
template<class T>
void
snip_(Svec<T> & v,size_t idx)
{v.erase(v.begin()+idx); }

// Functional version of vector::erase for single element:
template<class T>
Svec<T>
snip(Svec<T> const & v,size_t idx)
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
flatten(Svec<Svec<T> > const & v)
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
T const &
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
findLastIdx(Svec<T> const & vec,const U & val)
{
    for (size_t ii=vec.size(); ii!=0; --ii)
        if (vec[ii-1] == val)
            return ii-1;
    return vec.size();
}

// Returns first instance whose given member matches 'val'. Throws if none.
template<class T,class U>
T
findByMember(Svec<T> const & vec,U T::*mbr,U const & val)
{
    for (T const & elm : vec)
        if (elm.*mbr == val)
            return elm;
    FGASSERT_FALSE;
    return T{};
}

template<class T,class U>
bool
contains(Svec<T> const & vec,const U & val)     // Allows for T::operator==(U)
{
    for (size_t ii=0; ii<vec.size(); ++ii)
        if (vec[ii] == val)
            return true;
    return false;
}

template<class T,class U>
bool
containsAny(Svec<T> const & ctr,const Svec<U> & vals)     // Simple and slow: O(ctr * vals)
{
    for (size_t ii=0; ii<vals.size(); ++ii)
        if (contains(ctr,vals[ii]))
            return true;
    return false;
}

template<class T,class U>
bool
containsAll(Svec<T> const & ctr,const Svec<U> & vals)     // Simple and slow: O(ctr * vals)
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
replaceAll(Svec<T> const & vec,T const & a,T const & b) // Replace each 'a' with 'b'
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
Svec<Svec<T> >
splitAtChar(Svec<T> const & str,T ch)
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

// Overwrite the subset of 'target' starting at 'startPos' with the values from 'data':
template<class T>
void
inject_(Svec<T> const & data,size_t startPos,Svec<T> & target)
{
    FGASSERT(data.size() + startPos <= target.size());
    copy(data.begin(),data.end(),target.begin()+startPos);
}

// Returns the result of overwriting the subset of 'target' starting at 'startPos' with the values from 'data':
template<class T>
Svec<T>
inject(Svec<T> const & data,size_t startPos,Svec<T> const & target)
{
    FGASSERT(data.size() + startPos <= target.size());
    Svec<T>                 ret = target;
    copy(data.begin(),data.end(),ret.begin()+startPos);
    return ret;
}

template<class T>
bool
beginsWith(Svec<T> const & base,Svec<T> const & pattern)
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
endsWith(Svec<T> const & base,Svec<T> const & pattern)
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
void
cSum_(Svec<T> const & in,T & out)
{
    for (T const & i : in)
        out += i;
}

// NOTE: The value is accumulated in the templated type. Make a special purpose function
// if the accumulator type must be larger than the templated type:
template<class T>
T
cSum(Svec<T> const & v)
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
cProduct(Svec<T> const & v)
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
cMean(Svec<T> const & v)
{
    typedef typename Traits<T>::Scalar    S;
    static_assert(std::is_floating_point<S>::value,"Multiplication by a reciprocal");
    return cSum(v) * (S(1) / S(v.size()));
}

template<class T>
double
cMag(Svec<T> const & v)              // Sum of squared magnitude values:
{
    double      ret(0);
    for (size_t ii=0; ii<v.size(); ++ii)
        ret += cMag(v[ii]);
    return ret;
}

template<class T>
double
cLen(Svec<T> const & v)
{return std::sqrt(cMag(v)); }

template<class T>
Svec<T>
normalize(Svec<T> const & v)
{
    T           len = scast<T>(cLen(v));
    FGASSERT(len > 0.0f);
    return v * (1.0f/len);
}

// Functional version of std::transform. Type converting form requires explicit template arg for 'Out'
template<class Out,class In>
Svec<Out>
mapFuncT(Svec<In> const & in,std::function<Out(In const &)> const & func)
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
mapFunc(Svec<T> const & in,std::function<T(T const &)> const & func)
{
    Svec<T>    ret;
    ret.reserve(in.size());
    for (size_t ii=0; ii<in.size(); ++ii)
        ret.push_back(func(in[ii]));
    return ret;
}

// Map a LHS constant multiplication to new type:  Op * T -> U
// Output type explicit template arg required.
template<class Out,class In,class Op>
Svec<Out>
mapMulT(Op const & op,Svec<In> const & in)
{
    Svec<Out>       ret;
    ret.reserve(in.size());
    for (size_t ii=0; ii<in.size(); ++ii)
        ret.push_back(op * in[ii]);
    return ret;
}

// Map a LHS constant multiplication to same type as RHS:  L * R -> R
// Output type matches input so explicit template args not necessary.
template<class L,class R>
Svec<R>
mapMul(L const & lhs,Svec<R> const & rhs)
{
    Svec<R>     ret;
    ret.reserve(rhs.size());
    for (size_t ii=0; ii<rhs.size(); ++ii)
        ret.push_back(lhs * rhs[ii]);
    return ret;
}

// Non-functional version:
template<class T,class U,class Op>
void
mapMul_(Op const & op,Svec<T> const & in,Svec<U> & out)
{
    out.resize(in.size());
    for (size_t ii=0; ii<in.size(); ++ii)
        out[ii] = op * in[ii];
}

// Non-functional in-place version:
template<class T,class Op>
void
mapMul_(Op const & op,Svec<T> & data)
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

template<class T>
Svec<T>
mapAdd(Svec<T> const & vec,T const & val)
{
    Svec<T>       ret;
    ret.reserve(vec.size());
    for (T const & v : vec)
        ret.push_back(v + val);
    return ret;
}

template<class T>
Svec<T>
mapSub(T const & val,Svec<T> const & vec)
{
    Svec<T>       ret;
    ret.reserve(vec.size());
    for (T const & v : vec)
        ret.push_back(val - v);
    return ret;
}

template<class T>
Svec<T>
mapSub(Svec<T> const & vec,T const & val)
{
    Svec<T>       ret;
    ret.reserve(vec.size());
    for (T const & v : vec)
        ret.push_back(v - val);
    return ret;
}

// Element-wise division:
template<class T>
Svec<T>
mapDiv(Svec<T> const & lhs,Svec<T> const & rhs)
{
    Svec<T>         ret;
    FGASSERT(lhs.size() == rhs.size());
    ret.reserve(lhs.size());
    for (size_t ii=0; ii<lhs.size(); ++ii)
        ret.push_back(lhs[ii]/rhs[ii]);
    return ret;
}

template<class T>
Svec<T>
mapSqr(Svec<T> const & v)
{
    Svec<T>         ret;
    ret.reserve(v.size());
    for (size_t ii=0; ii<v.size(); ++ii)
        ret.push_back(v[ii]*v[ii]);
    return ret;
}

// Multiply-accumulate (MAC) an svec to another:
template<class T,class U,class V>
void
mapMulAcc_(Svec<T> const & vec,U val,Svec<V> & acc)
{
    FGASSERT(vec.size() == acc.size());
    for (size_t ii=0; ii<acc.size(); ++ii)
        acc[ii] += vec[ii] * val;
}

template<class T>
size_t
cMinIdx(Svec<T> const & v)
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
cMaxIdx(Svec<T> const & v)
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
cMaxIdx(Svec<T> const & v,const std::function<bool(T const & lhs,T const & rhs)> & lessThan)
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
cSort_(Svec<T> & v)
{std::sort(v.begin(),v.end()); }

// Functional version of sorting whole vector:
template<class T>
Svec<T>
cSort(Svec<T> const & v)
{
    Svec<T>  ret(v);
    std::sort(ret.begin(),ret.end());
    return ret;
}

// Return array of indices such that the input elements are sorted:
// (ie. A mapping from the NEW order to the ORIGINAL order):
template<class T>
Sizes
sortInds(Svec<T> const & v)
{
    Sizes       inds(v.size());
    for (size_t ii=0; ii<inds.size(); ++ii)
        inds[ii] = ii;
    std::sort(inds.begin(),inds.end(),[&](size_t l,size_t r){return (v[l]<v[r]); });
    return inds;
}

// Make use of a permuted indices array to reorder / re-order a list (or subset thereof)
// (ie. A mapping from the NEW order to the ORIGINAL order):
template<class T,class U,
    // Can be uint or uint64, and on some platforms size_t is different from these:
    FG_ENABLE_IF(U,is_unsigned),
    FG_ENABLE_IF(U,is_integral)
>
Svec<T>
permute(Svec<T> const & v,const Svec<U> & inds)
{
    Svec<T>       ret;
    ret.reserve(inds.size());
    for (size_t ii=0; ii<inds.size(); ++ii)
        ret.push_back(v[inds[ii]]);
    return ret;
}

template<class T>
bool
containsDuplicates(Svec<T> const & mustBeSorted)
{
    for (size_t ii=1; ii<mustBeSorted.size(); ++ii)
        if (mustBeSorted[ii] == mustBeSorted[ii-1])
            return true;
    return false;
}

// Removes duplicates from a sorted vector (I don't get std::unique):
template<class T>
Svec<T>
cUnique(Svec<T> const & sorted)
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
sliceMember(Svec<T> const & ts,U T::*m)
{
    Svec<U>       ret;
    ret.reserve(ts.size());
    for (size_t ii=0; ii<ts.size(); ++ii)
        ret.push_back(ts[ii].*m);
    return ret;
}

// Transpose a vector of vectors just like Python 'zip' on lists.
// All sub-vectors must have the same size():
template<class T>
Svec<Svec<T> >
transpose(Svec<Svec<T> > const & v)
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
cFilter(Svec<T> const & vals,const std::function<bool(T const & val)> & fnSelect)
{
    Svec<T>       ret;
    for (auto it=vals.begin(); it!=vals.end(); ++it)
        if (fnSelect(*it))
            ret.push_back(*it);
    return ret;
}

template<class T>
Svec<T>
cFilter(Svec<T> const & in,Bools const & accept)
{
    Svec<T>       ret;
    FGASSERT(accept.size() == in.size());
    for (size_t ii=0; ii<in.size(); ++ii)
        if (accept[ii])
            ret.push_back(in[ii]);
    return ret;
}

// Inject all elements of domain 'src' into codomain 'dst' in order at true values of 'where'.
// REQUIRED: where.size() == dst.size()
// REQUIRED: number of 'true' values in where == src.size()
template<class T>
Svec<T>
inject(Svec<T> const & src,Svec<T> const & dst,Bools const & where)
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
Sizes
cSizes(Svec<Svec<T> > const & v)
{
    Sizes               ret; ret.reserve(v.size());
    for (Svec<T> const & s : v)
        ret.push_back(s.size());
    return ret;
}

// Set intersection with vector containers; ignores duplicates returns intersection of uniques:
template<class T>
Svec<T>
setwiseIntersect(Svec<T> const & v0,Svec<T> const & v1)
{
    Svec<T>       ret;
    for (T const & v : v0) {
        if (contains(v1,v) && !contains(ret,v))
            ret.push_back(v);
    }
    return ret;
}

template<class T>
Svec<T>
setwiseIntersect(Svec<Svec<T> > const & vs)
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
Svec<T>
multisetIntersect(Svec<T> const & v0,Svec<T> v1)
{
    Svec<T>       ret;
    for (T const & v : v0) {
        auto        it = find(v1.begin(),v1.end(),v);
        if (it != v1.end()) {
            ret.push_back(v);
            v1.erase(it);
        }
    }
    return ret;
}

// Set union on vector containers, retaining lhs order then rhs order:
template<class T>
void
setwiseAdd_(Svec<T> & lhs,Svec<T> const & rhs)
{
    for (T const & r : rhs)
        if (!contains(lhs,r))
            lhs.push_back(r);
}

// Set union on vector containers, retaining lhs order then rhs order:
template<class T>
Svec<T>
setwiseAdd(Svec<T> lhs,Svec<T> const & rhs)
{
    setwiseAdd_(lhs,rhs);
    return lhs;
}

// Set subtraction on vector containers (lhs retains ordering):
template<class T>
Svec<T>
setwiseSubtract(Svec<T> const & lhs,Svec<T> const & rhs)
{
    Svec<T>       ret;
    for (T const & l : lhs) {
        if (!contains(rhs,l))
            ret.push_back(l);
    }
    return ret;
}

template<class T>
Svec<T>
cReverse(Svec<T> v)
{
    std::reverse(v.begin(),v.end());
    return v;
}

}

#endif
