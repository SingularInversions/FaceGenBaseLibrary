//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Jan 19, 2007
//

#include "stdafx.h"

using namespace std;

wstring     fgGetComputerName()
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
