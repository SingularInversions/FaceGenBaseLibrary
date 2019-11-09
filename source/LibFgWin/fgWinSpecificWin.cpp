//
// Copyright (C) Singular Inversions Inc. 2018
//
// Authors:     Andrew Beatty
// Created:     18.01.30
//

#include "stdafx.h"

#include "FgWinSpecific.hpp"
#include "FgCommand.hpp"
#include "FgSyntax.hpp"
#include "FgThrowWindows.hpp"

#pragma comment (lib, "AdvApi32.lib")   // Link to DLL with registry API

using namespace std;

namespace Fg {

// Returns no value if given dir/name not found:
Opt<ulong>
winRegistryLookupUlong(const Ustring & dir,const Ustring & name)
{
    Opt<ulong>        ret;
    HKEY                hKey;
    wstring             pathW = dir.as_wstring();
    LONG                result = RegOpenKeyExW(HKEY_CURRENT_USER,pathW.c_str(),0,KEY_READ,&hKey);
    if (result == ERROR_FILE_NOT_FOUND)
        return ret;
    if (result != ERROR_SUCCESS)
        throwWindows("Error opening registry directory",dir);
    DWORD               dwBufferSize(sizeof(DWORD));
    DWORD               nResult(0);
    wstring             nameW = name.as_wstring();
    result = RegQueryValueExW(hKey,nameW.c_str(),NULL,NULL,reinterpret_cast<LPBYTE>(&nResult),&dwBufferSize);
    RegCloseKey(hKey);
    if (result == ERROR_FILE_NOT_FOUND)
        return ret;
    if (result != ERROR_SUCCESS)
        throwWindows("Error reading registry value",name);
    ret = nResult;
    return ret;
}

// Returns no value if given dir/name not found:
Opt<Ustring>
fgWinRegistryLookupString(const Ustring & dir,const Ustring & name)
{
    Opt<Ustring>     ret;
    HKEY                hKey;
    wstring             pathW = dir.as_wstring();
    LONG                result = RegOpenKeyExW(HKEY_CURRENT_USER,pathW.c_str(),0,KEY_READ,&hKey);
    if (result == ERROR_FILE_NOT_FOUND)
        return ret;
    if (result != ERROR_SUCCESS)
        throwWindows("Error opening registry directory",dir);

    WCHAR               szBuffer[2048];
    DWORD               dwBufferSize = sizeof(szBuffer);
    wstring             nameW = name.as_wstring();
    result = RegQueryValueExW(hKey,nameW.c_str(),NULL,NULL,reinterpret_cast<LPBYTE>(&szBuffer),&dwBufferSize);
    RegCloseKey(hKey);
    if (result == ERROR_FILE_NOT_FOUND)
        return ret;
    if (result != ERROR_SUCCESS)
        throwWindows("Error reading registry value",name);
    ret = Ustring(szBuffer);
    return ret;
}

}
