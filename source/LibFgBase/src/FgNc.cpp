//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgNc.hpp"
#include "FgDiagnostics.hpp"

using namespace std;

namespace Fg {

string
getNcShare(BuildOS os)
{
    string      ret;
    if (os == BuildOS::win)
        ret = "Z:\\";
    // Both are compiled on MacOS:
    else if ((os == BuildOS::macos) || (os == BuildOS::ios))
        ret =  "/Volumes/Zeus_share/";
    else if ((os == BuildOS::linux) || (os == BuildOS::android))
        ret = "/mnt/share/";
    else
        fgThrow("getNcShare unhandled OS",int(os));
    return ret;
}

string
getNcShare()
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
