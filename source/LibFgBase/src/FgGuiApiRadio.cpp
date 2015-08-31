//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: June 28, 2014
//

#include "stdafx.h"

#include "FgGuiApiRadio.hpp"

using namespace std;

FgGuiPtr
fgGuiRadio(FgDgn<size_t> selN,const vector<FgString> & labels)
{
    FgGuiApiRadio   ret;
    ret.horiz = false;
    ret.labels = labels;
    ret.selection = selN;
    ret.updateIdx = g_gg.addUpdateFlag(selN.idx());
    return fgsp(ret);
}

// */
