//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgThrowWindows.hpp"
#include "FgException.hpp"
#include "FgDiagnostics.hpp"
#include "FgHex.hpp"
#include "FgDirect3D.hpp"

using namespace std;

namespace Fg {

// Empty if 'GetLastError()' is ERROR_SUCCESS or has no message.
static
string
winLastErrUtf8(bool forceEnglish)
{
    DWORD           errNum = GetLastError();
    if (errNum == ERROR_SUCCESS)    // If last Win API func had an error it forgot to set lasterror
        return string{};
    DWORD           language = forceEnglish ?
        MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US) :
        MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT);
    LPVOID          lpMsgBuf;
    DWORD           numChars = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        errNum,
        language,
        (LPTSTR) &lpMsgBuf,
        0,
        NULL);
    if (numChars == 0)              // No message for this error
        return string{};
    wstring         msgRaw {static_cast<wchar_t*>(lpMsgBuf)};
    LocalFree(lpMsgBuf);
    u32string       msg32 = toUtf32(toUtf8(msgRaw));
    // Windows often adds a trailing CR/LF which is not helpful to us:
    while ((msg32.back() == '\r') || (msg32.back() == '\n'))
        msg32.pop_back();
    return toUtf8(msg32);
} 

void
throwWindows(string const & msg,Ustring const & data)
{
    FgException     exc(msg,data.m_str);
    string          winMsgLocal = winLastErrUtf8(false);
    if (!winMsgLocal.empty())
        exc.pushMsg("Windows message",winMsgLocal);
    string          winMsgEnglish = winLastErrUtf8(true);
    if (!winMsgEnglish.empty() && (winMsgEnglish != winMsgLocal))
        exc.pushMsg("Windows message",winMsgEnglish);
    throw exc;
}

void
assertWindows(const char * fname,int line)
{
    throwWindows("Internal program error",fgDiagString(fname,line));
}

void
assertWinReturnZero(const char * fname,int line,long rval)
{
    throwWindows("Internal program error",fgDiagString(fname,line)+" rval: "+toHexString(rval));
}

void
assertHResult(char const * fpath,uint lineNum,HRESULT hr)
{
    if (hr < 0)
        throwWindows("Windows HRESULT",pathToName(fpath)+":"+toStr(lineNum)+":HR="+toHexString(hr));
}

void
assertHResultD3d(char const * fpath,uint lineNum,HRESULT hr,bool supports11_1,bool supportsFlip)
{
    if (hr < 0) {
        string          v11_1 = supports11_1 ? " D3D11.1" : " D3D11.0",     // wrong way around throug Mod v3.22
                        flip = supportsFlip ? " flip" : " noflip";
        throwWindows("Windows HRESULT",pathToName(fpath)+":"+toStr(lineNum)+":HR="+toHexString(hr)+v11_1+flip);
    }
}

}

// */
