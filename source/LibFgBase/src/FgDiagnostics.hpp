//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     May 7, 2004
//
// USE:
//
// FGASSERT is retained release builds, FG_ASSERT_FAST is not.
// Use FGASSERT for unexpected errors (use 'fgThrow' otherwise)
//

#ifndef FGDIAGNOSTICS_HPP
#define FGDIAGNOSTICS_HPP

#include "FgException.hpp"
#include "FgBoostLibs.hpp"

std::string
fgDiagString(const char *fname,int line);

// With visual studio 2012 the __FILE__ macro always includes the full path in release compiles,
// there is no way to specify otherwise. When this includes unicode, the literal becomes
// a wchar_t* instead of char*. Macros can be used to cast to wchar_t* in both cases, then
// dealt with but I haven't bothered; currently the source will not compile properly in a
// non-ascii path.

void
fgAssert(
    const char *        fname,
    int line,
    const std::string & msg = "");

// We use an 'if' 'else' structure for the macro to avoid the dangling 'else' bug.
// Leave off the semi-colon on the second line to force a compile error:
#define FGASSERT(X)                                                     \
    if(X) (void) 0;                                                     \
    else fgAssert(__FILE__,__LINE__)

#define FGASSERT1(X,msg)                                            \
    if(X) (void) 0;                                                     \
    else fgAssert(__FILE__,__LINE__,msg)

#ifdef _DEBUG

#define FGASSERT_FAST(X)                                                \
    if(X) (void) 0;                                                     \
    else fgAssert(__FILE__,__LINE__)

#define FGASSERT_FAST2(X,Y)                     \
    if(X) (void) 0;                             \
    else fgAssert(__FILE__,__LINE__,Y)          \

#else

#define FGASSERT_FAST(X)
#define FGASSERT_FAST2(X,Y)

#endif

// Use this instead of FGASSERT(false) to avoid warnings for constant conditionals:
#define FGASSERT_FALSE                                                  \
    fgAssert(__FILE__,__LINE__)

#define FGASSERT_FALSE1(msg)                                         \
    fgAssert(__FILE__,__LINE__,msg)

// Use this when you have to provide a return expression but the code
// will never be executed:
#ifdef _MSC_VER
#  ifdef _DEBUG
#    define FG_UNREACHABLE_RETURN(x) return x
#  else
#    define FG_UNREACHABLE_RETURN(x) (void)x
#  endif
#else
#  define FG_UNREACHABLE_RETURN(x) return x
#endif

#define FG_STATIC_ASSERT(X) BOOST_STATIC_ASSERT(X)

// Convenient for debugging:

#define FG_HI_                                                      \
std::cout << std::endl << "HI ! (" << __FILE__ << ": " << __LINE__       \
        << ")" << std::flush

#define FG_HI_1(X)                                                  \
    std::cout << std::endl << #X ": " << (X) << std::flush

#define FG_HI_2(X,Y)                                                \
	std::cout << std::endl << #X ": " << (X) << " "                 \
        << #Y ": " << (Y) << std::flush

#define FG_HI_3(X,Y,Z)                                              \
    std::cout << std::endl << #X ": " << (X) << " "                 \
         << #Y ": " << (Y) << " "                                   \
         << #Z ": " << (Z) << std::flush

#define FG_HI_4(X,Y,Z,A)                                            \
    std::cout << std::endl << #X ": " << (X) << " "                 \
         << #Y ": " << (Y) << " "                                   \
         << #Z ": " << (Z) << " "                                   \
         << #A ": " << (A) << std::flush

// Use string for debug build, empty string otherwise:
#ifdef _DEBUG
#define FG_DBG_STR(str) str
#else
#define FG_DBG_STR(str) ""
#endif

// Handy way of naming log files using current date and time (does not append a suffix):
std::string
fgDateTimeString();

#endif
