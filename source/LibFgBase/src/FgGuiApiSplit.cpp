//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgGuiApiSplit.hpp"

using namespace std;

namespace Fg {

GuiPtr
guiSplit(Img<GuiPtr> const & panes)
{
    FGASSERT(!panes.empty());
    if (panes.numPixels() == 1)
        return panes[0];
    GuiSplit        ret;
    ret.panes = panes;
    return make_shared<GuiSplit>(ret);
}

GuiPtr
guiSplitScroll(GuiPtrs const & panes,uint spacing)
{
    GuiSplitScroll     ret;
    ret.updateFlag = makeUpdateFlag(makeIPT(0));        // dummy node ensures initial one-time setup
    ret.getPanes = [panes](){return panes; };
    ret.spacing = spacing;
    return make_shared<GuiSplitScroll>(ret);
}

GuiPtr
guiSplitScroll(Sfun<GuiPtrs(void)> const & getPanes)
{
    GuiSplitScroll     ret;
    ret.updateFlag = makeUpdateFlag(makeIPT(0));        // dummy node ensures initial one-time setup
    ret.getPanes = getPanes;
    return make_shared<GuiSplitScroll>(ret);
}

GuiPtr
guiSplitScroll(
    DfgFPtr const &                 updateNodeIdx,      // Must be unique to this object
    Sfun<GuiPtrs(void)> const &     getPanes,           // Dynamic window recreation on updateFlag
    uint                            spacing)
{
    GuiSplitScroll     ret;
    ret.updateFlag = updateNodeIdx;
    ret.getPanes = getPanes;
    ret.spacing = spacing;
    return make_shared<GuiSplitScroll>(ret);
}

}

// */
