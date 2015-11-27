//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Jan 19, 2007
//

#include "stdafx.h"

#include "FgString.hpp"

using namespace std;

FgString
fgSystemInfo()
{
    OSVERSIONINFOEX     osvi;
    ZeroMemory(&osvi,sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx(LPOSVERSIONINFOW(&osvi));
    FgString            verStr;
    if (osvi.dwMajorVersion == 6) {
        if (osvi.dwMinorVersion == 0) {
            if (osvi.wProductType == VER_NT_WORKSTATION)
                verStr = "Vista";
            else
                verStr = "Server 2008";
        }
        if (osvi.dwMinorVersion == 1) {
            if (osvi.wProductType == VER_NT_WORKSTATION)
                verStr = "7";
            else
                verStr = "Server 2008 R2";
        }
        if (osvi.dwMinorVersion == 2) {
            if (osvi.wProductType == VER_NT_WORKSTATION)
                verStr = "8";
            else
                verStr = "Server 2012";
        }
        if (osvi.dwMinorVersion == 3) {
            if (osvi.wProductType == VER_NT_WORKSTATION)
                verStr = "8.1";
            else
                verStr = "Server 2012 R2";
        }
    }
    else if (osvi.dwMajorVersion == 10) {
        if (osvi.dwMinorVersion == 0) {
            if (osvi.wProductType == VER_NT_WORKSTATION)
                verStr = "10";
            else
                verStr = "Server 2016 TP";
        }
    }
#ifdef FG_64
    verStr += " 64bit ";
#else
    BOOL        is64;
    IsWow64Process(GetCurrentProcess(),&is64);
    if (is64)
        verStr += " 64bit ";
    else
        verStr += " 32bit ";
#endif
    verStr += FgString(wstring(osvi.szCSDVersion));
    if (verStr.empty())
        return "Windows Unknown Version";
    else
        return "Windows " + verStr;
}

FgString
fgComputerName()
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

// */
