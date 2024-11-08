//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgGuiApi.hpp"
#include "FgGuiApi.hpp"

using namespace std;

namespace Fg {

Vec2UI              GuiSplit::gapTotal() const
{
    auto                fn = [](uint dim,uint gap)
    {
        uint            num = (dim > 0) ? (dim-1) : 0;
        return num * gap;
    };
    return mapCall(panes.dims(),gapSize,fn);
}

GuiPtr              guiSplit(Img<GuiPtr> const & panes)
{
    if (panes.empty())
        return guiSpacer(0,0);
    if (panes.numPixels() == 1)
        return panes.xy(0,0);
    GuiSplit        ret;
    ret.panes = panes;
    return make_shared<GuiSplit>(ret);
}

GuiPtr              guiSplitScroll(GuiPtrs const & panes,uint spacing)
{
    GuiSplitScroll     ret;
    ret.updateFlag = cUpdateFlagT(makeIPT(0));        // dummy node ensures initial one-time setup
    ret.getPanes = [panes](){return Img<GuiPtr>(1,panes.size(),panes); };
    ret.spacing = spacing;
    return make_shared<GuiSplitScroll>(ret);
}

GuiPtr              guiSplitScroll(Sfun<GuiPtrs(void)> const & getPanes)
{
    GuiSplitScroll     ret;
    ret.updateFlag = cUpdateFlagT(makeIPT(0));        // dummy node ensures initial one-time setup
    ret.getPanes = [getPanes](){GuiPtrs ptrs = getPanes(); return Img<GuiPtr>(1,ptrs.size(),ptrs); };
    return make_shared<GuiSplitScroll>(ret);
}

GuiPtr              guiSplitScroll(
    DfFPtr const &                 updateNodeIdx,      // Must be unique to this object
    Sfun<Img<GuiPtr>(void)> const & getPanes,           // Dynamic window recreation on updateFlag
    uint                            spacing)
{
    GuiSplitScroll     ret;
    ret.updateFlag = updateNodeIdx;
    ret.getPanes = getPanes;
    ret.spacing = spacing;
    return make_shared<GuiSplitScroll>(ret);
}

GuiPtr              guiSplitScroll(Img<GuiPtr> const & panes)
{
    GuiSplitScroll      gui;
    gui.updateFlag = cUpdateFlagT(makeIPT(0));
    gui.getPanes = [panes](){return panes; };
    return make_shared<GuiSplitScroll>(gui);
}

}

// */
