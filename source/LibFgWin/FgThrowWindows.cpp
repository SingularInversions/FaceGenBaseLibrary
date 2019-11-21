//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgThrowWindows.hpp"
#include "FgException.hpp"
#include "FgDiagnostics.hpp"

using namespace std;

namespace Fg {

// Get the Windows text description of the last error to create an FgException, then append the
// client exception message.
void
throwWindows(string const & msg,Ustring const & data)
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
        Ustring        winData(wstring(static_cast<wchar_t*>(lpMsgBuf)));
        LocalFree(lpMsgBuf);
        FgException     exc("Windows has reported an error",winData.m_str);
        exc.pushMsg(msg,data.m_str);
        throw exc;
    }
    else
        fgThrow(msg,data);
}

void
assertWindows(
    const char *    fname,
    int             line)
{
    throwWindows("Internal program error",fgDiagString(fname,line));
}

void
assertWinReturnZero(const char * fname,int line,long rval)
{
    throwWindows("Internal program error",fgDiagString(fname,line)+" rval: "+toString(rval));
}

}

// */
