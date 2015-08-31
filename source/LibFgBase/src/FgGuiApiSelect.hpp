//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Oct 14, 2011
//
// Select between multiple windows for current display:

#ifndef FGGUIAPISELECT_HPP
#define FGGUIAPISELECT_HPP

#include "FgGuiApiBase.hpp"

struct  FgGuiApiSelect : FgGuiApi<FgGuiApiSelect>
{
    vector<FgGuiPtr>        wins;
    FgDgn<uint>             selection;
    uint                    updateNodeIdx;
};

inline
FgGuiPtr
fgGuiSelect(FgDgn<uint> select,const vector<FgGuiPtr> & wins)
{
    FgGuiApiSelect  ret;
    ret.wins = wins;
    ret.selection = select;
    ret.updateNodeIdx = g_gg.addUpdateFlag(ret.selection);
    return fgsp(ret);
}

#endif
