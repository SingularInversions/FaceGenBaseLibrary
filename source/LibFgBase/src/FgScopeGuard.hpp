//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// * For scope guarding with pointers, use std::unique_ptr with custom destructor type or std::function
// * Note that stack objects are always destructed in reverse order of their definitions

#ifndef FGSCOPEGUARD_HPP
#define FGSCOPEGUARD_HPP

#include "FgStdLibs.hpp"

namespace Fg {

struct ScopeGuard
{
    // Never define a default constructor to avoid this bug:
    // ScopeGuard(foo); // See Louis Brandy CPP2017
    ScopeGuard(std::function<void()> const & fn) : m_fn(fn) {}

    ~ScopeGuard() {m_fn(); }

    ScopeGuard(ScopeGuard const &) = delete;
    void operator=(ScopeGuard const &) = delete;
private:
    std::function<void()> m_fn;
};

// A version that allows you to ignore the return type of the end-of-scope function.
// This is required for VS2013 and for strict C++11 compliance:
template<typename T>
struct ScopeGuardT
{
    ScopeGuardT(std::function<T()> const & fn) : m_fn(fn) {}

    ~ScopeGuardT() {m_fn(); }

    ScopeGuardT(ScopeGuardT const &) = delete;
    void operator=(ScopeGuardT const &) = delete;
private:
    std::function<T()> m_fn;
};

}

#endif // FGSCOPEGUARD_HPP
