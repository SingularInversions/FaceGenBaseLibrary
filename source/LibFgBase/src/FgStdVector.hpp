//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Dec 29, 2004
//
// Global functions providing additional operations related to vector.
//
// Good article on possible optimizations:
//
// https://github.com/facebook/folly/blob/master/folly/docs/FBVector.md
//

#ifndef FGSTDVECTOR_HPP
#define FGSTDVECTOR_HPP

#include "FgStdLibs.hpp"

#include "FgTypes.hpp"
#include "FgDiagnostics.hpp"
#include "FgOut.hpp"

using std::vector;      // please god spare me from ever typing 'std::' again.

// Shorthands:

typedef vector<double>      FgDoubles;
typedef vector<float>       FgFloats;
typedef vector<uint>        FgUints;
typedef vector<size_t>      FgSizes;

// Acts just like bool for use with vector but avoids use of broken
// vector<bool> specialization:
struct FgBool
{
    uchar   m;
    FgBool() : m(0) {}
    FgBool(bool v) : m(v ? 1 : 0) {}
    operator bool () const {return (m > 0); }
};

// Construction:

template<class T>
vector<T>
fgSvec(const T & v1)
{
    return vector<T>(1,v1);
}
template<class T>
vector<T>
fgSvec(const T & v1,const T & v2)
{
    vector<T> vec(2);
    vec[0] = v1; vec[1] = v2;
    return vec;
}
template<class T>
vector<T>
fgSvec(const T & v1,const T & v2,const T & v3)
{
    vector<T> vec(3);
    vec[0] = v1; vec[1] = v2; vec[2] = v3;
    return vec;
}
template<class T>
vector<T>
fgSvec(const T & v0,const T & v1,const T & v2,const T & v3)
{
    vector<T> vec(4);
    vec[0] = v0; vec[1] = v1; vec[2] = v2; vec[3] = v3;
    return vec;
}

template<class T>
vector<T>
fgSvec(const T & v0,const T & v1,const T & v2,const T & v3,const T & v4)
{
    vector<T> vec(5);
    vec[0] = v0; vec[1] = v1; vec[2] = v2; vec[3] = v3; vec[4] = v4;
    return vec;
}

template<class T>
vector<T>
fgSvec(const T & v0,const T & v1,const T & v2,const T & v3,const T & v4,const T & v5)
{
    vector<T> vec(6);
    vec[0]=v0; vec[1]=v1; vec[2]=v2; vec[3]=v3; vec[4]=v4; vec[5]=v5;
    return vec;
}

template<class T>
vector<T>
fgSvec(const T & v0,const T & v1,const T & v2,const T & v3,const T & v4,const T & v5,const T & v6)
{
    vector<T> vec(7);
    vec[0]=v0; vec[1]=v1; vec[2]=v2; vec[3]=v3; vec[4]=v4; vec[5]=v5; vec[6]=v6;
    return vec;
}

template<class T>
vector<T>
fgSvec(const T & v0,const T & v1,const T & v2,const T & v3,const T & v4,const T & v5,
         const T & v6,const T & v7)
{
    vector<T> vec(8);
    vec[0]=v0; vec[1]=v1; vec[2]=v2; vec[3]=v3; vec[4]=v4; vec[5]=v5; vec[6]=v6;
    vec[7]=v7;
    return vec;
}

template<class T>
vector<T>
fgSvec(const T & v0,const T & v1,const T & v2,const T & v3,const T & v4,const T & v5,
         const T & v6,const T & v7,const T & v8)
{
    vector<T> vec(9);
    vec[0]=v0; vec[1]=v1; vec[2]=v2; vec[3]=v3; vec[4]=v4; vec[5]=v5; vec[6]=v6;
    vec[7]=v7; vec[8]=v8;
    return vec;
}

template<class T>
vector<T>
fgSvec(const T & v0,const T & v1,const T & v2,const T & v3,const T & v4,const T & v5,
       const T & v6,const T & v7,const T & v8,const T & v9)
{
    vector<T> vec(10);
    vec[0]=v0; vec[1]=v1; vec[2]=v2; vec[3]=v3; vec[4]=v4; vec[5]=v5; vec[6]=v6;
    vec[7]=v7; vec[8]=v8; vec[9]=v9;
    return vec;
}

template<class T>
vector<T>
fgSvec(const T & v0,const T & v1,const T & v2,const T & v3,const T & v4,const T & v5,
       const T & v6,const T & v7,const T & v8,const T & v9,const T & vA)
{
    vector<T> vec(11);
    vec[0]=v0; vec[1]=v1; vec[2]=v2; vec[3]=v3; vec[4]=v4; vec[5]=v5; vec[6]=v6;
    vec[7]=v7; vec[8]=v8; vec[9]=v9; vec[10] = vA;
    return vec;
}

template<class T,class U>
vector<T>
fgConvertElems(const vector<U> & rhs)
{
    vector<T>   ret;
    ret.reserve(rhs.size());
    for (size_t ii=0; ii<rhs.size(); ++ii)
        ret.push_back(T(rhs[ii]));
    return ret;
}

inline
FgDoubles
fgToDouble(const vector<float> & v)
{
    FgDoubles   ret(v.size());
    for (size_t ii=0; ii<ret.size(); ++ii)
        ret[ii] = double(v[ii]);
    return ret;
}

inline
FgFloats
fgToFloat(const vector<double> & v)
{
    FgFloats    ret(v.size());
    for (size_t ii=0; ii<ret.size(); ++ii)
        ret[ii] = float(v[ii]);
    return ret;
}

// Structural:

template<class T>
vector<T>
fgSubvec(
    const vector<T> &  vec,
    size_t                  start,
    size_t                  size)
{
    FGASSERT(start+size <= vec.size());
    return  vector<T>(vec.begin()+start,vec.begin()+start+size);
}

template<class T>
vector<T>
fgHead(
    const vector<T> &   vec,
    size_t              size)
{
    FGASSERT(size <= vec.size());
    return vector<T>(vec.begin(),vec.begin()+size);
}

template<class T>
vector<T>
fgRest(
    const vector<T> &   vec,
    size_t              start=1)
{
    FGASSERT(start <= vec.size());      // Can be size zero
    return vector<T>(vec.begin()+start,vec.end());
}

template<class T>
vector<T>
fgTail(
    const vector<T> &   vec,
    size_t              size)
{
    FGASSERT(size <= vec.size());
    return vector<T>(vec.end()-size,vec.end());
}

template<class T>
vector<T> &
fgAppend(
    vector<T> &        base,
    const vector<T> &  app)
{
    base.insert(base.end(),app.begin(),app.end());
    return base;
}

template<class T>
vector<T>
fgConcat(
    const vector<T> &  v0,
    const vector<T> &  v1)
{
    vector<T>  ret;
    ret.reserve(v0.size()+v1.size());
    for (size_t ii=0; ii<v0.size(); ++ii)
        ret.push_back(v0[ii]);
    for (size_t jj=0; jj<v1.size(); ++jj)
        ret.push_back(v1[jj]);
    return ret;
}

// Like std::find except it returns index rather than iterator of first occurance.
// If not found, returns v.size() or throws:
template<class T,class U>
size_t
fgFindFirstIdx(
    const vector<T> &  vec,
    const U &          val,     // Allow for T::operator==(U)
    bool               throwOnFail=false)
{
    for (size_t ii=0; ii<vec.size(); ++ii)
        if (vec[ii] == val)
            return ii;
    if (throwOnFail)
        FGASSERT_FALSE;
    return vec.size();
}

template<class T,class U>
const T &
fgFindFirst(
    const vector<T> &  vec,
    const U &          val)     // Allow for T::operator==(U)
{
    for (size_t ii=0; ii<vec.size(); ++ii)
        if (vec[ii] == val)
            return vec[ii];
    FGASSERT_FALSE;
    return vec[0];
}

template<class T>
size_t
fgFindLastIdx(const vector<T> & vec,const T & val)
{
    for (size_t ii=vec.size(); ii!=0; --ii)
        if (vec[ii-1] == val)
            return ii-1;
    return vec.size();
}

template<class T,class U>
bool
fgContains(
    const vector<T> &  vec,
    const U &          val)     // Allows for T::operator==(U)
{
    for (size_t ii=0; ii<vec.size(); ++ii)
        if (vec[ii] == val)
            return true;
    return false;
}

template<class T>
vector<T>
fgReplace(const vector<T> & v,T a,T b) // Replace each 'a' with 'b'
{
    vector<T>  ret(v);
    for (size_t ii=0; ii<ret.size(); ++ii)
        if (ret[ii] == a)
            ret[ii] = b;
    return ret;
}

// Returns at least size 1, with 1 additional for each split element:
template<class T>
vector<vector<T> >
fgSplit(const vector<T> & str,T ch)
{
    vector<vector<T> >    ret;
    vector<T>                  ss;
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
fgSetSubvec(vector<T> & mod,const vector<T> & sub,size_t pos=0)
{
    FGASSERT(sub.size() + pos <= mod.size());
    for (size_t ii=0; ii<sub.size(); ++ii)
        mod[pos+ii] = sub[ii];
}

template<class T>
bool
fgBeginsWith(const vector<T> & base,const vector<T> & pattern)
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
fgEndsWith(const vector<T> & base,const vector<T> & pattern)
{
    if (pattern.size() > base.size())
        return false;
    size_t      offset = base.size() - pattern.size();
    for (size_t ii=0; ii<pattern.size(); ++ii)
        if (pattern[ii] != base[ii+offset])
            return false;
    return true;
}

// Logical:

template<class T>
void
fgAnd(
    const vector<T> &  in0,
    const vector<T> &  in1,
    vector<T> &        out)    // Referencing one of the inputs is allowed.
{
    size_t      size = in0.size();
    FGASSERT(size == in1.size());
    out.resize(size);
    for (size_t ii=0; ii<size; ++ii)
        out[ii] = in0[ii] && in1[ii];
}

template<class T>
void
fgOr(
    const vector<T> &  in0,
    const vector<T> &  in1,
    vector<T> &        out)    // Referencing one of the inputs is allowed.
{
    size_t      size = in0.size();
    FGASSERT(size == in1.size());
    out.resize(size);
    for (size_t ii=0; ii<size; ++ii)
        out[ii] = in0[ii] || in1[ii];
}

template<class T>
void
fgAndNot(
    const vector<T> &  in0,
    const vector<T> &  in1,
    vector<T> &        out)    // Referencing one of the inputs is allowed.
{
    size_t      size = in0.size();
    FGASSERT(size == in1.size());
    out.resize(size);
    for (size_t ii=0; ii<size; ++ii)
        out[ii] = in0[ii] && !in1[ii];
}

template<class T>
bool
fgOrOfAnd(                          // Equivalent to but more efficient than fgOr(fgAnd(vec0,vec1))
    const vector<T> &  in0,
    const vector<T> &  in1)
{
    size_t      size = in0.size();
    FGASSERT(size == in1.size());
    for (size_t ii=0; ii<size; ++ii)
        if (in0[ii] && in1[ii])
            return true;
    return false;
}

// Numerical:

template<class T,class U>
void
fgConvert_(
    const vector<T> &  lhs,
    vector<U> &        rhs)
{
    rhs.resize(lhs.size());
    for (size_t ii=0; ii<lhs.size(); ++ii)
        rhs[ii] = static_cast<U>(lhs[ii]);
}

template<class In,class Op,class Out>
void
fgMap_(const vector<In> & in,Op op,vector<Out> & out)
{
    out.resize(in.size());
    for (size_t ii=0; ii<out.size(); ++ii)
        out[ii] = op(in[ii]);
}

template<class In,class Op,class Out>
vector<Out>
fgMap(const vector<In> & in,Op op)
{
    vector<Out>    ret(in.size());
    for (size_t ii=0; ii<ret.size(); ++ii)
        ret[ii] = op(in[ii]);
    return ret;
}

template<class In,class Op,class Out>
void
fgMap2_(const vector<In> & in0,const vector<In> & in1,Op op,vector<Out> & out)
{
    FGASSERT(in0.size() == in1.size());
    out.resize(in0.size());
    for (size_t ii=0; ii<out.size(); ++ii)
        out[ii] = op(in0[ii],in1[ii]);
}

// Transform using an operator*() :
template<class T,class Op>
vector<T>
fgTransform(
    const vector<T> &  in,
    // Getting rid of 'const' here would be handly for template matching and for 'op' to
    // compute things but then you get warning when op is an r-value...
    const Op &         op)
{
    vector<T>  ret;
    ret.reserve(in.size());
    for (size_t ii=0; ii<in.size(); ++ii)
        ret.push_back(op * in[ii]);
    return ret;
}

// Output-by-reference version:
template<class T,class Op>
void
fgTransform_(
    const vector<T> &  in,
    vector<T> &        out,    // Can be same object as in
    const Op &         op)
{
    out.resize(in.size());
    for (size_t ii=0; ii<in.size(); ++ii)
        out[ii] = op * in[ii];
}

// in-place version:
template<class T,class Op>
void
fgTransform_(
    vector<T> &         data,
    const Op &          op)
{
    for (size_t ii=0; ii<data.size(); ++ii)
        data[ii] = op * data[ii];
}

template<class T>
T
fgSum(const vector<T> & v)
{
    typedef typename FgTraits<T>::Accumulator Acc;
    FGASSERT(v.size() > 0);
    Acc     acc = Acc(v[0]);
    for (size_t ii=1; ii<v.size(); ++ii)
        acc += Acc(v[ii]);
    return T(acc);
}

template<class T>
T
fgMean(const vector<T> & v)
{
    typedef typename FgTraits<T>::Scalar      Scal;
    return (fgSum(v) / Scal(v.size()));
}

template<class T>
void
fgSubtract(
    const vector<T> &  lhs,
    const vector<T> &  rhs,
    vector<T> &        res)
{
    FGASSERT(lhs.size() == rhs.size());
    res.resize(lhs.size());
    for (size_t ii=0; ii<lhs.size(); ++ii)
        res[ii] = lhs[ii] - rhs[ii];
}

template<class T>
vector<T>
fgDivide(
    const vector<T> &   lhs,
    const vector<T> &   rhs)
{
    vector<T>       ret;
    FGASSERT(lhs.size() == rhs.size());
    ret.reserve(lhs.size());
    for (size_t ii=0; ii<lhs.size(); ++ii)
        ret.push_back(lhs[ii]/rhs[ii]);
    return ret;
}

template<class T>
vector<T>
operator-(
    const vector<T> &  lhs,
    const vector<T> &  rhs)
{
    vector<T>   ret;
    ret.reserve(lhs.size());
    FGASSERT(lhs.size() == rhs.size());
    for (size_t ii=0; ii<lhs.size(); ++ii)
        ret.push_back(lhs[ii] - rhs[ii]);
    return ret;
}

template<class T>
vector<T>
operator+(
    const vector<T> &  lhs,
    const vector<T> &  rhs)
{
    vector<T>       ret;
    FGASSERT(lhs.size() == rhs.size());
    ret.reserve(lhs.size());
    for (size_t ii=0; ii<lhs.size(); ++ii)
        ret.push_back(lhs[ii] + rhs[ii]);
    return ret;
}

template<class T>
void
operator-=(
    vector<T> &         lhs,
    const vector<T> &   rhs)
{
    FGASSERT(lhs.size() == rhs.size());
    for (size_t ii=0; ii<lhs.size(); ++ii)
        lhs[ii] -= rhs[ii];
}

template<class T>
void
operator+=(
    vector<T> &         lhs,
    const vector<T> &   rhs)
{
    FGASSERT(lhs.size() == rhs.size());
    for (size_t ii=0; ii<lhs.size(); ++ii)
        lhs[ii] += rhs[ii];
}

template<class T,class U>
vector<T>
operator*(
    const vector<T> &   lhs,
    const U &           rhs)
{
    vector<T>   ret;
    ret.reserve(lhs.size());
    for (size_t ii=0; ii<lhs.size(); ++ii)
        ret.push_back(lhs[ii] * rhs);
    return ret;
}

template<class T,class U>
void
operator*=(
    vector<T> &     lhs,
    U               rhs)    // Different type useful for eg. vector<FgVect> * float
{
    for (size_t ii=0; ii<lhs.size(); ++ii)
        lhs[ii] *= rhs;
}

template<class T>
struct FgTraits<vector<T> >
{
    typedef vector<typename FgTraits<T>::Accumulator>    Accumulator;
    typedef vector<typename FgTraits<T>::Floating>       Floating;
};

// Sum of squares:
template<class T>
T
fgLengthSqr(const vector<T> & v)
{
    typename FgTraits<T>::Accumulator  ret(0);
    for (size_t ii=0; ii<v.size(); ++ii) {
        T       tmp = v[ii];
        ret += tmp * tmp;   // fgSqr not yet defined.
    }
    return T(ret);
}

template<class T>
T
fgLength(const vector<T> & v)
{return std::sqrt(fgLengthSqr(v)); }

template<class T>
T
fgSsd(const vector<T> & v0,const vector<T> & v1)
{
    FGASSERT(v0.size() == v1.size());
    T   acc(0);
    for (size_t ii=0; ii<v0.size(); ++ii) {
        T   tmp = v1[ii]-v0[ii];
        acc += tmp*tmp;     // fgSqr not yet defined.
    }
    return acc;
}

template<class T>
T
fgRms(const vector<T> & v)
{
    FGASSERT(v.size() > 0);
    return std::sqrt(fgLengthSqr(v) / v.size());
}

template<class T>
vector<T>
fgMapAbs(const vector<T> & v)
{
    vector<T>  ret;
    ret.reserve(v.size());
    for (size_t ii=0; ii<v.size(); ++ii)
        ret.push_back(std::abs(v[ii]));
    return ret;
}

template<class T>
vector<T>
fgMapSqr(const vector<T> & v)
{
    vector<T>  ret;
    ret.reserve(v.size());
    for (size_t ii=0; ii<v.size(); ++ii)
        ret.push_back(v[ii]*v[ii]);
    return ret;
}

template<typename T>
size_t
fgMinIdx(const vector<T> & v)
{
    size_t  ret = 0;
    FGASSERT(!v.empty());
    T       min = v[0];
    for (size_t ii=1; ii<v.size(); ++ii)
        if (v[ii] < min) {
            min = v[ii];
            ret = ii;
        }
    return ret;
}

template<typename T>
T
fgMin(const vector<T> & v)
{
    FGASSERT(!v.empty());
    T       ret = v[0];
    for (size_t ii=1; ii<v.size(); ++ii)
        if (v[ii] < ret)
            ret = v[ii];
    return ret;
}

template<class T>
T
fgMax(const vector<T> & v)
{
    FGASSERT(!v.empty());
    T       ret = v[0];
    for (size_t ii=1; ii<v.size(); ++ii)
        if (v[ii] > ret)
            ret = v[ii];
    return ret;
}

template<class T>
vector<T>
fgSort(const vector<T> & v)
{
    if (v.size() < 2)
        return v;
    vector<T>  ret(v);
    std::sort(ret.begin(),ret.end());
    return ret;
}

template<class T>
bool
fgSortIndsComparator(const T * v,size_t l,size_t r)
{return (v[l] < v[r]); }

// Return a list of the permuted indices instead of the sorted list itself:
template<class T>
vector<size_t>
fgSortInds(const vector<T> & v)
{
    vector<size_t>  inds(v.size());
    for (size_t ii=0; ii<inds.size(); ++ii)
        inds[ii] = ii;
    if (!inds.empty())
        std::sort(inds.begin(),inds.end(),boost::bind(&fgSortIndsComparator<T>,&v[0],_1,_2));
    return inds;
}

// Make use of a permuted indices list to re-order a list:
template<class T>
vector<T>
fgReorder(const vector<T> & v,const vector<size_t> & inds)
{
    vector<T>       ret(v.size());
    for (size_t ii=0; ii<ret.size(); ++ii)
        ret[ii] = v[inds[ii]];
    return ret;
}

template<class T>
vector<T>
fgUnique(const vector<T> & v)
{
    if (v.size() < 2)
        return v;
    // std::unique is retarded:
    vector<T>  ret(1,v[0]);
    for (size_t ii=1; ii<v.size(); ++ii)
        if (v[ii] != ret.back())
            ret.push_back(v[ii]);
    return ret;
}

template<class T>
std::ostream &
operator<<(std::ostream & ss,const vector<T> & vv)
{
    ss << "[" << fgpush;
    if (vv.size() > 0)
        ss << vv[0];
	for (uint ii=1; ii<vv.size(); ii++)
		ss << "," << vv[ii];
    ss << fgpop << "]";
	return ss;
}

#endif
