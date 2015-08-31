//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: June 28, 2014
//

#include "stdafx.h"

#include "FgGuiApiButton.hpp"

using namespace std;

FgGuiPtr
fgGuiButton(const FgString & label,FgGuiAction action)
{
    FgGuiApiButton      b;
    b.label = label;
    b.action = action;
    return fgsp(b);
}

FgGuiPtr
fgGuiButtonTr(const string & label,FgGuiAction action)
{
    FgGuiApiButton      b;
    b.label = label;
    b.action = action;
    return fgsp(b);
}

// */
