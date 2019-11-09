//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

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
    NPT<Ustring>        content;
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
guiText(NPT<Ustring> node,uint minWidth=0,bool rich=true);

GuiPtr
guiTextLines(NPT<Ustring> node,uint minHeight,bool wantStretchVert=false);

GuiPtr
guiText(Ustring text,uint minWidth=0);

// This function must be defined in the corresponding OS-specific implementation:
struct  GuiTextEdit;
GuiImplPtr guiGetOsImpl(GuiTextEdit const & guiApi);

struct  GuiTextEdit : GuiBase
{
    DfgFPtr                     updateFlag;
    std::function<Ustring(void)>   getInput;
    std::function<void(Ustring)>   setOutput;
    uint                            minWidth;
    bool                            wantStretch;    // Width only.

    virtual
    GuiImplPtr getInstance() {return guiGetOsImpl(*this); }
};

// String text edit box:
GuiPtr
guiTextEdit(IPT<Ustring> t,bool wantStretch=true);

// Fixed-point numerical text edit box with specified fractional digits, clips output values to bounds:
GuiPtr
guiTextEditFixed(IPT<double> valN,VecD2 bounds,uint numFraction=2);

// Floating-point numerical text edit box clips output values to bounds:
GuiPtr
guiTextEditFloat(
    IPT<double> valN,
    VecD2        bounds,                 // Bounds of the internal representation (see v2t and t2v below)
    uint            numDigits=6,
    FgFuncD2D       v2t=FgFuncD2D(),        // Convert the internal value for display
    FgFuncD2D       t2v=FgFuncD2D());       // Invert the displayed value for internal (must be inverse of above)

}

#endif
