//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Select between multiple windows for current display:

#ifndef FGGUIAPISELECT_HPP
#define FGGUIAPISELECT_HPP

#include "FgGuiApiBase.hpp"

namespace Fg {

// This function must be defined in the corresponding OS-specific implementation:
struct  GuiSelect;
GuiImplPtr guiGetOsImpl(GuiSelect const & guiApi);

struct  GuiSelect : GuiBase
{
    Svec<GuiPtr>        wins;
    IPT<size_t>      selection;
    DfgFPtr             updateFlag;

    virtual
    GuiImplPtr getInstance() {return guiGetOsImpl(*this); }
};

inline
GuiPtr
guiSelect(IPT<size_t> select,const Svec<GuiPtr> & wins)
{
    GuiSelect    ret;
    ret.wins = wins;
    ret.selection = select;
    ret.updateFlag = makeUpdateFlag(ret.selection);
    return std::make_shared<GuiSelect>(ret);
}

}

#endif
