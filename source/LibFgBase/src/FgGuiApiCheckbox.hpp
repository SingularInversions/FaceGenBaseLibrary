//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     March 19, 2011
//

#ifndef FGGUIAPICHECKBOX_HPP
#define FGGUIAPICHECKBOX_HPP

#include "FgGuiApiBase.hpp"
#include "FgDepGraph.hpp"

struct FgGuiApiCheckbox : FgGuiApi<FgGuiApiCheckbox>
{
    FgString        label;
    FgDgn<bool>     val;
    uint            updateFlagIdx;
};

inline
FgGuiPtr
fgGuiApiCheckbox(const FgString & label,FgDgn<bool> node)
{
    FgGuiApiCheckbox    cb;
    cb.label = label;
    cb.val = node;
    cb.updateFlagIdx = g_gg.addUpdateFlag(node);
    return fgsp(cb);
}

inline
FgGuiPtr
fgGuiCheckboxTr(const string & label,FgDgn<bool> node)
{return fgGuiApiCheckbox(fgTr(label),node); }

#endif
