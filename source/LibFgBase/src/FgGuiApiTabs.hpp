//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGGUIAPITABS_HPP
#define FGGUIAPITABS_HPP

#include "FgGuiApiBase.hpp"
#include "FgStdString.hpp"
#include "FgStdVector.hpp"

namespace Fg {

struct  GuiTabDef
{
    String8        label;
    GuiPtr          win;
    uint            padLeft;        // pixels ...
    uint            padRight;
    uint            padTop;
    uint            padBottom;

    GuiTabDef()
    : padLeft(1), padRight(1), padTop(1), padBottom(1)
    {}

    GuiTabDef(String8 const & l,GuiPtr w)
    : label(l), win(w), padLeft(1), padRight(1), padTop(1), padBottom(1)
    {}

    GuiTabDef(String8 const & l,bool spacer,GuiPtr w)
    :   label(l), win(w),
        padLeft(spacer ? 5 : 1), padRight(spacer ? 5 : 1),
        padTop(spacer ? 10 : 1), padBottom(1)
    {}
};
typedef Svec<GuiTabDef>  GuiTabDefs;

inline
GuiTabDef
guiTab(String const & l,GuiPtr w)
{return GuiTabDef(fgTr(l),w); }

inline
GuiTabDef
guiTab(String const & label,bool spacer,GuiPtr w)
{return GuiTabDef(fgTr(label),spacer,w); }

// This function must be defined in the corresponding OS-specific implementation:
struct  GuiTabs;
GuiImplPtr guiGetOsImpl(GuiTabs const & guiApi);

struct  GuiTabs : GuiBase
{
    Svec<GuiTabDef>        tabs;

    virtual
    GuiImplPtr getInstance() {return guiGetOsImpl(*this); }
};

inline
GuiPtr
guiTabs(const Svec<GuiTabDef> & tabs)
{
    FGASSERT(!tabs.empty());
    GuiTabs        gat;
    gat.tabs = tabs;
    return std::make_shared<GuiTabs>(gat);
}

}

#endif
