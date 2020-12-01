//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgSystemInfo.hpp"
#include "FgPlatform.hpp"

using namespace std;

namespace Fg {

bool
fg64bitOS()     // Nix is simple, OS bits = build bits:
{return is64Bit(); }

string
osDescription()
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

Ustring
getDefaultGpuDescription()
{return "GPU Description not implemented on unix"; }

Ustring
fgComputerName()
{return Ustring("Unknown"); }

}
