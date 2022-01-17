//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// WARNING: Any state that you want remembered in sub-windows must be saved
// (eg. makeSavedIPT) since the object may be destroyed and re-created between views.
//
// This approach didn't work so well since the dynamically created window often has
// to modify depgraph state during creation, but any dependent nodes may already
// have been updated.
//

#ifndef FGGUIAPIDYNAMIC_HPP
#define FGGUIAPIDYNAMIC_HPP

#include "FgGuiApiBase.hpp"
#include "FgStdVector.hpp"

namespace Fg {

// This function must be defined in the corresponding OS-specific implementation:
struct  GuiDynamic;
GuiImplPtr guiGetOsImpl(GuiDynamic const & guiApi);

struct  GuiDynamic : GuiBase
{
    std::function<GuiPtr(void)>   makePane;
    DfgFPtr                     updateFlag;

    virtual
    GuiImplPtr getInstance() {return guiGetOsImpl(*this); }
};

inline
GuiPtr
guiDynamic(
    const std::function<GuiPtr(void)> &   makePane,
    DfgFPtr const &                     updateFlag)
{
    GuiDynamic     d;
    d.makePane = makePane;
    d.updateFlag = updateFlag;
    return std::make_shared<GuiDynamic>(d);
}

}

#endif
