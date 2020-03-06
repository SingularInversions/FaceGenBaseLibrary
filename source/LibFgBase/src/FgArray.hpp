//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
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

template<class T,uint maxSize>
struct VArray
{
    T       m[maxSize];
    uint    sz = 0;

    uint
    size() const
    {return sz; }

    T const &
    operator[](uint i) const
    {FGASSERT(i < sz); return m[i]; }

    T &
    operator[](uint i)
    {FGASSERT(i < sz); return m[i]; }

    void
    add(T const & v)    // 'push_back' too verbose
    {
        FGASSERT(sz < maxSize);
        m[sz] = v;
        ++sz;
    }

    void
    erase(uint idx)
    {
        FGASSERT(idx < sz);
        for (uint ii=idx; ii<sz-1; ++ii)
            m[ii] = m[ii+1];
        --sz;
    }

    bool
    empty() const
    {return (sz == 0); }
};

}

#endif
