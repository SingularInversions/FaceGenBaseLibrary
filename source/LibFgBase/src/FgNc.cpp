//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

//

#include "stdafx.h"

#include "FgNc.hpp"
#include "FgDiagnostics.hpp"

using namespace std;

namespace Fg {

string
fgNcShare(FgBuildOS os)
{
    string      ret;
    if (os == FgBuildOS::win)
        ret = "Z:\\";
    // Both are compiled on MacOS:
    else if ((os == FgBuildOS::macos) || (os == FgBuildOS::ios))
        ret =  "/Volumes/Zeus_share/";
    else if ((os == FgBuildOS::linux) || (os == FgBuildOS::android))
        ret = "/mnt/share/";
    else
        fgThrow("fgNcShare unhandled OS",int(os));
    return ret;
}

string
fgNcShare()
{
    string      ret;
#if defined _WIN32
    ret = "Z:\\";
#elif defined __APPLE__
    ret =  "/Volumes/Zeus_share/";
#else
    ret = "/mnt/share/";
#endif
    return ret;
}

}
