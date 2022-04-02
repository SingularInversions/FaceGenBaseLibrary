//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGGUIAPIBUTTON_HPP
#define FGGUIAPIBUTTON_HPP

#include "FgGuiApiBase.hpp"

namespace Fg {

struct      GuiButton;

// This function must be defined in the corresponding OS-specific implementation:
GuiImplPtr          guiGetOsImpl(GuiButton const & guiApi);

struct      GuiButton : GuiBase
{
    String8                label;
    Sfun<void()>           action;

    virtual GuiImplPtr getInstance() {return guiGetOsImpl(*this); }
};

GuiPtr              guiButton(String8 const & label,Sfun<void()> const & action);
GuiPtr              guiButtonTr(const std::string & label,Sfun<void()> const & action);

}

#endif
