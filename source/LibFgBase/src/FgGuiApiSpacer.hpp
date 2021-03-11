//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Layout space

#ifndef FGGUIAPISPACER_HPP
#define FGGUIAPISPACER_HPP

#include "FgGuiApiBase.hpp"

namespace Fg {

// This function must be defined in the corresponding OS-specific implementation:
struct  GuiSpacer;
GuiImplPtr guiGetOsImpl(GuiSpacer const & guiApi);

struct  GuiSpacer : GuiBase
{
    Vec2UI       size;       // One dim can be zero

    virtual
    GuiImplPtr getInstance() {return guiGetOsImpl(*this); }
};

inline
GuiPtr
guiSpacer(uint wid,uint hgt)
{
    GuiSpacer  ret;
    ret.size = Vec2UI(wid,hgt);
    return std::make_shared<GuiSpacer>(ret);
}

}

#endif
