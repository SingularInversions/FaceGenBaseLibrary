//
// Copyright (c) 2018 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     18.04.14
//
// An order-preserving set for small collections based on std::vector. For:
// 1. Small sets where we don't want the overhead of std::set
// 2. Sets which need to preserve ordering
//
// The element type must have operator==() defined

#ifndef FGSET_HPP
#define FGSET_HPP

#include "FgStdLibs.hpp"
#include "FgDiagnostics.hpp"

template<typename T>
struct  FgSet
{
    std::vector<T>      set;

    size_t
    size() const
    {return set.size(); }

    bool
    contains(const T & val) const
    {
        for (const T & s : set)
            if (s == val)
                return true;
        return false;
    }

    void
    insert(const T & val)
    {
        for (const T & s : set)
            if (s == val)
                return;
        set.push_back(val);
    }
};

#endif
