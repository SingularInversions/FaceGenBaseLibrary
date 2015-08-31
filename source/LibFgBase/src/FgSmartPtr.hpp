//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: July 16, 2012
//

#ifndef FGSINGLEPTR_HPP
#define FGSINGLEPTR_HPP

#include "FgStdLibs.hpp"
#include "FgDiagnostics.hpp"

// Always keeps ownership of pointer; copies get NULL value.
// Useful for composition of instance-local pointers (eg. to cached values)
// in objects that can be copied:
template<typename T>
struct FgSinglePtr
{
    FgSinglePtr() : ptr(0) {}

    explicit FgSinglePtr(T * p) : ptr(p) {}

    // Dummy CC
    FgSinglePtr(FgSinglePtr<T> const &) : ptr(0) {}

    FgSinglePtr<T> &
    operator=(T * p) {
        if (ptr != 0)
            delete ptr;
        ptr = p;
        return *this;
    }

    ~FgSinglePtr() {
        if (ptr != 0)
            delete ptr; }

    // Dummy =
    FgSinglePtr<T> &
    operator=(FgSinglePtr<T> const &) {
        if (ptr != 0)
            delete ptr;
        ptr = 0;
        return *this; }

    T *
    operator->() const {
        FGASSERT_FAST(ptr != 0);
        return ptr; }

    T &
    operator*() const {
        FGASSERT_FAST(ptr != 0);
        return *ptr; }

    operator bool() const
    {return (ptr != 0); }

private:
    T *         ptr;
};

// Handy for pointers which need to be de-allocated when going out of scope
// and also checked for validity when assigned:
template<typename T>
struct  FgScopePtr
{
    T *                         ptr;
    boost::function<void(T*)>   dealloc;

    FgScopePtr() : ptr(0) {}

    FgScopePtr(T*p,boost::function<void(T*)> d)
        : ptr(p), dealloc(d)
    {FGASSERT(p != 0); }

    T *
    get() const
    {return ptr; }

    T *
    operator->()
    {return ptr; }

    ~FgScopePtr()
    {
        if (ptr != 0)
            dealloc(ptr);
    }
};

#endif
