//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

// Platform-independent macros for pure C DLL interface specification.
//
// USE:
//
// * #include this file in your FgSomeDll.h file
// * Use 'FG_DLL_DECLARE' for function declarations in that file
// * #define FG_DLL_DEFINITION_FILE at the top of your FgSomeDll.cpp file
// * then #include this file and use 'FG_DLL_DEFINE' for function definitions.
// * The extra trouble is necessary for Windows which warns if the declaration visibility is
//   different from the definition visibility.
//

#ifndef FGDYNLIB_H
#define FGDYNLIB_H

#if defined(_WIN32)
  #ifdef FG_DLL_DEFINITION_FILE
    #define FG_DLL_DECLARE __declspec(dllexport)
    #define FG_DLL_DEFINE extern "C" __declspec(dllexport)
  #else
    #define FG_DLL_DECLARE __declspec(dllimport)
  #endif
#else
    #define FG_DLL_DECLARE __attribute__((visibility("default")))
    #define FG_DLL_DEFINE extern "C" __attribute__((visibility("default")))
#endif

#endif
