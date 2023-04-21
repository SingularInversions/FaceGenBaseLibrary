//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgSystem.hpp"

using namespace std;

namespace Fg {

bool                is64bitOS()     // Nix is simple, OS bits = build bits:
{
    return is64Bit();
}

string              getOSString()
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

String8             getDefaultGpuDescription()
{
    return "GPU Description not implemented on unix";
}

String8             getComputerName()
{
    return String8("Unknown");
}

Opt<ulong>          getWinRegistryUlong(String8 const &,String8 const &)
{
    return {};
}

Opt<String8>        getWinRegistryString(String8 const &,String8 const &)
{
    return {};
}

}
