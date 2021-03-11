//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// USE:
//
// When deciding to use string or size_t output below, choose string only if you need to actually use
// the string. There is no win if you're just switching based on the value since manual validation is
// required in either case and string comparisons are vulnerable to failure if you change the text.
//
// DESIGN:
//
// Selections are represented by string nodes because it makes save / load of input nodes more robust to
// changes and user-friendly for customization. However it does require the strings to be unique.
//

#ifndef FGGUIAPIRADIO_HPP
#define FGGUIAPIRADIO_HPP

#include "FgGuiApiBase.hpp"
#include "FgDataflow.hpp"

namespace Fg {

// This function must be defined in the corresponding OS-specific implementation:
struct  GuiRadio;
GuiImplPtr guiGetOsImpl(GuiRadio const & guiApi);

struct GuiRadio : GuiBase
{
    bool                horiz;
    String8s            labels;
    IPT<size_t>         selection;

    virtual
    GuiImplPtr getInstance() {return guiGetOsImpl(*this); }
};

// If desired output is the index:
GuiPtr
guiRadio(String8s const & labels,IPT<size_t> idxN);

// If desired output is the label:
GuiVal<String8>
guiRadioLabel(String8s const & labels,IPT<size_t> idxN);

}

#endif
