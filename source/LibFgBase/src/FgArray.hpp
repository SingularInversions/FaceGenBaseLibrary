//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Stack-based variable-size array with fixed max size.
// Less verbose than boost::static_vector.

#ifndef FGARRAY_HPP
#define FGARRAY_HPP

#include "FgStdLibs.hpp"
#include "FgDiagnostics.hpp"

namespace Fg {

template<class T,size_t maxSize>
struct VArray
{
    T                   m[maxSize];
    size_t              sz = 0;

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
        FGASSERT((idx < sz) && (idx < maxSize));
        for (size_t ii=idx; ii+1<sz; ++ii)
            m[ii] = m[ii+1];
        --sz;
    }
};

}

#endif
