//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     July 23, 2004
//
// Basic type abbreviations and attributes
//

#ifndef FGTYPES_HPP
#define FGTYPES_HPP

#include "FgStdLibs.hpp"
#include "FgBoostLibs.hpp"
#include "FgPlatform.hpp"

// Shorter names for C++98 builtin types:
typedef signed char     schar;      // The C/C++ Standard does not specify whether 'char' is
                                    // signed  or unsigned. MSVC defaults 'char' to unsigned.
                                    // char is >= 1 bytes by the C++ standard.
typedef unsigned char   uchar;
typedef unsigned short  ushort;     // short is >= 2 bytes >= char by the C++ standard.
typedef unsigned int    uint;       // int is >= short by the C++ standard.
typedef unsigned long   ulong;      // long is >= 4 bytes >= int by the C++ standard.
typedef long long       int64;      // C99 / C++0x but widely supported.
typedef unsigned long long uint64;  // "

// Shorter names for C++11 fixed size integers:
typedef std::uint8_t  uint8;
typedef std::int8_t   int8;
typedef std::int16_t  int16;
typedef std::uint16_t uint16;
typedef std::int32_t  int32;
typedef std::uint32_t uint32;

// Useful if we need to initialize templated members only in the case of builtins:
template<class T> inline void fgInitializeBuiltinsToZero(T &) {}
template<> inline void fgInitializeBuiltinsToZero(char & v) {v=0;}
template<> inline void fgInitializeBuiltinsToZero(uchar & v) {v=0;}
template<> inline void fgInitializeBuiltinsToZero(schar & v) {v=0;}
template<> inline void fgInitializeBuiltinsToZero(short & v) {v=0;}
template<> inline void fgInitializeBuiltinsToZero(ushort & v) {v=0;}
template<> inline void fgInitializeBuiltinsToZero(int & v) {v=0;}
template<> inline void fgInitializeBuiltinsToZero(uint & v) {v=0;}
template<> inline void fgInitializeBuiltinsToZero(int64 & v) {v=0;}
template<> inline void fgInitializeBuiltinsToZero(uint64 & v) {v=0;}
template<> inline void fgInitializeBuiltinsToZero(float & v) {v=0.0f;}
template<> inline void fgInitializeBuiltinsToZero(double & v) {v=0.0;}
template<> inline void fgInitializeBuiltinsToZero(bool & v) {v=false;}

// Similar to above but gcc doesn't recognize above as initialization:
template<typename T> inline T fgDefaultVal() {return T(); }
template<> inline float fgDefaultVal() {return 0.0f; }
template<> inline double fgDefaultVal() {return 0.0; }
template<> inline int fgDefaultVal() {return 0; }
template<> inline uint fgDefaultVal() {return 0U; }
template<> inline int64 fgDefaultVal() {return 0LL; }
template<> inline uint64 fgDefaultVal() {return 0ULL; }

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
template<> struct FgTraits<long>
{
    typedef long    Scalar;
    typedef int64   Accumulator;
    typedef double  Floating;
};
template<> struct FgTraits<ulong>
{
    typedef ulong   Scalar;
    typedef uint64  Accumulator;
    typedef double  Floating;
};
template<> struct FgTraits<long long>
{
    typedef long long   Scalar;
    typedef long long   Accumulator;
    typedef double      Floating;
};
template<> struct FgTraits<unsigned long long>
{
    typedef unsigned long long  Scalar;
    typedef unsigned long long  Accumulator;
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

// Template stubs:

inline double fgMag(double v) {return v*v; }
inline double fgDot(double a,double b) {return a*b; }

inline void fgCast_(float  i,uchar &  o) {o = static_cast<uchar>(i); }
inline void fgCast_(uchar  i,float &  o) {o = static_cast<float>(i); }
inline void fgCast_(ushort i,float &  o) {o = static_cast<float>(i); }
inline void fgCast_(double i,float &  o) {o = static_cast<float>(i); }
inline void fgCast_(uchar  i,double & o) {o = static_cast<double>(i); }
inline void fgCast_(ushort i,double & o) {o = static_cast<double>(i); }
inline void fgCast_(float  i,double & o) {o = static_cast<double>(i); }
inline void fgCast_(unsigned int i,double & o) {o = static_cast<double>(i); }
inline void fgCast_(unsigned long i,double & o) {o = static_cast<double>(i); }
inline void fgCast_(unsigned long long i,double & o) {o = static_cast<double>(i); }

inline void fgRound_(float in,int & out) {out = static_cast<int>(std::floor(in + 0.5f)); }
inline void fgRound_(float in,uchar & out) {out = static_cast<uchar>(in + 0.5f); }
inline int fgRound(double v) {return static_cast<int>(std::floor(v+0.5)); }
inline uint fgRoundU(double v) {return static_cast<uint>(v+0.5); }

// Add default ordering to types:


#endif

// */
