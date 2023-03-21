//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGGUIAPICHECKBOX_HPP
#define FGGUIAPICHECKBOX_HPP

#include "FgGuiApiBase.hpp"

namespace Fg {

// This function must be defined in the corresponding OS-specific implementation:
struct      GuiCheckbox;
GuiImplPtr          guiGetOsImpl(GuiCheckbox const & guiApi);

struct      GuiCheckbox : GuiBase
{
    String8             label;
    // Will be called to get current status for display updates (must return true if box selected):
    Sfun<bool()>        getFn;
    // Will be called when user clicks on checkbox:
    Sfun<void()>        clickFn;

    GuiCheckbox() {}
    GuiCheckbox(String8 const & l,Sfun<bool()> const & g,Sfun<void()> const & c)
        : label(l), getFn(g), clickFn(c)
    {}

    virtual GuiImplPtr getInstance() {return guiGetOsImpl(*this); }
};

GuiPtr              guiCheckbox(String8 const & label,IPT<bool> const & valInp);
GuiPtr              guiCheckboxes(String8s const & labels,Svec<IPT<bool>> const & selNs);

}

#endif
