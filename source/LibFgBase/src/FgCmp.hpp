//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Macros to define comparison operators for structs

#ifndef FGCMP_HPP
#define FGCMP_HPP

#include "FgTypes.hpp"

namespace Fg {

enum struct Cmp { lt=-1, eq=0, gt=1 };

// Default for compound types is to redirect to member operation:
template<typename T,
    FG_ENABLE_IF(T,is_compound)
>
inline Cmp cmp(T const & l,T const & r) {return l.cmp(r); }

// Base cases include all scalars:
template<typename T,
    FG_ENABLE_IF(T,is_scalar)
>
Cmp cmp(T l,T r) {return (l<r) ? Cmp::lt : ((r<l) ? Cmp::gt : Cmp::eq); }

#define FG_CMP_M1(T,A)                                                                      \
    bool cmp(T const & r) const {return cmp(A,r.A); }

#define FG_CMP_M2(T,A,B)                                                                    \
    bool cmp(T const & r) const {                                                           \
        Cmp         c = cmp(A,r.A);                                                         \
        if (c != Cmp::eq) return c;                                                         \
        return cmp(B,r.B);                                                                  \
     }                                                                                   

#define FG_CMP_M3(T,A,B,C)                                                                  \
    bool cmp(T const & r) const {                                                           \
        Cmp         c = cmp(A,r.A);                                                         \
        if (c != Cmp::eq) return c;                                                         \
        c = cmp(B,r.B);                                                                     \
        if (c != Cmp::eq) return c;                                                         \
        return cmp(C,r.C);                                                                  \
     }                                                                                   

#define FG_LTE_M1(T,A)                                                                                  \
    bool operator<(T const & r) const {return (A < r.A); }                                              \
    bool operator==(T const & r) const {return (A == r.A); }

#define FG_LTE_M2(T,A,B)                                                                                \
    bool operator<(T const & r) const {                                                                 \
        if (A < r.A) return true; if (r.A < A) return false;                                            \
        return (B < r.B); }                                                                             \
    bool operator==(T const & r) const {return ((A == r.A) && (B == r.B)); }

#define FG_LTE_M3(T,A,B,C)                                                                              \
    bool operator<(T const & r) const {                                                                 \
        if (A < r.A) return true; if (r.A < A) return false;                                            \
        if (B < r.B) return true; if (r.B < B) return false;                                            \
        return (C < r.C); }                                                                             \
    bool operator==(T const & r) const                                                                  \
        {return ((A == r.A) && (B == r.B) && (C == r.C)); }

#define FG_LTE_M4(T,A,B,C,D)                                                                           \
    bool operator<(T const & r) const {                                                                 \
        if (A < r.A) return true; if (r.A < A) return false;                                            \
        if (B < r.B) return true; if (r.B < B) return false;                                            \
        if (C < r.C) return true; if (r.C < C) return false;                                            \
        return (D < r.D); }                                                                             \
    bool operator==(T const & r) const                                                                  \
        {return ((A == r.A) && (B == r.B) && (C == r.C) && (D == r.D)); }

// When only equality is needed:

#define FG_EQ_M1(T,A) bool operator==(T const & r) const {return (A == r.A); }
#define FG_EQ_M2(T,A,B) bool operator==(T const & r) const {return ((A == r.A) && (B == r.B)); }
#define FG_EQ_M3(T,A,B,C) bool operator==(T const & r) const {return ((A == r.A) && (B == r.B) && (C == r.C)); }

}

#endif
