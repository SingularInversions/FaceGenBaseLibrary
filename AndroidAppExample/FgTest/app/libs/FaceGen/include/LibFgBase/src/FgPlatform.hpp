//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Jan 19, 2005
//
// Compile target platform and compiler specific definitions.
//
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
//      1800 : VS2013 12.0
//      1900 : VS2015 14.0
//      1910-1916 : VS2017 14.1
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

#ifndef FGPLATFORM_HPP
#define FGPLATFORM_HPP

// FaceGen defines:

// FG_64            Targeting 64-bit ISA (there is no cross-compiler standard for this)
#ifdef _WIN64
    #define FG_64
#elif __x86_64__
    #define FG_64
#endif

// FG_SANDBOX       Targeting a sandboxed platform (eg. Android, iOS, WebAssembly). No system() calls etc.
#ifdef __ANDROID__
    #define FG_SANDBOX
#endif
#if defined(__APPLE__) && defined(ENABLE_BITCODE)
    // Including "TargetConditionals.h" and testing TARGET_OS_IPHONE does not work because this file
    // is in sysroot/usr/include/ which doesn't compile with C++ (C only):
    #define FG_SANDBOX
#endif

int
fgSizeofPtr();    // avoid 'conditional expression is constant' errors

// Is current binary 64 bit (avoid 'conditional expression is constant') ?
bool
fgIs64bit();

// As above
bool
fgIsDebug();

// Returns "32 if the current executable is 32-bit, "64" if 64-bit.
std::string
fgBitsString();

#endif
