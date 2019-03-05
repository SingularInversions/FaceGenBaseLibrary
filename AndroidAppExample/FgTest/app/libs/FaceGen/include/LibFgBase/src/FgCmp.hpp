//
// Copyright (C); Singular Inversions Inc. (facegen.com) 2017
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Nov 22, 2017
//
// Macros to define default comparison operators for structs

#ifndef FGCMP_HPP
#define FGCMP_HPP

#include "FgStdLibs.hpp"

#define FGORDERED1(T,A)                                                                                 \
    bool operator<(const T & r) const {return (A < r.A); }                                              \
    bool operator==(const T & r) const {return (A == r.A); }
#define FGORDERED2(T,A,B)                                                                               \
    bool operator<(const T & r) const {                                                                 \
        if (A < r.A) return true; if (r.A < A) return false;                                            \
        return (B < r.B); }                                                                             \
    bool operator==(const T & r) const {return ((A == r.A) && (B == r.B)); }
#define FGORDERED3(T,A,B,C)                                                                             \
    bool operator<(const T & r) const {                                                                 \
        if (A < r.A) return true; if (r.A < A) return false;                                            \
        if (B < r.B) return true; if (r.B < B) return false;                                            \
        return (C < r.C); }                                                                             \
    bool operator==(const T & r) const                                                                  \
        {return ((A == r.A) && (B == r.B) && (C == r.C)); }
#define FGORDERED4(T,A,B,C,D)                                                                           \
    bool operator<(const T & r) const {                                                                 \
        if (A < r.A) return true; if (r.A < A) return false;                                            \
        if (B < r.B) return true; if (r.B < B) return false;                                            \
        if (C < r.C) return true; if (r.C < C) return false;                                            \
        return (D < r.D); }                                                                             \
    bool operator==(const T & r) const                                                                  \
        {return ((A == r.A) && (B == r.B) && (C == r.C) && (D == r.D)); }

#endif
