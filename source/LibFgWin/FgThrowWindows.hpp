//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
//

#ifndef FGTHROWWINDOWS_HPP
#define FGTHROWWINDOWS_HPP

#include "FgSerial.hpp"
#include "FgSerial.hpp"

namespace Fg {

// get the Windows error string from the error code
String              errCodeToMsg(
    DWORD               errCode,        // returns empty if this is ERROR_SUCCESS
    bool                forceEnglish);  // if false, use the current default language
String              getWinErrMsgEnglish(DWORD errCode);
String              getWinErrMsgIfNotEnglish(DWORD errCode);

// Prepends the information from Windows' "GetLastError" into the exception context:
// If you called GetLastError earlier and did some other stuff that may invalidate it, pass on the code:
void                throwWindows(DWORD errCode,String const & msg,String8 const & data);
// Otherwise this will do it for you:
inline void         throwWindows(String const & msg,String8 const & data=String8()) {throwWindows(GetLastError(),msg,data); }
inline void         throwWindows(String const & msg,String const & data) {throwWindows(msg,String8(data)); }
template<class T>
void                throwWindows(String const & msg,T const & data) {return throwWindows(msg,String8(toStr(data))); }
void                assertWindows(char const * fname,int line);
void                assertWinReturnZero(char const * fname,int line,long rval);

// Templated custom destructor superior to function object since it's compatible with containers:
template<class T>
struct      WinRelease
{
    // unique_ptr only calls this if ptr != nullptr:
    void operator()(T* ptr) const {ptr->Release(); }
};

template<class T>
using WinPtr = std::unique_ptr<T,WinRelease<T> >;

// Reference-counted pointer:
template<class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

void                assertHResult(char const * fpath,uint lineNum,HRESULT hr);

}

#define FG_ASSERT_HR(hr) Fg::assertHResult(__FILE__,__LINE__,hr)

#define FGASSERTWIN(X)                                                  \
    if(X) (void) 0;                                                     \
    else Fg::assertWindows(__FILE__,__LINE__)

// It's important to record the return value on failure since 'getLastError' sometimes
// returns SUCCESS after a failure - eg. CoCreateInstance with CLSID_FileOpenDialog:
#define FGASSERTWINOK(X) if(X>=0) (void)0; else Fg::assertWinReturnZero(__FILE__,__LINE__,X)

#endif

// */
