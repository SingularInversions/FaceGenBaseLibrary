//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: March 24, 2015
//
// Stack-based variable-size array with fixed max size.

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

    // Unchecked !
    const T &
    operator[](uint i) const
    {FGASSERT_FAST(i < sz); return m[i]; }

    T &
    operator[](uint i)
    {FGASSERT_FAST(i < sz); return m[i]; }

    void
    add(const T & v)
    {
        FGASSERT_FAST(sz < maxSize);
        m[sz] = v;
        ++sz;
    }
};

#endif
