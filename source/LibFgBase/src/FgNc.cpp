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

std::string
fgNcShare(const std::string & os)
{
    if (os == "win")
        return fgBuildShareWin();
    else if (os == "osx")
        return fgBuildShareOsx();
    else
        FGASSERT(os == "ubuntu");
    return fgBuildShareUbuntu();
}
