//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
//

#ifndef FGTHROWWINDOWS_HPP
#define FGTHROWWINDOWS_HPP

#include "FgStdLibs.hpp"
#include "FgString.hpp"

namespace Fg {

// Appends the information from Windows' "GetLastError" into the exception message:
void
throwWindows(const std::string & msg,Ustring const & data=Ustring());

inline
void
throwWindows(const std::string & msg,const std::string & data)
{throwWindows(msg,Ustring(data)); }

template<class T>
void
throwWindows(const std::string & msg,const T & data)
{return throwWindows(msg,Ustring(toString(data))); }

void
assertWindows(const char * fname,int line);

void
assertWinReturnZero(const char * fname,int line,long rval);

// Templated custom destructor superior to function object since it's compatible with containers:
template<class T>
struct      Release
{
    // unique_ptr only calls this if ptr != nullptr:
    void operator()(T* ptr) const {ptr->Release(); }
};

}

#define FGASSERTWIN(X)                                                  \
    if(X) (void) 0;                                                     \
    else Fg::assertWindows(__FILE__,__LINE__)

// It's important to record the return value on failure since 'getLastError' sometimes
// returns SUCCESS after a failure - eg. CoCreateInstance with CLSID_FileOpenDialog:
#define FGASSERTWINOK(X) if(X>=0) (void)0; else Fg::assertWinReturnZero(__FILE__,__LINE__,X)

#endif

// */
