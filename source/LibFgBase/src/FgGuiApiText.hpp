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

// Possibly obsolete, places text itself by handling WM_PAINT:
struct FgGuiApiText : FgGuiApi<FgGuiApiText>
{
    FgDgn<FgString>         content;
    uint                    updateFlagIdx;
};

FgGuiPtr
fgGuiText(FgDgn<FgString> t);

struct  FgGuiApiTextEdit : FgGuiApi<FgGuiApiTextEdit>
{
    uint                            updateFlagIdx;
    boost::function<FgString(void)> getInput;
    boost::function<void(FgString)> setOutput;
};

FgGuiPtr
fgGuiTextEdit(FgDgn<FgString> t);

struct FgGuiApiTextRich : FgGuiApi<FgGuiApiTextRich>
{
    FgDgn<FgString>         content;
    uint                    updateFlagIdx;
};

FgGuiPtr
fgGuiTextRich(FgDgn<FgString> node);

FgGuiPtr
fgGuiTextRich(FgString text);

#endif
