//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGGUIAPITEXT_HPP
#define FGGUIAPITEXT_HPP

#include "FgGuiApiBase.hpp"

typedef std::function<double(double)>     FgFuncD2D;

namespace Fg {

// This function must be defined in the corresponding OS-specific implementation:
struct  GuiText;
GuiImplPtr guiGetOsImpl(GuiText const & guiApi);

struct GuiText : GuiBase
{
    NPT<String8>        content;
    // Usually set to true for dynamic text:
    Vec2B               wantStretch = Vec2B(false);
    // Used to specify a fixed min width for 2D layouts (eg. label - slider lists). Zero ignores.
    uint                minWidth = 0;
    // Given in lines. When you expect overflow from one line, reserve more:
    uint                minHeight = 1;
    // Set this to false to avoid bug in Win10 RichEdit that causes copy operations from this richedit
    // to hang on paste (in any other context) until this main window regains focus. Note that
    // newlines and hyptertext links are not supported with RichEdit:
    bool                rich = true;

    virtual
    GuiImplPtr getInstance() {return guiGetOsImpl(*this); }
};

// Assumes dynamic text and sets 'wantStretch' to true:
GuiPtr
guiText(NPT<String8> node,uint minWidth=0,bool rich=true);

GuiPtr
guiTextLines(NPT<String8> node,uint minHeight,bool wantStretchVert=false);

GuiPtr
guiText(String8 text,uint minWidth=0);

// This function must be defined in the corresponding OS-specific implementation:
struct  GuiTextEdit;
GuiImplPtr guiGetOsImpl(GuiTextEdit const & guiApi);

struct  GuiTextEdit : GuiBase
{
    DfgFPtr                 updateFlag;
    Sfun<String8(void)>     getInput;
    Sfun<void(String8)>     setOutput;
    uint                    minWidth;
    bool                    wantStretch;    // Width only.

    virtual
    GuiImplPtr getInstance() {return guiGetOsImpl(*this); }
};

// String text edit box:
GuiPtr
guiTextEdit(IPT<String8> t,bool wantStretch=true);

// Fixed-point numerical text edit box with given number of fractional digits and clamp values:
GuiPtr
guiTextEditFixed(
    DfgFPtr                 updateFlag,     // Must be unique to each function call
    Sfun<double(void)>      getVal,
    Sfun<void(double)>      setVal,
    VecD2                   bounds,
    uint                    numFraction);

// Fixed-point numerical text edit box with specified fractional digits, clips output values to bounds:
GuiPtr
guiTextEditFixed(IPT<double> valN,VecD2 bounds,uint numFraction=2);

// Flaoting-point numerical text edit box with given number of digits and clamp values:
GuiPtr
guiTextEditFloat(
    DfgFPtr                 updateFlag,     // Must be unique to each function call
    Sfun<double()>          getVal,
    Sfun<void(double)>      setVal,
    VecD2                   bounds,
    uint                    numDigits);

// Fixed-point numerical text edit box with specified fractional digits, clips output values to bounds:
GuiPtr
guiTextEditFloat(IPT<double> valN,VecD2 bounds,uint numDigits);

}

#endif
