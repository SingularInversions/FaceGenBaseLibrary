//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     April 23, 2011
//

#ifndef FGGUIAPIRADIO_HPP
#define FGGUIAPIRADIO_HPP

#include "FgGuiApiBase.hpp"
#include "FgDepGraph.hpp"

struct FgGuiApiRadio : FgGuiApi<FgGuiApiRadio>
{
    bool                    horiz;
    vector<FgString>        labels;     // Can be of size zero
    FgDgn<size_t>           selection;
    // Node idx for updating can be different from selection (must be exclusive):
    uint                    updateIdx;
};

FgGuiPtr
fgGuiRadio(FgDgn<size_t> selN,const vector<FgString> & labels);

#endif
