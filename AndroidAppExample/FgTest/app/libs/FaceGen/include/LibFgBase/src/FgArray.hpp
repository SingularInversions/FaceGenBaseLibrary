//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: March 24, 2015
//
// Stack-based variable-size array with fixed max size.
// Less verbose than boost::static_vector.

#ifndef FGARRAY_HPP
#define FGARRAY_HPP

#include "FgStdLibs.hpp"
#include "FgDiagnostics.hpp"

template<class T,uint maxSize>
struct FgArray
{
    T       m[maxSize];
    uint    sz;

    FgArray() : sz(0) {}

    uint
    size() const
    {return sz; }

    const T &
    operator[](uint i) const
    {FGASSERT(i < sz); return m[i]; }

    T &
    operator[](uint i)
    {FGASSERT(i < sz); return m[i]; }

    void
    add(const T & v)    // 'push_back' too verbose
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

#endif
