//
// Copyright (C); Singular Inversions Inc. (facegen.com) 2017
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Macros to define default comparison operators for structs

#ifndef FGCMP_HPP
#define FGCMP_HPP

#include "FgTypes.hpp"

namespace Fg {

enum struct Cmp {lt,eq,gt};

template<typename T,
    FG_ENABLE_IF(T,is_arithmetic)
>
Cmp
cmp(T l,T r) {return (l<r) ? Cmp::lt : ((r<l) ? Cmp::gt : Cmp::eq); }

}

#define FGORDERED1(T,A)                                                                                 \
    bool operator<(T const & r) const {return (A < r.A); }                                              \
    bool operator==(T const & r) const {return (A == r.A); }
#define FGORDERED2(T,A,B)                                                                               \
    bool operator<(T const & r) const {                                                                 \
        if (A < r.A) return true; if (r.A < A) return false;                                            \
        return (B < r.B); }                                                                             \
    bool operator==(T const & r) const {return ((A == r.A) && (B == r.B)); }
#define FGORDERED3(T,A,B,C)                                                                             \
    bool operator<(T const & r) const {                                                                 \
        if (A < r.A) return true; if (r.A < A) return false;                                            \
        if (B < r.B) return true; if (r.B < B) return false;                                            \
        return (C < r.C); }                                                                             \
    bool operator==(T const & r) const                                                                  \
        {return ((A == r.A) && (B == r.B) && (C == r.C)); }
#define FGORDERED4(T,A,B,C,D)                                                                           \
    bool operator<(T const & r) const {                                                                 \
        if (A < r.A) return true; if (r.A < A) return false;                                            \
        if (B < r.B) return true; if (r.B < B) return false;                                            \
        if (C < r.C) return true; if (r.C < C) return false;                                            \
        return (D < r.D); }                                                                             \
    bool operator==(T const & r) const                                                                  \
        {return ((A == r.A) && (B == r.B) && (C == r.C) && (D == r.D)); }

#endif
