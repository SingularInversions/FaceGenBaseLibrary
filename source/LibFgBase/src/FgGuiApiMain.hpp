//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGGUIAPIMAIN_HPP
#define FGGUIAPIMAIN_HPP

#ifdef _MSC_VER                 // Microsoft Windows:

// set Windows 7 as minimum version (required for Kinect):
#define NTDDI_VERSION   0x06010000
#define _WIN32_WINNT    0x0601      // NT legacy also required according to MSDN but not Raymond Chen

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#undef max
#undef min

// These CRT includes are absolutely required here to avoid link errors. No idea why but
// has something to do with the fact that the actual entry point of the EXE is the CRT initialization
// entry point, which eventually calls _tWinMain below:
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

//int APIENTRY _tWinMain(
//    HINSTANCE hInstance,          Base address of memory image of executable. Used to load resources.
//                                  Can also be obtained with GetModuleHandle()
//    HINSTANCE hPrevInstance,      Always 0
//    LPTSTR    lpCmdLine,          Can also be obtained with GetCommandLine()
//    int       nCmdShow)           Not of any practical use.
#define FG_GUI_MAIN(func) int APIENTRY _tWinMain(HINSTANCE,HINSTANCE,LPTSTR,int) {func(); return 0; }

#else

#define FG_GUI_MAIN(func) int main(int,char const*[]) {func(); return 0; }

#endif

#endif // FGGUIAPIMAIN_HPP
