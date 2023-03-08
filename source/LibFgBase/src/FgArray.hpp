//
// Copyright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Stack-based variable-size array with fixed max size similar to boost::static_vector.

#ifndef FGARRAY_HPP
#define FGARRAY_HPP

#include "FgSerial.hpp"

namespace Fg {

template<class T,size_t maxSize>
struct VArray
{
    T                   m[maxSize];
    size_t              sz = 0;

    VArray() {}
    VArray(T a,T b,T c) : m{a,b,c}, sz{3} {static_assert(3<=maxSize,"VArray arguments > 3"); }
    VArray(T a,T b,T c,T d) : m{a,b,c,d}, sz{4} {static_assert(4<=maxSize,"VArray arguments > 4"); }

    bool                empty() const {return (sz == 0); }
    size_t              size() const {return sz; }
    T const &           operator[](size_t i) const {FGASSERT(i < sz); return m[i]; }
    T &                 operator[](size_t i) {FGASSERT(i < sz); return m[i]; }
    void                clear() {sz = 0; }

    void                add(T const & v)    // 'push_back' too verbose
    {
        FGASSERT(sz < maxSize);
        m[sz] = v;
        ++sz;
    }

    void                erase(size_t idx)
    {
        // can't use #pragma GCC diagnostic ignored "-Wmaybe-uninitialized" since clang will give warnings
        FGASSERT((idx < sz) && (idx < maxSize));    // must check against maxSize to avoid warnings
        for (size_t ii=idx; ii+1<sz; ++ii)
            m[ii] = m[ii+1];
        --sz;
    }
};

typedef     VArray<uint,3>      VArrayUI3;
typedef     VArray<uint,4>      VArrayUI4;
typedef     Svec<VArrayUI3>     VArrayUI3s;
typedef     Svec<VArrayUI4>     VArrayUI4s;

}

#endif
