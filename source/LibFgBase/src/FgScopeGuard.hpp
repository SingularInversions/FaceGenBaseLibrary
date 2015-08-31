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
    FgScopeGuard(boost::function<void()> const & fn):
        m_fn(fn)
    {}

    ~FgScopeGuard()
    { m_fn(); }

private:
    boost::function<void()> m_fn;
};

#endif // FGSCOPEGUARD_HPP
