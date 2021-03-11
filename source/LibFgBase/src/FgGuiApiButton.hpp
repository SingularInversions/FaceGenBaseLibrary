//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGGUIAPIBUTTON_HPP
#define FGGUIAPIBUTTON_HPP

#include "FgGuiApiBase.hpp"

namespace Fg {

typedef std::function<void(void)>     FgFnVoid2Void;

// This function must be defined in the corresponding OS-specific implementation:
struct  GuiButton;
GuiImplPtr guiGetOsImpl(GuiButton const & guiApi);

struct GuiButton : GuiBase
{
    String8                label;
    FgFnVoid2Void           action;

    virtual
    GuiImplPtr getInstance() {return guiGetOsImpl(*this); }
};

GuiPtr
guiButton(String8 const & label,FgFnVoid2Void action);

GuiPtr
guiButtonTr(const std::string & label,FgFnVoid2Void action);

}

#endif
