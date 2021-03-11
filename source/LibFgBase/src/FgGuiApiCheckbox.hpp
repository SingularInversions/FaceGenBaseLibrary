//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGGUIAPICHECKBOX_HPP
#define FGGUIAPICHECKBOX_HPP

#include "FgGuiApiBase.hpp"

namespace Fg {

// This function must be defined in the corresponding OS-specific implementation:
struct  GuiCheckbox;
GuiImplPtr guiGetOsImpl(GuiCheckbox const & guiApi);

struct GuiCheckbox : GuiBase
{
    String8            label;
    IPT<bool>    val;
    DfgFPtr         updateFlag;

    virtual
    GuiImplPtr getInstance() {return guiGetOsImpl(*this); }
};

GuiPtr
guiCheckbox(String8 const & label,const IPT<bool> & valInp);

GuiPtr
guiCheckboxes(String8s const & labels,Svec<IPT<bool> > const & selNs);

GuiVal<Bools >
guiCheckboxes(String8s const & labels,Bools const & defaults);

}

#endif
