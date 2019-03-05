//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Sohail Somani
//
// Note that stack objects are always destructed in reverse order of their definitions.

#ifndef INCLUDED_FGSCOPEGUARD_HPP
#define INCLUDED_FGSCOPEGUARD_HPP

#include "FgBoostLibs.hpp"
#include "FgNonCopyable.hpp"

struct FgScopeGuard : FgNonCopyable
{
    FgScopeGuard(std::function<void()> const & fn) : m_fn(fn) {}

    ~FgScopeGuard() {m_fn(); }

private:
    std::function<void()> m_fn;
};

// A version that allows you to ignore the return type of the end-of-scope function.
// This is required for VS2013 and for strict C++11 compliance:
template<typename T>
struct FgScopeGuardT : FgNonCopyable
{
    FgScopeGuardT(std::function<T()> const & fn) : m_fn(fn) {}

    ~FgScopeGuardT() {m_fn(); }

private:
    std::function<T()> m_fn;
};

#endif // FGSCOPEGUARD_HPP
