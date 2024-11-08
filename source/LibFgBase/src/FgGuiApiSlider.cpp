//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgGuiApi.hpp"

using namespace std;
using namespace std::placeholders;


namespace Fg {

GuiTickLabels       guiTickLabels(VecD2 range,double spacing,double basePos)
{
    FGASSERT((basePos >= range[0]) && (basePos <= range[1]));
    double              pos = basePos - std::floor((basePos - range[0])/spacing) * spacing;
    GuiTickLabels       ret;
    do {
        GuiTickLabel        t;
        t.pos = pos;
        t.label = toStrPrec(pos,3);
        ret.push_back(t);
        pos += spacing;
    }
    while (pos <= range[1]);
    return ret;
}

GuiPtr              guiSlider(
    IPT<double>         valN,
    String8             label,
    VecD2               range,
    double              tickSpacing,
    GuiTickLabels const & tl,
    GuiTickLabels const & ul,
    uint                edgePadding,
    bool                editBox)
{
    GuiSlider           sldr;
    sldr.updateFlag = cUpdateFlagT(valN);
    sldr.getValFn = [valN](){return valN.val(); };
    sldr.setValFn = [valN](double v){valN.set(v); };
    sldr.label = label;
    sldr.range = range;
    sldr.tickSpacing = tickSpacing;
    sldr.tickLabels = tl;
    sldr.tockLabels = ul;
    sldr.edgePadding = edgePadding;
    GuiPtr              ret = guiPtr(sldr);
    if (editBox)
        ret = guiSplitH({ret,guiTextEditFixed(valN,range)});
    return ret;
}

Img<GuiPtr>             guiSliders(
    Svec<IPT<double> > const & valNs,
    String8s const &        labels,
    VecD2                   range,
    double                  tickSpacing,
    GuiTickLabels const &   tickLabels,
    bool                    editBox)
{
    size_t                  sz = valNs.size();
    FGASSERT(labels.size() == sz);
    Img<GuiPtr>             grid {2,sz};
    for (size_t ii=0; ii<sz; ++ii) {
        grid.xy(0,ii) = guiText(labels[ii]);
        if (ii+1==sz)
            grid.xy(1,ii) = guiSlider(valNs[ii],"",range,tickSpacing,tickLabels,{},5U,editBox);
        else
            grid.xy(1,ii) = guiSlider(valNs[ii],"",range,tickSpacing,{},{},5U,editBox);
    }
    return grid;
}

Img<GuiPtr>         guiSliders(
    IPT<Doubles> const &    valsN,
    String8s const &        labels,
    VecD2                   range,
    double                  tickSpacing,
    GuiTickLabels const &   tickLabels)
{
    size_t              S = valsN.val().size();
    FGASSERT(labels.size() == S);
    Img<GuiPtr>         ret {2,S};
    for (size_t ii=0; ii<S; ++ii) {
        GuiSlider           gs;
        gs.getValFn = [valsN,ii](){return valsN.val()[ii]; };
        gs.setValFn = [valsN,ii](double v){valsN.ref()[ii] = v; };
        gs.updateFlag = cUpdateFlagT(valsN);
        gs.range = range;
        gs.tickSpacing = tickSpacing;
        if (ii+1==S)
            gs.tickLabels = tickLabels;
        ret.xy(0,ii) = guiText(labels[ii]);
        ret.xy(1,ii) = guiPtr(gs);
    }
    return ret;
}


}

// */
