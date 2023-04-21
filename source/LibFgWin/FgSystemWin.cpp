//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
//

#include "stdafx.h"

#include "FgSystem.hpp"
#include "FgThrowWindows.hpp"

#pragma comment (lib, "AdvApi32.lib")   // Link to DLL with registry API

using namespace std;

namespace Fg {

bool                is64bitOS()
{
#ifdef FG_64
    return true;
#else
    BOOL        is64;
    IsWow64Process(GetCurrentProcess(),&is64);
    return bool(is64);
#endif
}

string              getOSString()
{
    // Note that GetVersionEx, VerifyVersionInfo and IsWindows10OrGreater will all report Windows 8,
    // Unless your program manifest *requires* a version of Windows higher than that, in which case
    // that's what it will report. Because of course MS is so much smarter than developers that
    // they can't be trusted to use version info correctly.
    // In order to bypass this sh*t you have to call 'RtlGetVersion' which means accessing the
    // drivers API which isn't worth the effort, so for now we'll just always get back Win 8 or lower:
    string              ret = "Windows ";
    if (IsWindows10OrGreater())
        ret += "10";
    else if (IsWindows8Point1OrGreater())
        ret += "8.1";
    else if (IsWindows8OrGreater())
        ret += "8";
    else if (IsWindows7SP1OrGreater())
        ret += "7 SP1";
    else if (IsWindows7OrGreater())
        ret += "7";
    else
        ret += "Vista/XP";
    if (IsWindowsServer())
        ret += " Server";
    if (is64bitOS())
        ret += " 64bit ";
    else
        ret += " 32bit ";
    return ret;
}

String8             getComputerName()
{
    TCHAR       buff[512];
    DWORD       len = 512;
    if (GetComputerNameEx(ComputerNamePhysicalNetBIOS,buff,&len))
        return wstring(buff,buff+len);
    if (GetComputerNameEx(ComputerNamePhysicalDnsHostname,buff,&len))
        return wstring(buff,buff+len);
    else
        return wstring(L"Unknown");
}

Opt<ulong>          getWinRegistryUlong(String8 const & dir,String8 const & name)
{
    HKEY                hKey;
    wstring             pathW = dir.as_wstring();
    LONG                result = RegOpenKeyExW(HKEY_CURRENT_USER,pathW.c_str(),0,KEY_READ,&hKey);
    if (result == ERROR_FILE_NOT_FOUND)
        return {};
    if (result != ERROR_SUCCESS)
        throwWindows("Error opening registry directory",dir);
    DWORD               dwBufferSize(sizeof(DWORD));
    DWORD               nResult(0);
    wstring             nameW = name.as_wstring();
    result = RegQueryValueExW(hKey,nameW.c_str(),NULL,NULL,reinterpret_cast<LPBYTE>(&nResult),&dwBufferSize);
    RegCloseKey(hKey);
    if (result == ERROR_FILE_NOT_FOUND)
        return {};
    if (result != ERROR_SUCCESS)
        throwWindows("Error reading registry value",name);
    return nResult;
}

Opt<String8>        getWinRegistryString(String8 const & dir,String8 const & name)
{
    HKEY                hKey;
    wstring             pathW = dir.as_wstring();
    LONG                result = RegOpenKeyExW(HKEY_CURRENT_USER,pathW.c_str(),0,KEY_READ,&hKey);
    if (result == ERROR_FILE_NOT_FOUND)
        return {};
    if (result != ERROR_SUCCESS)
        throwWindows("Error opening registry directory",dir);

    WCHAR               szBuffer[2048];
    DWORD               dwBufferSize = sizeof(szBuffer);
    wstring             nameW = name.as_wstring();
    result = RegQueryValueExW(hKey,nameW.c_str(),NULL,NULL,reinterpret_cast<LPBYTE>(&szBuffer),&dwBufferSize);
    RegCloseKey(hKey);
    if (result == ERROR_FILE_NOT_FOUND)
        return {};
    if (result != ERROR_SUCCESS)
        throwWindows("Error reading registry value",name);
    return String8(szBuffer);
}

}

// */
