//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGGUIAPIGROUPBOX_HPP
#define FGGUIAPIGROUPBOX_HPP

#include "FgGuiApiBase.hpp"

namespace Fg {

// This function must be defined in the corresponding OS-specific implementation:
struct  GuiGroupbox;
GuiImplPtr guiGetOsImpl(GuiGroupbox const & guiApi);

struct
GuiGroupbox : GuiBase
{
    GuiGroupbox(Ustring const & l,GuiPtr c)
    : label(l), contents(c)
    {}

    Ustring        label;
    GuiPtr     contents;

    virtual
    GuiImplPtr getInstance() {return guiGetOsImpl(*this); }
};

inline
GuiPtr
guiGroupboxTr(const std::string & label,GuiPtr p)
{return std::make_shared<GuiGroupbox>(fgTr(label),p); }

inline
GuiPtr
guiGroupbox(Ustring const & label,GuiPtr p)
{return std::make_shared<GuiGroupbox>(label,p); }

}

#endif
