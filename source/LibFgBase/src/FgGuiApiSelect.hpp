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
    GuiPtrs             wins;
    NPTF<size_t>        selection;

    GuiSelect(GuiPtrs const & w,NPT<size_t> const & s) : wins(w), selection(s) {}

    virtual
    GuiImplPtr getInstance() {return guiGetOsImpl(*this); }
};

inline
GuiPtr
guiSelect(IPT<size_t> select,GuiPtrs const & wins)
{
    return std::make_shared<GuiSelect>(wins,select);
}

}

#endif
