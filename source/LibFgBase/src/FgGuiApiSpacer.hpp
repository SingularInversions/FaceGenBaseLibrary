//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Oct 06, 2015
//
// Layout space

#ifndef FGGUIAPISPACER_HPP
#define FGGUIAPISPACEF_HPP

#include "FgGuiApiBase.hpp"

struct  FgGuiApiSpacer : FgGuiApi<FgGuiApiSpacer>
{
    FgVect2UI       size;       // One dim can be zero
};

inline
FgGuiPtr
fgGuiSpacer(uint wid,uint hgt)
{
    FgGuiApiSpacer  ret;
    ret.size = FgVect2UI(wid,hgt);
    return fgsp(ret);
}

#endif
