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

using std::vector;

template<class T>
struct FgTraits<vector<T> >
{
    typedef typename FgTraits<T>::Scalar                Scalar;
    typedef vector<typename FgTraits<T>::Accumulator>   Accumulator;
    typedef vector<typename FgTraits<T>::Floating>      Floating;
};

// Frequently used shorthands:
typedef vector<double>              FgDbls;
typedef vector<float>               FgFlts;
typedef vector<uint>                FgUints;
typedef vector<size_t>              FgSizes;

typedef vector<FgDbls>              FgDblss;
typedef vector<FgFlts>              FgFltss;
typedef vector<FgUints>             FgUintss;

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
    const U &           rhs)    // Different type useful for eg. vector<FgVect> * float
{
    vector<T>   ret;
    ret.reserve(lhs.size());
    for (size_t ii=0; ii<lhs.size(); ++ii)
        ret.push_back(lhs[ii] * rhs);
    return ret;
}

template<class T,class U>
vector<T>
operator/(
    const vector<T> &   lhs,
    const U &           rhs)
{
    vector<T>   ret;
    ret.reserve(lhs.size());
    for (size_t ii=0; ii<lhs.size(); ++ii)
        ret.push_back(lhs[ii] / rhs);
    return ret;
}

template<class T,class U>
void
operator*=(
    vector<T> &     lhs,
    U               rhs)        // Different type useful for eg. vector<FgVect> * float
{
    for (size_t ii=0; ii<lhs.size(); ++ii)
        lhs[ii] *= rhs;
}

template<class T,class U>
void
operator/=(
    vector<T> &     lhs,
    U               rhs)
{
    for (size_t ii=0; ii<lhs.size(); ++ii)
        lhs[ii] /= rhs;
}

template<class T,class U>
vector<U>
fgMapConvert(const vector<T> & in)
{
    vector<U>   ret;
    ret.reserve(in.size());
    for (typename vector<T>::const_iterator it=in.begin(); it != in.end(); ++it)
        ret.push_back(U(*it));
    return ret;
}

inline
FgDbls
fgToDouble(const vector<float> & v)
{
    FgDbls   ret(v.size());
    for (size_t ii=0; ii<ret.size(); ++ii)
        ret[ii] = double(v[ii]);
    return ret;
}

inline
FgFlts
fgToFloat(const vector<double> & v)
{
    FgFlts    ret(v.size());
    for (size_t ii=0; ii<ret.size(); ++ii)
        ret[ii] = float(v[ii]);
    return ret;
}

// Structural:

template<class T>
vector<T>
fgSubvec(const vector<T> & vec,size_t start,size_t size)
{
    FGASSERT(start+size <= vec.size());
    return  vector<T>(vec.begin()+start,vec.begin()+start+size);
}

template<class T>
vector<T>
fgHead(const vector<T> & vec,size_t size)
{
    FGASSERT(size <= vec.size());
    return vector<T>(vec.begin(),vec.begin()+size);
}

template<class T>
vector<T>
fgRest(const vector<T> & vec,size_t start=1)
{
    FGASSERT(start <= vec.size());      // Can be size zero
    return vector<T>(vec.begin()+start,vec.end());
}

template<class T>
vector<T>
fgTail(const vector<T> & vec,size_t size)
{
    FGASSERT(size <= vec.size());
    return vector<T>(vec.end()-size,vec.end());
}

template<class T>
inline void
fgAppend(vector<T> & base,const vector<T> & app)
{base.insert(base.end(),app.begin(),app.end()); }

// Concatenation in several forms:
template<class T>
vector<T>
fgCat(const vector<T> &  v0,const vector<T> &  v1)
{
    vector<T>   ret;
    ret.reserve(v0.size()+v1.size());
    ret.insert(ret.end(),v0.begin(),v0.end());
    ret.insert(ret.end(),v1.begin(),v1.end());
    return ret;
}
template<class T>
vector<T>
fgCat(const vector<T> & v0,const vector<T> & v1,const vector<T> & v2)
{
    vector<T>  ret;
    ret.reserve(v0.size()+v1.size()+v2.size());
    ret.insert(ret.end(),v0.begin(),v0.end());
    ret.insert(ret.end(),v1.begin(),v1.end());
    ret.insert(ret.end(),v2.begin(),v2.end());
    return ret;
}
template<class T>
vector<T>
fgCat(const vector<T> & v0,const vector<T> & v1,const vector<T> & v2,const vector<T> & v3)
{
    vector<T>  ret;
    ret.reserve(v0.size()+v1.size()+v2.size()+v3.size());
    ret.insert(ret.end(),v0.begin(),v0.end());
    ret.insert(ret.end(),v1.begin(),v1.end());
    ret.insert(ret.end(),v2.begin(),v2.end());
    ret.insert(ret.end(),v3.begin(),v3.end());
    return ret;
}
// Flatten vector of vectors into single contiguous vector:
template<class T>
vector<T>
fgCat(const vector<vector<T> > & v)
{
    vector<T>       ret;
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

template<class T,class U>
size_t
fgFindLastIdx(const vector<T> & vec,const U & val)
{
    for (size_t ii=vec.size(); ii!=0; --ii)
        if (vec[ii-1] == val)
            return ii-1;
    return vec.size();
}

// Return first index of 'val' in 'vec' or if not found append and return index:
template<class T>
size_t
fgFindOrAppend(vector<T> & vec,const T & val)
{
    for (size_t ii=0; ii<vec.size(); ++ii)
        if (vec[ii] == val)
            return ii;
    vec.push_back(val);
    return vec.size()-1;
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

template<class T,class U>
bool
fgContainsAny(
    const vector<T> &   ctr,
    const vector<U> &   vals)
{
    for (size_t ii=0; ii<vals.size(); ++ii)
        if (fgContains(ctr,vals[ii]))
            return true;
    return false;
}

template<class T>
void
fgReplace_(vector<T> & v,T a,T b)       // Replace each 'a' with 'b'
{
    for (size_t ii=0; ii<v.size(); ++ii)
        if (v[ii] == a)
            v[ii] = b;
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
fgSetSubVec(vector<T> & mod,size_t pos,const vector<T> & sub)
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
fgCast_(const vector<T> & lhs,vector<U> & rhs)
{
    rhs.resize(lhs.size());
    for (size_t ii=0; ii<lhs.size(); ++ii)
        fgCast_(lhs[ii],rhs[ii]);
}

template<class Out,class In>
vector<Out>
fgMap(const vector<In> & in,std::function<Out(const In &)> func)
{
    vector<Out>    ret;
    ret.reserve(in.size());
    for (size_t ii=0; ii<in.size(); ++ii)
        ret.push_back(func(in[ii]));
    return ret;
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
    Acc         acc(0);
    for (size_t ii=0; ii<v.size(); ++ii)
        acc += Acc(v[ii]);
    return T(acc);
}

// Easier to do this as special case than to have global templated traits for casting
// vector to accumulator type and back. Plus we want things to work a little differently
// in this case:
template<class T>
vector<T>
fgSum(const vector<vector<T> > & v)
{
    vector<T>       ret;
    if (v.empty())
        return ret;
    typedef typename FgTraits<T>::Accumulator   Acc;
    vector<Acc>     acc(v[0].size(),Acc(0));
    for (size_t ii=0; ii<v.size(); ++ii) {
        FGASSERT(v[ii].size() == acc.size());
        for (size_t jj=0; jj<acc.size(); ++jj)
            acc[jj] += Acc(v[ii][jj]);
    }
    ret.reserve(acc.size());
    for (size_t ii=0; ii<acc.size(); ++ii)
        ret.push_back(T(acc[ii]));
    return ret;
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
void
fgMapAddConst_(vector<T> & v,const T & val)
{
    for (size_t ii=0; ii<v.size(); ++ii)
        v[ii] += val;
}

template<class T>
vector<T>
fgMapAddConst(const vector<T> & in,const T & val)
{
    vector<T>       ret;
    ret.reserve(in.size());
    for (typename vector<T>::const_iterator it=in.begin(); it != in.end(); ++it)
        ret.push_back(*it + val);
    return ret;
}

// Element-wise division:
template<class T>
vector<T>
fgMapDiv(
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
double
fgDot(const vector<T> & v0,const vector<T> & v1)
{
    double      acc(0);
    FGASSERT(v0.size() == v1.size());
    for (size_t ii=0; ii<v0.size(); ++ii)
        acc += fgDot(v0[ii],v1[ii]);
    return acc;
}

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

// To get a min version, just use greater than for 'lessThan':
template<class T>
size_t
fgMaxIdx(const vector<T> & v,const std::function<bool(const T & lhs,const T & rhs)> & lessThan)
{
    FGASSERT(!v.empty());
    size_t      ret = 0;
    for (size_t ii=1; ii<v.size(); ++ii)
        if (lessThan(v[ret],v[ii]))
            ret = ii;
    return ret;    
}

// Functional version of std::sort
template<class T>
vector<T>
fgSort(const vector<T> & v)
{
    vector<T>  ret(v);
    std::sort(ret.begin(),ret.end());
    return ret;
}

template<class T>
bool
fgSortIndsLt(const T * v,size_t l,size_t r)
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
        std::sort(inds.begin(),inds.end(),boost::bind(fgSortIndsLt<T>,&v[0],_1,_2));
    return inds;
}

template<class T>
bool
fgSortIndsGt(const T * v,size_t l,size_t r)
{return (v[l] > v[r]); }

// Reverse sort.
// Define as separate function to avoid forcing T to require definitions for both LT and GT:
// Note that !(a < b) is not the same as (a > b) and (a == b) may not be defined either:
template<class T>
vector<size_t>
fgSortIndsRev(const vector<T> & v)
{
    vector<size_t>  inds(v.size());
    for (size_t ii=0; ii<inds.size(); ++ii)
        inds[ii] = ii;
    if (!inds.empty())
        std::sort(inds.begin(),inds.end(),boost::bind(fgSortIndsGt<T>,&v[0],_1,_2));
    return inds;
}

// Make use of a permuted indices list to re-order a list (or subset thereof):
template<class T>
vector<T>
fgReorder(const vector<T> & v,const vector<size_t> & inds)
{
    vector<T>       ret;
    ret.reserve(inds.size());
    for (size_t ii=0; ii<inds.size(); ++ii)
        ret.push_back(v[inds[ii]]);
    return ret;
}

// Sort key / data in separate std::vectors:
template<class Key,class Data>
void
fgSortKey(vector<Key> & k,vector<Data> & d)
{
    vector<size_t>  order = fgSortInds(k);
    k = fgReorder(k,order);
    d = fgReorder(d,order);
}
template<class Key,class Data>
void
fgSortKeyRev(vector<Key> & k,vector<Data> & d)
{
    vector<size_t>  order = fgSortIndsRev(k);
    k = fgReorder(k,order);
    d = fgReorder(d,order);
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

// Take a slice of a multi-dimensional array in a vector:
template<class T>
vector<T>
fgSlice(const vector<T> & v,size_t initial,size_t stride)
{
    vector<T>       ret;
    FGASSERT(initial < v.size());
    ret.reserve((v.size()-initial+stride-1)/stride);
    for (size_t ii=initial; ii<ret.size(); ii+=stride)
        ret.push_back(v[ii]);
    return ret;
}

template<class T,class U>
vector<U>
fgSliceMember(const vector<T> & ts,U T::*m)
{
    vector<U>       ret;
    ret.reserve(ts.size());
    for (size_t ii=0; ii<ts.size(); ++ii)
        ret.push_back(ts[ii].*m);
    return ret;
}

// Transpose a row-major contiguous array:
template<class T>
vector<T>
fgTranspose(const vector<T> & v,size_t wid,size_t hgt)
{
    vector<T>       ret;
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
fgFill(vector<T> & vec,T val)
{
    for (size_t ii=0; ii<vec.size(); ++ii)
        vec[ii] = val;
}

// Transpose a vector of vectors just like Python 'zip' on lists.
// All sub-vectors must have the same size():
template<class T>
vector<vector<T> >
fgZip(const vector<vector<T> > & v)
{
    vector<vector<T> >  ret;
    if (v.empty())
        return ret;
    for (size_t jj=1; jj<v.size(); ++jj)
        FGASSERT(v[jj].size() == v[0].size());
    ret.resize(v[0].size());
    for (size_t ii=0; ii<ret.size(); ++ii) {
        ret[ii].reserve(v.size());
        for (size_t jj=0; jj<v.size(); ++jj)
            ret[ii].push_back(v[jj][ii]);
    }
    return ret;
}

template<class T>
std::ostream &
operator<<(std::ostream & ss,const vector<T> & vv)
{
    std::ios::fmtflags
        oldFlag = ss.setf(
            std::ios::fixed |
            std::ios::showpos |
            std::ios::right);
    std::streamsize oldPrec = ss.precision(6);
    ss << "[" << fgpush;
    if (vv.size() > 0)
        ss << vv[0];
	for (uint ii=1; ii<vv.size(); ii++)
		ss << "," << vv[ii];
    ss << fgpop << "]";
    ss.flags(oldFlag);
    ss.precision(oldPrec);
	return ss;
}

template<class T>
double
fgMag(const vector<T> & v)
{
    double      ret(0);
    for (size_t ii=0; ii<v.size(); ++ii)
        ret += fgMag(v[ii]);
    return ret;
}

template<class T>
vector<T>
fgFilter(const vector<T> & vals,const std::function<bool(const T & val)> & fnSelect)
{
    vector<T>       ret;
    for (auto it=vals.begin(); it!=vals.end(); ++it)
        if (fnSelect(*it))
            ret.push_back(*it);
    return ret;
}

template<class T>
vector<T>
fgFilter(const vector<T> & in,const vector<bool> & accept)
{
    vector<T>       ret;
    FGASSERT(accept.size() == in.size());
    for (size_t ii=0; ii<in.size(); ++ii)
        if (accept[ii])
            ret.push_back(in[ii]);
    return ret;
}

template<class T>
vector<size_t>
fgJaggedDims(const vector<vector<T> > & v)
{
    vector<size_t>      ret(v.size());
    for (size_t ii=0; ii<ret.size(); ++ii)
        ret[ii] = v[ii].size();
    return ret;
}

template<class T>
vector<vector<T> >
fgJaggedVec(const vector<size_t> & dims,const T & initVal)
{
    vector<vector<T> >      ret(dims.size());
    for (size_t ii=0; ii<ret.size(); ++ii)
        ret[ii].resize(dims[ii],initVal);
    return ret;
}

template<class T>
vector<vector<T> >
fgVecOfVecs(size_t dim0,size_t dim1,const T & initVal)
{
    vector<vector<T> >      ret(dim0);
    for (size_t ii=0; ii<ret.size(); ++ii)
        ret[ii].resize(dim1,initVal);
    return ret;
}

#endif
