//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Stack-based variable-size array with fixed max size similar to boost::static_vector. 

#ifndef FGARRAY_HPP
#define FGARRAY_HPP

#include "FgSerial.hpp"

namespace Fg {

template<class T,size_t M>
class       VArray
{
    Arr<T,M>            m;
    size_t              sz {0};     // can be at most M

public:
    VArray() {}
    // more useful than intializer lists; allow type conversion, emplace_back and static size check:
    VArray(T a,T b) : sz{2} {m[0]=a; m[1]=b; static_assert(2<=M); }
    VArray(T a,T b,T c) : sz{3} {m[0]=a; m[1]=b; m[2]=c; static_assert(3<=M); }
    VArray(T a,T b,T c,T d) : sz{4} {m[0]=a; m[1]=b; m[2]=c; m[3]=d; static_assert(4<=M); }
    VArray(T a,T b,T c,T d,T e) : sz{5} {m[0]=a; m[1]=b; m[2]=c; m[3]=d; m[4]=e; static_assert(5<=M); }
    VArray(T a,T b,T c,T d,T e,T f) : sz{6} {m[0]=a; m[1]=b; m[2]=c; m[3]=d; m[4]=e; m[5]=f; static_assert(6<=M); }
    template<size_t S>
    explicit VArray(Arr<T,S> const & a) : sz{S}
    {
        static_assert(S<=M);
        std::copy(a.begin(),a.end(),m.begin());
    }

    bool                empty() const {return (sz == 0); }
    size_t              size() const {return sz; }
    T const &           operator[](size_t i) const {FGASSERT(i < sz); return m[i]; }
    T &                 operator[](size_t i) {FGASSERT(i < sz); return m[i]; }
    auto                begin() const {return m.begin(); }
    auto                end() const {return m.begin() + sz; }
    void                clear() {sz = 0; }
    T const &           back() const {FGASSERT(sz>0); return m[sz-1]; }
    T &                 back() {FGASSERT(sz>0); return m[sz-1]; }
    bool                operator==(VArray const & r) const
    {
        if (r.sz != sz)
            return false;
        for (size_t ii=0; ii<sz; ++ii)
            if (!(r[ii] == m[ii]))
                return false;
        return true;
    }
    void                append(T const & v)     // 'push_back' is inferior terminology
    {
        FGASSERT(sz < M);
        m[sz++] = v;
    }
    void                insertOverflow( // if full, last value is discarded
        size_t              ii,         // index of an existing element to insert before
        T const &           v)          // value to insert
    {
        FGASSERT(ii<sz);
        if (sz < M) {                   // no overflow
            for (size_t jj=sz; jj>ii; --jj)
                m[jj] = m[jj-1];
            ++sz;
        }
        else                            // last element is lost
            for (size_t jj=M-1; jj>ii; --jj)
                m[jj] = m[jj-1];
        m[ii] = v;
    }
    void                erase(size_t idx)
    {
        // can't use #pragma GCC diagnostic ignored "-Wmaybe-uninitialized" since clang will give warnings
        FGASSERT((idx < sz) && (idx < M));    // must check against M to avoid warnings
        for (size_t ii=idx; ii+1<sz; ++ii)
            m[ii] = m[ii+1];
        --sz;
    }
};

typedef     VArray<uint,3>      VArrayUI3;
typedef     VArray<uint,4>      VArrayUI4;
typedef     Svec<VArrayUI3>     VArrayUI3s;
typedef     Svec<VArrayUI4>     VArrayUI4s;

template<class T,size_t M,class F>
auto            mapCall(VArray<T,M> const & arr,F f)
{
    typedef decltype(f(arr[0])) R;
    VArray<R,M>                 ret;
    for (T const & a : arr)
        ret.append(f(a));
    return ret;
}

template<typename T,typename U,size_t M>
VArray<U,M>         mapMember(VArray<T,M> const & arr,U T::*m)
{
    return mapCall(arr,[m](T const & e){return e.*m; });
}

template<typename T,size_t M>
T                   cSum(VArray<T,M> const & vals)
{
    FGASSERT(vals.size() > 0);
    auto                it = vals.begin();
    T                   ret = *it;
    for (++it; it != vals.end(); ++it)
        ret += *it;
    return ret;
}

template<typename T,size_t M>
T                   cMean(VArray<T,M> const & vals)
{
    typedef typename Traits<T>::Scalar   Scalar;
    return cSum(vals) / scast<Scalar>(vals.size());
}

// insert new element into sort location in a sorted array, dropping the last element in the case of overflow:
template<class T,size_t M,class C=std::less<T>>
void                insertSorted_(VArray<T,M> & arr,T const & v)
{
    C                   cmp{};
    for (size_t ii=0; ii<arr.size(); ++ii) {
        if (cmp(v,arr[ii])) {
            arr.insertOverflow(ii,v);
            return;
        }
    }
    if (arr.size() < M)
        arr.append(v);
}

}

#endif
