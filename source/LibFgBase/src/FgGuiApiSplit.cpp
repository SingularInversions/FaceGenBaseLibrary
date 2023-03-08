//
// Copyright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgGuiApiSplit.hpp"
#include "FgGuiApiSpacer.hpp"

using namespace std;

namespace Fg {

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
    ret.updateFlag = makeUpdateFlag(makeIPT(0));        // dummy node ensures initial one-time setup
    ret.getPanes = [panes](){return panes; };
    ret.spacing = spacing;
    return make_shared<GuiSplitScroll>(ret);
}

GuiPtr              guiSplitScroll(Sfun<GuiPtrs(void)> const & getPanes)
{
    GuiSplitScroll     ret;
    ret.updateFlag = makeUpdateFlag(makeIPT(0));        // dummy node ensures initial one-time setup
    ret.getPanes = getPanes;
    return make_shared<GuiSplitScroll>(ret);
}

GuiPtr              guiSplitScroll(
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

GuiPtr              guiSplitScroll(Img<GuiPtr> const & panes)
{
    // TODO: split scroll needs to work with IMG as it's native format:
    GuiPtrs             merged;
    for (size_t yy=0; yy<panes.height(); ++yy) {
        GuiPtrs             row;
        for (size_t xx=0; xx<panes.width(); ++xx)
            row.push_back(panes.xy(xx,yy));
        merged.push_back(guiSplitH(row));
    }
    return guiSplitScroll(merged);
}

}

// */
