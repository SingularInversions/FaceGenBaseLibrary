//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Feb 24, 2007
//

#ifndef FGTHROWWINDOWS_HPP
#define FGTHROWWINDOWS_HPP

#include "FgStdLibs.hpp"
#include "FgString.hpp"

// Appends the information from Windows' "GetLastError" into the exception message:
void
fgThrowWindows(const std::string & msg,const FgString & data=FgString());

inline
void
fgThrowWindows(const std::string & msg,const std::string & data)
{fgThrowWindows(msg,FgString(data)); }

template<class T>
void
fgThrowWindows(const std::string & msg,const T & data)
{return fgThrowWindows(msg,FgString(fgToStr(data))); }

void
fgAssertWin(const char * fname,int line);

#define FGASSERTWIN(X)                                                  \
    if(X) (void) 0;                                                     \
    else fgAssertWin(__FILE__,__LINE__)

void
fgAssertWinReturnZero(const char * fname,int line,long rval);

// It's important to record the return value on failure since 'getLastError' sometimes
// returns SUCCESS after a failure - eg. CoCreateInstance with CLSID_FileOpenDialog:
#define FGASSERTWINOK(X) if(X==0) (void)0; else fgAssertWinReturnZero(__FILE__,__LINE__,X)

#endif

// */
