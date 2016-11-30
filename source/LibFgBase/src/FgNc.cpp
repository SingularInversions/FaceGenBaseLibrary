//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: May 5, 2012
//

#include "stdafx.h"

#include "FgNc.hpp"
#include "FgDiagnostics.hpp"

using namespace std;

string
fgNcShare(const string & os)
{
    string      ret;
    if (os == "win")
        ret = "Z:\\";
    else if (os == "osx")
        ret =  "/Volumes/Zeus_share/";
    else if (os == "ubuntu")
        ret = "/mnt/share/";
    else
        FGASSERT_FALSE1(os);
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

