//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// An order-preserving set for small collections based on std::vector. For:
// 1. Small sets where we don't want the overhead of std::set
// 2. Sets which need to preserve ordering
//
// The element type must have operator==() defined

#ifndef FGSET_HPP
#define FGSET_HPP

#include "FgSerial.hpp"

namespace Fg {

template<typename T>
struct  FgSet
{
    Svec<T>      set;

    size_t
    size() const
    {return set.size(); }

    bool
    contains(T const & val) const
    {
        for (T const & s : set)
            if (s == val)
                return true;
        return false;
    }

    void
    insert(T const & val)
    {
        for (T const & s : set)
            if (s == val)
                return;
        set.push_back(val);
    }
};

}

#endif
