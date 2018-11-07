//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Nov 1, 2011

#include "stdafx.h"

#include "FgThrowWindows.hpp"
#include "FgException.hpp"
#include "FgDiagnostics.hpp"

using namespace std;

// Get the Windows text description of the last error to create an FgException, then append the
// client exception message.
void
fgThrowWindows(const string & msg,const FgString & data)
{
    DWORD           errNum = GetLastError();
    if (errNum != ERROR_SUCCESS) {
        LPVOID      lpMsgBuf;
        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            errNum,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0, NULL );
        FgString        winData(wstring(static_cast<wchar_t*>(lpMsgBuf)));
        LocalFree(lpMsgBuf);
        FgException     exc("Windows has reported an error",winData);
        exc.pushMsg(msg,data);
        throw exc;
    }
    else
        fgThrow(msg,data);
}

void
fgAssertWin(
    const char *    fname,
    int             line)
{
    fgThrowWindows("Internal program error",fgDiagString(fname,line));
}

// */
