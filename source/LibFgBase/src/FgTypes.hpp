//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Platform, type abbreviations and attributes
//

#ifndef FGTYPES_HPP
#define FGTYPES_HPP

#include "FgStdLibs.hpp"

// ANSI defines:
// _DATE_
// _FILE_
// _LINE_
// _TIMESTAMP_
// NDEBUG               Controls expression of ANSI 'assert'.
//
// MSVC defines:
// _WIN32               Compiling for windows (32 or 64 bit)
// _WIN64               Targeting 64-bit ISA
// _DEBUG               Defined automatically by MSVC (/MTd or /MDd). We define for gcc/clang.
// _MSC_VER :
//      1920+ : VS2019 (v16.x)
//      1930+ : VS2022 (v17.x)
// _M_AMD64             Targeting Intel/AMD 64-bit ISA

// gcc,clang,icpc defines:
// __GNUC__             Compiler is gcc, clang or icpc
// __clang__            clang compiler
// __INTEL_COMPILER     Intel's icc and icpc compilers
// _LP64                LP64 paradigm
// __x86_64__           Intel/AMD 64 bit ISA
// __ppc64__            PowerPC 64 bit ISA
// __APPLE__            Defined on all apple platform compilers (along with __MACH__)
// __ANDROID__
//
// FaceGen defines:

// FG_64            Targeting 64-bit ISA (there is no cross-compiler standard for this)
#ifdef _WIN64
    #define FG_64
#elif _LP64
    #define FG_64
#endif

// FG_SANDBOX       Targeting a sandboxed platform (eg. Android, iOS, WebAssembly). No system() calls etc.
#ifdef __ANDROID__
    #define FG_SANDBOX
#endif
#ifdef TARGET_OS_IPHONE
    #define FG_SANDBOX
#endif

#ifdef _MSC_VER
// Too many false positives (avoid unnamed objects with custom construction or destruction):
#pragma warning(disable:4244)
#endif

namespace Fg {

// SHORTER ALIASES FOR FUNDAMENTAL TYPES:
// C++98
// The C/C++ Standard does not specify whether 'char' is signed  or unsigned.
// MSVC defaults 'char' to unsigned. char is >= 1 bytes by the C++ standard:
typedef signed char     schar;
typedef unsigned char   uchar;
typedef unsigned short  ushort;     // short is >= 2 bytes >= char by the C++ standard.
typedef unsigned int    uint;       // int is >= short by the C++ standard.
typedef unsigned long   ulong;      // long is >= 4 bytes >= int by the C++ standard.
typedef long long       int64;      // C99 / C++0x but widely supported.
typedef unsigned long long uint64;  // "
// C++11
typedef std::uint8_t    uint8;
typedef std::int8_t     int8;
typedef std::int16_t    int16;
typedef std::uint16_t   uint16;
typedef std::int32_t    int32;
typedef std::uint32_t   uint32;

// shorthand for static_cast along with an in-place version. Note that 'using' doesn't work here since
// 'static_cast' is not a normal function:
template<class T,class F> inline T      scast(F val) {return static_cast<T>(val); }
template<class T,class F> inline void   scast_(F from,T & to) {to = static_cast<T>(from); }

template<class T> using lims = std::numeric_limits<T>;

// SHORTER ALIASES FOR STANDARD LIBRARY TYPES
// convenient, but also useful for ensuring successful name lookup for functions overloaded on both
// untemplated std:: classes and FG classes.

template<class T,size_t S> using Arr = std::array<T,S>;

typedef Arr<uchar,2>            Arr2UC;
typedef Arr<int,2>              Arr2I;
typedef Arr<float,2>            Arr2F;
typedef Arr<double,2>           Arr2D;

typedef Arr<uchar,3>            Arr3UC;
typedef Arr<schar,3>            Arr3SC;
typedef Arr<ushort,3>           Arr3US;
typedef Arr<int,3>              Arr3I;
typedef Arr<uint,3>             Arr3UI;
typedef Arr<uint64,3>           Arr3UL;
typedef Arr<float,3>            Arr3F;
typedef Arr<double,3>           Arr3D;
typedef std::vector<Arr3F>      Arr3Fs;
typedef std::vector<Arr3D>      Arr3Ds;

typedef Arr<uchar,4>            Arr4UC;
typedef Arr<int,4>              Arr4I;
typedef Arr<uint,4>             Arr4UI;
typedef Arr<float,4>            Arr4F;
typedef Arr<double,4>           Arr4D;
typedef Arr<double,5>           Arr5D;

template<class T>   using Svec = std::vector<T>;
template<class T>   using Ptrs = std::vector<T const *>;

typedef Svec<bool>              Bools;
typedef Svec<std::byte>         Bytes;
typedef Svec<char>              Chars;
typedef Svec<signed char>       Schars;
typedef Svec<unsigned char>     Uchars;
typedef Svec<int>               Ints;
typedef Svec<uint>              Uints;
typedef Svec<size_t>            Sizes;
typedef Svec<int64>             Int64s;
typedef Svec<uint64>            Uint64s;
typedef Svec<float>             Floats;
typedef Svec<double>            Doubles;

typedef Svec<Bools>             Boolss;
typedef Svec<Doubles>           Doubless;
typedef Svec<Floats>            Floatss;
typedef Svec<Ints>              Intss;
typedef Svec<Uints>             Uintss;
typedef Svec<Sizes>             Sizess;

typedef std::string             String;
typedef std::u16string          Str16;
typedef std::u32string          Str32;       // Always assume UTF-32
typedef Svec<String>            Strings;
typedef Svec<Strings>           Stringss;
typedef Svec<Str32>             Str32s;

// shorthand for some std:: containers:
template<class T> using         Sfun = std::function<T>;
template<class T> using         Sptr = std::shared_ptr<T>;
template<class T> using         Uptr = std::unique_ptr<T>;
template<class T,class U> using Pair = std::pair<T,U>;
template<class T> using         Opt = std::optional<T>;

// CROSS-PLATFORM ARCHITECTURE INFORMATION:
// handy for avoiding ugly macros when things compile cross-platform but we want conditional behaviour:
bool constexpr      isCompiledWithMsvc()
{
#ifdef _MSC_VER
    return true;
#else
    return false;
#endif
}
bool constexpr      is64Bit()
{
#ifdef FG_64
    return true;
#else
    return false;
#endif
}
bool constexpr      is32Bit() {return !is64Bit(); }
bool constexpr      isDebug()
{
#ifdef _DEBUG
    return true;
#else
    return false;
#endif
}
bool constexpr      isRelease() {return !isDebug(); }

// Returns "32" if the current executable is 32-bit, "64" if 64-bit:
std::string         cBitsString();

// numeric aggregate type traits includes scalar base case so that arbitrary depths of type nesting of numeric
// aggregates can be automatically handled in template code:
template<class T> struct Traits;
// Scalar - the underlying scalar type of, eg., a std::array of std::vector of Fg::MatrixC
// Accumulator - same nested type except that Scalar is replaced with an accumulator type
// Floating - same nested type except that Scalar is replace with 'float'
// Printable - only affect structures of uchar/schar which don't print numbers with std::ostream
template<> struct Traits<uchar>
{
    typedef uchar   Scalar;
    typedef uint    Accumulator;
    typedef float   Floating;
    typedef uint    Printable;
};
template<> struct Traits<schar>
{
    typedef schar   Scalar;
    typedef int     Accumulator;
    typedef float   Floating;
    typedef int     Printable;
};
template<> struct Traits<int>
{
    typedef int     Scalar;
    typedef int64   Accumulator;
    typedef float   Floating;
    typedef int     Printable;
};
template<> struct Traits<ushort>
{
    typedef ushort  Scalar;
    typedef uint64  Accumulator;
    typedef float   Floating;
    typedef ushort  Printable;
};
template<> struct Traits<uint>
{
    typedef uint    Scalar;
    typedef uint64  Accumulator;
    typedef float   Floating;
    typedef uint    Printable;
};
template<> struct Traits<ulong>
{
    typedef ulong   Scalar;
    typedef size_t  Accumulator;
    typedef double  Floating;
    typedef ulong   Printable;
};
template<> struct Traits<int64>
{
    typedef int64   Scalar;
    typedef int64   Accumulator;
    typedef double  Floating;
    typedef int64   Printable;
};
template<> struct Traits<uint64>
{
    typedef uint64  Scalar;
    typedef uint64  Accumulator;
    typedef double  Floating;
    typedef uint64  Printable;
};
template<> struct Traits<float>
{
    typedef float   Scalar;
    typedef double  Accumulator;
    typedef float   Floating;
    typedef float   Printable;
};
template<> struct Traits<double>
{
    typedef double  Scalar;
    typedef double  Accumulator;
    typedef double  Floating;
    typedef double  Printable;
};
template<> struct Traits<std::complex<double>>
{
    typedef std::complex<double>    Scalar;
    typedef std::complex<double>    Accumulator;
    typedef std::complex<double>    Floating;
    typedef std::complex<double>    Printable;
};
template<class T,size_t S>
struct  Traits<Arr<T,S>>
{
    typedef typename Traits<T>::Scalar              Scalar;
    typedef Arr<typename Traits<T>::Accumulator,S>  Accumulator;
    typedef Arr<typename Traits<T>::Floating,S>     Floating;
};
template<class T>
struct Traits<Svec<T>>
{
    typedef typename Traits<T>::Scalar              Scalar;
    typedef Svec<typename Traits<T>::Accumulator>   Accumulator;
    typedef Svec<typename Traits<T>::Floating>      Floating;
};

#define FG_ENABLE_IF(Type,Trait) typename std::enable_if<std::Trait<Type>::value,Type>::type* =nullptr

}

#endif

// */
