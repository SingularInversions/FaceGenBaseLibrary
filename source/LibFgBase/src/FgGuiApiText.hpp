//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Oct. 12, 2011
//

#ifndef FGGUIAPITEXT_HPP
#define FGGUIAPITEXT_HPP

#include "FgGuiApiBase.hpp"
#include "FgDepGraph.hpp"

struct FgGuiApiText : FgGuiApi<FgGuiApiText>
{
    FgDgn<FgString>         content;
    uint                    updateFlagIdx;
    uint                    minWidth;       // If text is shorter than this, use this. Otherwise ignore.

    FgGuiApiText() : minWidth(0) {}
};

FgGuiPtr
fgGuiText(FgDgn<FgString> node,uint minWidth=0);

FgGuiPtr
fgGuiText(FgString text,uint minWidth=0);

struct  FgGuiApiTextEdit : FgGuiApi<FgGuiApiTextEdit>
{
    uint                            updateFlagIdx;
    boost::function<FgString(void)> getInput;
    boost::function<void(FgString)> setOutput;
    uint                            minWidth;
    bool                            wantStretch;    // Width only.
};

FgGuiPtr
fgGuiTextEdit(FgDgn<FgString> t,bool wantStretch=true);

// Clips output values to bounds and displays only 6 digits.
FgGuiPtr
fgGuiTextEditFloat(FgDgn<double> valN,FgVect2D bounds);

#endif
