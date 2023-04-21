//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgThrowWindows.hpp"
#include "FgSerial.hpp"
#include "FgDirect3D.hpp"

using namespace std;

namespace Fg {

string              getWinErrMsg(DWORD errCode,DWORD language)
{
    if (errCode == ERROR_SUCCESS)    // If last Win API func had an error it forgot to set lasterror
        return string{};
    LPVOID          lpMsgBuf;
    DWORD           numChars = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        errCode,
        language,
        (LPTSTR) &lpMsgBuf,
        0,
        NULL);
    if (numChars == 0)              // No message for this error
        return string{};
    wstring         msgRaw {static_cast<wchar_t*>(lpMsgBuf)};
    LocalFree(lpMsgBuf);
    String32       msg32 = toUtf32(toUtf8(msgRaw));
    // Windows often adds a trailing CR/LF which is not helpful to us:
    while ((msg32.back() == '\r') || (msg32.back() == '\n'))
        msg32.pop_back();
    return toUtf8(msg32);
}

// Empty if 'GetLastError()' is ERROR_SUCCESS or has no message.
string              errCodeToMsg(DWORD errCode,bool forceEnglish)
{
    DWORD           language = forceEnglish ?
        MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US) :
        MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT);
    return getWinErrMsg(errCode,language);
}

String              getWinErrMsgEnglish(DWORD errCode)
{
    DWORD           englishID = MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US);
    return getWinErrMsg(errCode,englishID);
}
String              getWinErrMsgIfNotEnglish(DWORD errCode)
{
    DWORD           englishID = MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),
                    localID = MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT);
    if (localID == englishID)
        return String{};
    else
        return getWinErrMsg(errCode,localID);
}

void                throwWindows(String const & msg,String8 const & data)
{
    DWORD               errCode = GetLastError();
    String              errStr = "0x"+toHexString(uint32(errCode)),
                        english = getWinErrMsgEnglish(errCode),
                        foreign = getWinErrMsgIfNotEnglish(errCode);
    throw FgException {{
        {msg,"",data.m_str},
        {english,foreign,errStr},
    }};
}

void                assertWindows(char const * fname,int line)
{
    throwWindows("Internal program error",toFilePosString(fname,line));
}

void                assertWinReturnZero(char const * fname,int line,long rval)
{
    throwWindows("Internal program error",toFilePosString(fname,line)+" rval: "+toHexString(scast<uint32>(rval)));
}

void                assertHResult(char const * fpath,uint lineNum,HRESULT hr)
{
    if (hr < 0)
        throwWindows("Windows HRESULT",pathToName(fpath)+":"+toStr(lineNum)+":HR="+toHexString(scast<uint32>(hr)));
}

}

// */
