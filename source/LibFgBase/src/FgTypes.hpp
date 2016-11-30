//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     July 23, 2004
//
// Basic builtin-type abbreviations and attributes
//
// DESIGN
//
// For x64 support we make use of 'long long', which is not in the C++98 standard but
// is widely supported (being in the C99 and C++0x standards).
//

#ifndef FGTYPES_HPP
#define FGTYPES_HPP

#include "FgStdLibs.hpp"
#include "FgBoostLibs.hpp"
#include "FgPlatform.hpp"

// C++ builtin type aliases:
typedef signed char     schar;      // The C/C++ Standard does not specify whether 'char' is
                                    // signed  or unsigned. MSVC defaults 'char' to unsigned.
                                    // char is >= 1 bytes by the C++ standard.
typedef unsigned char   uchar;
typedef unsigned short  ushort;     // short is >= 2 bytes >= char by the C++ standard.
typedef unsigned int    uint;       // int is >= short by the C++ standard.
typedef unsigned long   ulong;      // long is >= 4 bytes >= int by the C++ standard.
typedef long long       int64;      // C99 / C++0x but widely supported.
typedef unsigned long long uint64;  // "

typedef boost::uint8_t  uint8;
typedef boost::int8_t   int8;
typedef boost::int16_t  int16;
typedef boost::uint16_t uint16;
typedef boost::int32_t  int32;
typedef boost::uint32_t uint32;

template<typename T> struct FgTypeAttributeFloatingS;   // Type must be floating point.
template<> struct FgTypeAttributeFloatingS<float> {};
template<> struct FgTypeAttributeFloatingS<double> {};

template<typename T> struct FgTypeAttributeFixedS;      // Type must be fixed point.
template<> struct FgTypeAttributeFixedS<uchar> {};
template<> struct FgTypeAttributeFixedS<short> {};
template<> struct FgTypeAttributeFixedS<ushort> {};
template<> struct FgTypeAttributeFixedS<int> {};
template<> struct FgTypeAttributeFixedS<uint> {};
template<> struct FgTypeAttributeFixedS<long> {};
template<> struct FgTypeAttributeFixedS<ulong> {};
template<> struct FgTypeAttributeFixedS<int64> {};
template<> struct FgTypeAttributeFixedS<uint64> {};

template<class T> struct FgTraits;

template<> struct FgTraits<uchar>
{
    typedef uchar   Scalar;             // Stub for scalar type of templated vector/matrix types
    typedef uint    Accumulator;
    typedef double  Floating;           // Stub for conversion of templated vec/mat to floating point
};
template<> struct FgTraits<schar>
{
    typedef schar   Scalar;
    typedef int     Accumulator;
    typedef double  Floating;
};
template<> struct FgTraits<int>
{
    typedef int     Scalar;
    typedef int64   Accumulator;
    typedef double  Floating;
};
template<> struct FgTraits<uint>
{
    typedef uint    Scalar;
    typedef uint64  Accumulator;
    typedef double  Floating;
};
template<> struct FgTraits<uint64>
{
    typedef uint64  Scalar;
    typedef uint64  Accumulator;
    typedef double  Floating;
};
template<> struct FgTraits<float>
{
    typedef float   Scalar;
    typedef double  Accumulator;
    typedef double  Floating;
};
template<> struct FgTraits<double>
{
    typedef double  Scalar;
    typedef double  Accumulator;
    typedef double  Floating;
};

// Handy within templated algorithms:

inline double fgMag(double v) {return v*v; }    // Template stub

inline void fgRound_(float in,int & out) {out = static_cast<int>(std::floor(in + 0.5f)); }
inline void fgRound_(float in,uchar & out) {out = static_cast<uchar>(in + 0.5f); }
inline int fgRound(double v) {return static_cast<int>(std::floor(v+0.5)); }
inline uint fgRoundU(double v) {return static_cast<uint>(v+0.5); }

// Useful for implicit typing in type conversion:
template<class T,class U>
inline void
fgConvert_(const T & in,U & out)
{out = static_cast<U>(in); }

#endif

// */
