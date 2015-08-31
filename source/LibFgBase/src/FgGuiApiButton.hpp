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

typedef boost::function<void(void)>     FgGuiAction;

struct FgGuiApiButton : FgGuiApi<FgGuiApiButton>
{
    FgString                label;
    FgGuiAction             action;
};

FgGuiPtr
fgGuiButton(const FgString & label,FgGuiAction action);

FgGuiPtr
fgGuiButtonTr(const std::string & label,FgGuiAction action);

#endif
