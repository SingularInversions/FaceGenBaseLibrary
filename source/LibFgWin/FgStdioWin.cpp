//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Feb 25, 2011
//

#include "stdafx.h"
#include "FgStdio.hpp"

using namespace std;

FILE *
fgOpen(
    const FgString &    filename,
    bool                write,
    bool                binary)
{
    wstring     mode;
    if (write)
        mode += L'w';
    else
        mode += L'r';
    if (binary)
        mode += L'b';
    return
        _wfopen(
            filename.as_wstring().c_str(),
            mode.c_str());
}

// */
