//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgGuiApiSplit.hpp"

using namespace std;


namespace Fg {

static
GuiPtrs
getPanes(GuiPtrs ps)
{return ps; }

GuiPtr
guiSplit(bool horiz,GuiPtrs const & panes)
{
    FGASSERT(!panes.empty());
    GuiSplit        ret;
    uint            sz = int(panes.size());
    if (horiz)
        ret.panes = Img<GuiPtr>(Vec2UI{sz,1U},panes);
    else
        ret.panes = Img<GuiPtr>(Vec2UI{1U,sz},panes);
    return make_shared<GuiSplit>(ret);
}

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
    ret.updateFlag = makeUpdateFlag(makeIPT(0));  // dummy node ensures initial one-time setup
    ret.getPanes = std::bind(getPanes,panes);
    ret.spacing = spacing;
    return make_shared<GuiSplitScroll>(ret);
}

GuiPtr
guiSplitScroll(std::function<GuiPtrs(void)> getPanes)
{
    GuiSplitScroll     ret;
    ret.updateFlag = makeUpdateFlag(makeIPT(0));  // dummy node ensures initial one-time setup
    ret.getPanes = getPanes;
    return make_shared<GuiSplitScroll>(ret);
}

GuiPtr
guiSplitScroll(
    const DfgFPtr &                     updateNodeIdx,  // Must be unique to this object
    std::function<GuiPtrs(void)>        getPanes,       // Dynamic window recreation on updateFlag
    uint                                spacing)
{
    GuiSplitScroll     ret;
    ret.updateFlag = updateNodeIdx;
    ret.getPanes = getPanes;
    ret.spacing = spacing;
    return make_shared<GuiSplitScroll>(ret);
}

}

// */
