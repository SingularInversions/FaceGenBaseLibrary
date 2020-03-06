//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Basic type abbreviations and attributes
//

#ifndef FGTYPES_HPP
#define FGTYPES_HPP

#include "FgStdLibs.hpp"
#include "FgPlatform.hpp"

namespace Fg {

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
typedef std::uint8_t    uint8;
typedef std::int8_t     int8;
typedef std::int16_t    int16;
typedef std::uint16_t   uint16;
typedef std::int32_t    int32;
typedef std::uint32_t   uint32;

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

template<class T> struct Traits;

template<> struct Traits<uchar>
{
    // Stub for scalar type of templated vector/matrix types:
    typedef uchar   Scalar;
    typedef uint    Accumulator;
    // Stub for conversion of templated vec/mat to floating point for fractional operations (not accumulation):
    typedef float   Floating;
};
template<> struct Traits<schar>
{
    typedef schar   Scalar;
    typedef int     Accumulator;
    typedef float   Floating;
};
template<> struct Traits<int>
{
    typedef int     Scalar;
    typedef int64   Accumulator;
    typedef float   Floating;
};
template<> struct Traits<uint>
{
    typedef uint    Scalar;
    typedef uint64  Accumulator;
    typedef float   Floating;
};
// MSVC and Android do not consider size_t to be its own type but others do:
#ifdef _MSC_VER
#elif defined(__ANDROID__)
#else
template<> struct Traits<size_t>
{
    typedef size_t  Scalar;
    typedef size_t  Accumulator;
    typedef double  Floating;
};
#endif
template<> struct Traits<int64>
{
    typedef int64   Scalar;
    typedef int64   Accumulator;
    typedef double  Floating;
};
template<> struct Traits<uint64>
{
    typedef uint64  Scalar;
    typedef uint64  Accumulator;
    typedef double  Floating;
};
template<> struct Traits<float>
{
    typedef float   Scalar;
    typedef double  Accumulator;
    typedef float   Floating;
};
template<> struct Traits<double>
{
    typedef double  Scalar;
    typedef double  Accumulator;
    typedef double  Floating;
};

// Template resolution base case:
template<typename T>
inline T
sfloor(T v)
{return std::floor(v); }

// Template resolution base base:
template<typename To,typename From>
inline void
scast_(From from,To & to)
{to = static_cast<To>(from); }

// Abbreviation for static_cast. Was unable to make recursive template resolution due
// to difficulty of specifying return type. Instead, each combination of containers to be used
// with it needs to be its own template function:
template<typename To,typename From>
inline To
scast(From val)
{return static_cast<To>(val); }

#define FG_ENABLE_IF(Type,Trait) typename std::enable_if<std::Trait<Type>::value,Type>::type* =nullptr

// 'round' for static cast which does proper rounding when necessary:
// No bounds checking is done by these 'round' functions:

template<typename To,typename From,
    FG_ENABLE_IF(To,is_signed),
    FG_ENABLE_IF(To,is_integral)
>
inline To
round(From v)
{
    static_assert(std::is_floating_point<From>::value,"round only from floating point");
    return static_cast<To>(std::floor(v+From(0.5)));
}

template<typename To,typename From,
    FG_ENABLE_IF(To,is_unsigned),
    FG_ENABLE_IF(To,is_integral)
>
inline To
round(From v)
{
    static_assert(std::is_floating_point<From>::value,"round only from floating point");
    return static_cast<To>(v+From(0.5));
}

template<typename To,typename From,
    FG_ENABLE_IF(To,is_floating_point)
>
inline To
round(From v)
{
    static_assert(std::is_floating_point<From>::value,"round only from floating point");
    return static_cast<To>(v);
}

template<typename To,typename From>
void
round_(From from,To & to)
{to = round<To,From>(from); }

template<typename T,
    FG_ENABLE_IF(T,is_arithmetic)
>
T
interpolate(T v0,T v1,float val)   // returns v0 when val==0, v1 when val==1
{
    float       v = static_cast<float>(v0) * (1.0f-val) + static_cast<float>(v1) * val;
    return round<T,float>(v);
}

// Squared magnitude function base cases:
inline float cMag(float v) {return v*v; }
inline double cMag(double v) {return v*v; }
inline double cMag(std::complex<double> v) {return std::norm(v); }
template<typename T,size_t S>
double
cMag(std::array<T,S> const a)
{
    T       acc(0);
    for (T const & e : a)
        acc += cMag(e);
    return acc;
}

}

#endif

// */
