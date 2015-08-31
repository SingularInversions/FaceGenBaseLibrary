//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Sohail Somani, Andrew Beatty
//
// Platform-independent macros for pure C DLL interface specification.
//
// USE:
//
// * #include this file in your FgSomeDll.h file
// * Use 'FG_DLL_DECLARE' for function declarations in that file
// * #define FG_DLL_DEFINITION_FILE at the top of your FgSomeDll.cpp file
// * then #include this file and use 'FG_DLL_DEFINE' for function definitions.
//
// DESIGN:
//
// The mutable nature of the function declarations is necessary due to the 
// idiotic design of Microsoft DLL declaration specifications.
//

#ifndef FGDYNLIB_H
#define FGDYNLIB_H

#if defined(_WIN32)
  #ifdef FG_DLL_DEFINITION_FILE
    #define FG_DLL_DECLARE __declspec(dllexport)
  #else
    #define FG_DLL_DECLARE __declspec(dllimport)
  #endif
#else
#  if __GNUC__ >= 4
#    define FG_DLL_DECLARE __attribute__((visibility("default")))
#  endif
#endif

#if defined(_WIN32)
#  define FG_DLL_DEFINE extern "C" __declspec(dllexport)
#else
#  if __GNUC__ >= 4
#    define FG_DLL_DEFINE extern "C" __attribute__((visibility("default")))
#  endif
#endif

#endif
