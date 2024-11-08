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

#define FG_ENABLE_IF(Type,Trait) typename std::enable_if<std::Trait<Type>::value,Type>::type* =nullptr

namespace Fg {

// SHORTER ALIASES FOR FUNDAMENTAL TYPES:
// * The C/C++ Standard does not specify whether 'char' is signed  or unsigned.
// * MSVC defaults 'char' to unsigned. char is >= 1 bytes by the C++ standard.
// * sizeof(int / uint) = 32 is assumed in some of this codebase
// from C++ 98:
typedef signed char     schar;
typedef unsigned char   uchar;
typedef unsigned short  ushort;     // short is >= 2 bytes >= char by the standard.
typedef unsigned int    uint;       // int is >= short by the standard.
typedef unsigned long   ulong;      // long is >= 4 bytes >= int by the standard.
// from C++11
typedef std::uint8_t    uint8;
typedef std::int8_t     int8;
typedef std::int16_t    int16;
typedef std::uint16_t   uint16;
typedef std::int32_t    int32;
typedef std::uint32_t   uint32;
typedef std::int64_t    int64;
typedef std::uint64_t   uint64;

// shorthand for static_cast along with an in-place version. Note that 'using' doesn't work here since
// 'static_cast' is not a normal function:
template<class T,class F>
inline T constexpr      scast(F val) {return static_cast<T>(val); }

template<class T> using lims = std::numeric_limits<T>;

// use instead of std::array to get better constructors:
// * single-curly-brace element list
// * single-arg fill ctor
// * error if wrong number of terms (other than single-arg)
// * construct using emplace_back
template<class T,size_t S>
class       Arr
{
    T               m[S];

public:
    // No initialization is done for types without a default ctor. Default zero-fill is not a solution, be explicit:
    Arr() {}
    explicit constexpr Arr(T v) {for (size_t ii=0; ii<S; ++ii) m[ii] = v; }
    constexpr Arr(T x,T y) : m {x,y}
    {static_assert(S == 2,"Number of arguments does not match elements"); }   //-V557 (for PVS-Studio)
    constexpr Arr(T x,T y,T z) : m {x,y,z}
    {static_assert(S == 3,"Number of arguments does not match elements"); }   //-V557
    constexpr Arr(T a,T b,T c,T d) : m {a,b,c,d}
    {static_assert(S == 4,"Number of arguments does not match elements"); }   //-V557
    constexpr Arr(T a,T b,T c,T d,T e) : m {a,b,c,d,e}
    {static_assert(S == 5,"Number of arguments does not match elements"); }   //-V557
    constexpr Arr(T a,T b,T c,T d,T e,T f) : m {a,b,c,d,e,f}
    {static_assert(S == 6,"Number of arguments does not match elements"); }   //-V557
    constexpr Arr(T a,T b,T c,T d,T e,T f,T g,T h,T i) : m {a,b,c,d,e,f,g,h,i}
    {static_assert(S == 9,"Number of arguments does not match elements"); }   //-V557
    constexpr Arr(T a,T b,T c,T d,T e,T f,T g,T h,T i,T j,T k,T l) : m {a,b,c,d,e,f,g,h,i,j,k,l}
    {static_assert(S == 12,"Number of arguments does not match elements"); }   //-V557
    constexpr Arr(T a,T b,T c,T d,T e,T f,T g,T h,T i,T j,T k,T l,T m,T n,T o,T p) : m {a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p}
    {static_assert(S == 16,"Number of arguments does not match elements"); }   //-V557

    size_t constexpr    size() const {return S; }
    T const *           begin() const {return &m[0]; }
    T const *           end() const {return &m[0]+S; }
    T *                 begin() {return &m[0]; }
    T *                 end() {return &m[0]+S; }
    T const &           operator[](size_t xx) const {return m[xx]; }
    T &                 operator[](size_t xx) {return m[xx]; }
    T const &           back() const {return m[S-1]; }
    T &                 back() {return m[S-1]; }
    void                fill(T v) {for (size_t ii=0; ii<S; ++ii) m[ii] = v; }
    bool                operator==(Arr const & rhs) const
    {
        for (size_t ii=0; ii<S; ++ii)
            if (!(m[ii] == rhs.m[ii]))
                return false;
        return true;
    }
    bool                operator!=(Arr const & rhs) const {return !(*this == rhs); }
    bool                operator<(Arr const & rhs) const    // handy when we need a sorted order
    {
        for (size_t ii=0; ii<S; ++ii) {    // arbitrarily choose first element as most significant
            if (m[ii] < rhs.m[ii])
                return true;
            if (rhs.m[ii] < m[ii])
                return false;
        }
        return false;           // equal
    }
};

// SHORTER ALIASES FOR STANDARD LIBRARY TYPES
// convenient, but also useful for ensuring successful name lookup for functions overloaded on both
// untemplated std:: classes and FG classes.

template<class T>           using Svec = std::vector<T>;
template<class T>           using Ptrs = std::vector<T const *>;

typedef Arr<bool,2>             Arr2B;
typedef Arr<uchar,2>            Arr2UC;
typedef Arr<int,2>              Arr2I;
typedef Arr<uint,2>             Arr2UI;
typedef Arr<uint64,2>           Arr2UL;
typedef Arr<size_t,2>           Arr2Z;
typedef Arr<float,2>            Arr2F;
typedef Arr<double,2>           Arr2D;
typedef Svec<Arr2UI>            Arr2UIs;
typedef Svec<Arr2UIs>           Arr2UIss;
typedef Svec<Arr2UIss>          Arr2UIsss;

typedef Arr<bool,3>             Arr3B;
typedef Arr<uchar,3>            Arr3UC;
typedef Arr<schar,3>            Arr3SC;
typedef Arr<ushort,3>           Arr3US;
typedef Arr<int,3>              Arr3I;
typedef Arr<uint,3>             Arr3UI;
typedef Arr<uint64,3>           Arr3UL;
typedef Arr<float,3>            Arr3F;
typedef Arr<double,3>           Arr3D;
typedef Svec<Arr3B>             Arr3Bs;
typedef Svec<Arr3UI>            Arr3UIs;
typedef Svec<Arr3F>             Arr3Fs;
typedef Svec<Arr3D>             Arr3Ds;
typedef Svec<Arr3UIs>           Arr3UIss;

typedef Arr<uchar,4>            Arr4UC;
typedef Arr<int,4>              Arr4I;
typedef Arr<uint,4>             Arr4UI;
typedef Arr<float,4>            Arr4F;
typedef Arr<double,4>           Arr4D;
typedef Arr<double,5>           Arr5D;
typedef Svec<Arr4D>             Arr4Ds;
typedef Svec<Arr4UI>            Arr4UIs;

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
// Floating - same nested type except that Scalar is replace with 'float'
// Printable - only affect structures of uchar/schar which don't print numbers with std::ostream
template<> struct Traits<uchar>
{
    typedef uchar   Scalar;
    typedef float   Floating;
    typedef uint    Printable;
};
template<> struct Traits<schar>
{
    typedef schar   Scalar;
    typedef float   Floating;
    typedef int     Printable;
};
template<> struct Traits<ushort>
{
    typedef ushort  Scalar;
    typedef float   Floating;
    typedef ushort  Printable;
};
template<> struct Traits<int32>
{
    typedef int32   Scalar;
    typedef float   Floating;
    typedef int     Printable;
};
template<> struct Traits<uint32>
{
    typedef uint32  Scalar;
    typedef float   Floating;
    typedef uint    Printable;
};
template<> struct Traits<int64>
{
    typedef int64   Scalar;
    typedef double  Floating;
    typedef int64   Printable;
};
template<> struct Traits<uint64>
{
    typedef uint64  Scalar;
    typedef double  Floating;
    typedef uint64  Printable;
};
template<> struct Traits<float>
{
    typedef float   Scalar;
    typedef float   Floating;
    typedef float   Printable;
};
template<> struct Traits<double>
{
    typedef double  Scalar;
    typedef double  Floating;
    typedef double  Printable;
};
template<> struct Traits<std::complex<double>>
{
    typedef std::complex<double>    Scalar;
    typedef std::complex<double>    Floating;
    typedef std::complex<double>    Printable;
};
template<> struct Traits<bool>
{
    typedef bool    Printable;
};
template<class T,size_t S>
struct  Traits<Arr<T,S>>
{
    typedef typename Traits<T>::Scalar              Scalar;
    typedef Arr<typename Traits<T>::Floating,S>     Floating;
};
template<class T>
struct Traits<Svec<T>>
{
    typedef typename Traits<T>::Scalar              Scalar;
    typedef Svec<typename Traits<T>::Floating>      Floating;
};

}

#endif

// */
