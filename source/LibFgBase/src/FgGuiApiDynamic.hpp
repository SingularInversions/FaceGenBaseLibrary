//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Oct 26, 2015
//

#ifndef FGGUIAPIDYNAMIC_HPP
#define FGGUIAPIDYNAMIC_HPP

#include "FgGuiApiBase.hpp"
#include "FgStdVector.hpp"

// Shim to use a dynmically updated window instead of a static one:
struct  FgGuiApiDynamic : FgGuiApi<FgGuiApiDynamic>
{
    boost::function<FgGuiPtr(void)> getWin;
    // Dynamic windows must have static size/stretch values since:
    // 1. The parent of a dynamic shim doesn't know to resize.
    // 2. Resizing dynamically can propogate arbitrarily up.
    FgVect2UI                       minSize;
    FgVect2B                        wantStretch;
    uint                            updateFlagIdx;
};

inline
FgGuiPtr
fgGuiDynamic(boost::function<FgGuiPtr(void)> getWin,FgVect2UI minSize,FgVect2B wantStretch,uint updateIdx)
{
    FgGuiApiDynamic     d;
    d.getWin = getWin;
    d.minSize = minSize;
    d.wantStretch = wantStretch;
    d.updateFlagIdx = g_gg.addUpdateFlag(updateIdx);
    return fgsp(d);
}

#endif
