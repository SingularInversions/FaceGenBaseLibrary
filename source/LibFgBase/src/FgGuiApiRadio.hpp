//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     April 23, 2011
//
// USE:
//
// When deciding to use string or size_t output below, choose string only if you need to actually use
// the string. There is no win if you're just switching based on the value since manual validation is
// required in either case and string comparisons are vulnerable to failure if you change the text.
//
// DESIGN:
//
// Selections are represented by string nodes because it makes save / load of input nodes more robust to
// changes and user-friendly for customization. However it does require the strings to be unique.
//

#ifndef FGGUIAPIRADIO_HPP
#define FGGUIAPIRADIO_HPP

#include "FgGuiApiBase.hpp"
#include "FgDepGraph.hpp"

struct FgGuiApiRadio : FgGuiApi<FgGuiApiRadio>
{
    bool                    horiz;
    FgStrings        vals;       // The output values for each selection. Must be non-empty.
    FgStrings        labels;     // Must be same size as 'val'. Each must be unique.
    FgDgn<FgString>         selection;
    // Node idx for updating can be different from selection (must be exclusive):
    uint                    updateIdx;
};

FgGuiPtr
fgGuiRadio(FgDgn<FgString> selN,const FgStrings & vals,const FgStrings & labels);

// When 'vals' and 'labels' are the same:
inline
FgGuiPtr
fgGuiRadio(FgDgn<FgString> selN,const FgStrings & vals)
{return fgGuiRadio(selN,vals,vals); }

struct  FgGuiRadio
{
    FgDgn<FgString>     strN;       // Currently selected string. Input node.
    FgDgn<size_t>       idxN;       // Currently selected index. Derived from above.
    FgGuiPtr            win;
};

// When you just need indices into 'labels'.
// 'labels' must be unique.
// The selection will be a stored input if 'store' is non-empty.
// If 'defVal' is empty, the first element in 'vals' will be the default.
FgGuiRadio
fgGuiRadio(const FgStrings & vals,const FgStrings & labels,FgString store="",FgString defVal="");

// When 'vals' and 'labels' are the same:
inline
FgGuiRadio
fgGuiRadio(const FgStrings & vals,const FgString & store="")
{return fgGuiRadio(vals,vals,store); }

#endif
