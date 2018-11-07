//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     July 2, 2011
//

#ifndef FGGUIAPIBUTTON_HPP
#define FGGUIAPIBUTTON_HPP

#include "FgGuiApiBase.hpp"
#include "FgDepGraph.hpp"

typedef std::function<void(void)>     FgFnVoid2Void;

struct FgGuiApiButton : FgGuiApi<FgGuiApiButton>
{
    FgString                label;
    FgFnVoid2Void           action;
};

FgGuiPtr
fgGuiButton(const FgString & label,FgFnVoid2Void action);

FgGuiPtr
fgGuiButtonTr(const std::string & label,FgFnVoid2Void action);

#endif
