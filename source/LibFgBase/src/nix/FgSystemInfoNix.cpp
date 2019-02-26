//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgSystemInfo.hpp"
#include "FgPlatform.hpp"

using namespace std;

bool
fg64bitOS()     // Nix is simple, OS bits = build bits:
{return fgIs64bit(); }

string
fgOsName()
{
    string      ret;
#ifdef __APPLE__
    ret = "MacOS";
#elif __linux__
    ret = "Linux";
#elif __FreeBSD__
    ret = "FreeBSD";
#else
    ret = "Unknown";
#endif
    return ret;
}

FgString
fgComputerName()
{return FgString("Unknown"); }
