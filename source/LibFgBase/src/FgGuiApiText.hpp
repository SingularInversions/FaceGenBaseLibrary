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
    // Usually set to true for dynamic text:
    FgVect2B                wantStretch = FgVect2B(false);
    // Used to specify a fixed min width for 2D layouts (eg. label - slider lists). Zero ignores.
    uint                    minWidth = 0;
    // Given in lines. When you expect overflow from one line, reserve more:
    uint                    minHeight = 1;
    // Set this to false to avoid bug in Win10 RichEdit that causes copy operations from this richedit
    // to hang on paste (in any other context) until this main window regains focus. Note that
    // newlines and hyptertext links are not supported with RichEdit:
    bool                    rich = true;

    FgGuiApiText() {}
};

// Assumes dynamic text and sets 'wantStretch' to true:
FgGuiPtr
fgGuiText(FgDgn<FgString> node,uint minWidth=0,bool rich=true);

FgGuiPtr
fgGuiTextLines(FgDgn<FgString> node,uint minHeight,bool wantStretchVert=false);

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

// String text edit box:
FgGuiPtr
fgGuiTextEdit(FgDgn<FgString> t,bool wantStretch=true);

// Fixed-point numerical text edit box with specified fractional digits, clips output values to bounds:
FgGuiPtr
fgGuiTextEditFixed(FgDgn<double> valN,FgVectD2 bounds,uint numFraction=2);

typedef boost::function<double(double)>     FgFuncD2D;

// Floating-point numerical text edit box clips output values to bounds:
FgGuiPtr
fgGuiTextEditFloat(FgDgn<double> valN,
    FgVectD2        bounds,                 // Bounds of the internal representation (see v2t and t2v below)
    uint            numDigits=6,
    FgFuncD2D       v2t=FgFuncD2D(),        // Convert the internal value for display
    FgFuncD2D       t2v=FgFuncD2D());       // Invert the displayed value for internal (must be inverse of above)

#endif
