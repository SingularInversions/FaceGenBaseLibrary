//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Jan 19, 2005
//
// Platform and compiler specific definitions.
//
// Relevent ANSI defines:
// _DATE_
// _FILE_
// _LINE_
// _TIMESTAMP_
// _DEBUG           Not ANSI but universal

#ifndef FGPLATFORM_HPP
#define FGPLATFORM_HPP

// Visual Studio name - _MSC_VER - vc++ version
// VS2013 - 1800 - 12.0
// VS2015 - 1900 - 14.0
// VS2017 - 1910 - 14.1
// VS2017 - 1911 - 14.1
// VS2017 - 1912 - 14.1
#ifdef _MSC_VER

    // Useful defines:
    // _WIN32       We are compiling for windows (32 or 64 bit)
    // _WIN64       We are compiling for 64 bit windows
    // _M_AMD64     Defined if targetting 64-bit

    #define FG_RESTRICT __restrict

    #ifdef _WIN64
    #define FG_64
    #endif
 
#elif defined(__GNUC__)     // gcc, clang or icpc

    // Useful defines:
    // _LP64        LP64 paradigm
    // __x86_64__   64-bit instruction set
    // __ppc64__    ppc 64 bit instruction set

    #define FG_RESTRICT __restrict__

    #ifdef _LP64
    #define FG_64
    #endif

// If we need compiler-specific behaviour, check for
// __clang__            clang compiler
// __INTEL_COMPILER     Intel's icc and icpc compilers
#else

    compiler_definitions_here;          // Put your compiler's equivalents here (in an #elif).

#endif

int
fgSizeofPtr();    // avoid 'conditional expression is constant' errors

// Is current binary 64 bit (avoid 'conditional expression is constant') ?
bool
fgIs64bit();

// As above
bool
fgIsDebug();

// Returns "32" if the current EXE is 32-bit, "64" if 64-bit.
std::string
fgBitsString();

#endif
