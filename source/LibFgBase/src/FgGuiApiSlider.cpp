//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgGuiApiSlider.hpp"
#include "FgGuiApiSplit.hpp"
#include "FgGuiApiText.hpp"

using namespace std;
using namespace std::placeholders;


namespace Fg {

GuiTickLabels
guiTickLabels(
    VecD2        range,
    double          spacing,
    double          basePos)
{
    FGASSERT((basePos >= range[0]) && (basePos <= range[1]));
    double              pos = basePos - std::floor((basePos - range[0])/spacing) * spacing;
    GuiTickLabels  ret;
    do {
        GuiTickLabel   t;
        t.pos = pos;
        t.label = toStr(pos);
        ret.push_back(t);
        pos += spacing;
    }
    while (pos <= range[1]);
    return ret;
}

Ustrings
numberedLabels(Ustring const & baseLabel,size_t num)
{
    Ustrings       ret;
    ret.reserve(num);
    uint            numDigits = 1;
    size_t          tmp = num;
    while ((tmp/=10) > 9)
        ++numDigits;
    for (size_t ii=0; ii<num; ++ii)
        ret.push_back(baseLabel+toStrDigits(ii,numDigits));
    return ret;
}

GuiPtr
guiSlider(
    IPT<double>     valN,
    Ustring        label,
    VecD2        range,
    double          tickSpacing,
    const GuiTickLabels & tl,
    const GuiTickLabels & ul,
    uint            edgePadding,
    bool            editBox)
{
    GuiSlider sldr;
    sldr.updateFlag = makeUpdateFlag(valN);
    sldr.getInput = [valN](){return valN.val(); };
    sldr.setOutput = [valN](double v){valN.set(v); };
    sldr.label = label;
    sldr.range = range;
    sldr.tickSpacing = tickSpacing;
    sldr.tickLabels = tl;
    sldr.tockLabels = ul;
    sldr.edgePadding = edgePadding;
    GuiPtr        ret = guiMakePtr(sldr);
    if (editBox)
        ret = guiSplit(true,ret,guiTextEditFixed(valN,range));
    return ret;
}

Arr<GuiPtr,3>
guiSliders(
    IPT<Vec3F>    valN,
    const array<Ustring,3> & labels,
    VecD2                range,
    double                  tickSpacing)
{
    Arr<GuiPtr,3>  ret;
    for (size_t ss=0; ss<3; ++ss) {
        GuiSlider        s;
        s.updateFlag = makeUpdateFlag(valN);
        s.getInput = [valN,ss](){return valN.cref()[ss]; };
        s.setOutput = [valN,ss](double v){valN.ref()[ss] = v; };
        s.label = labels[ss];
        s.range = range;
        s.tickSpacing = tickSpacing;
        ret[ss] = guiMakePtr(s);
    }
    return ret;
}

GuiSliders
guiSliders(
    Ustrings const &       labels,
    VecD2                range,
    double                  initVal,
    double                  tickSpacing,
    Ustring const &        relStore)
{
    GuiSliders           ret;
    vector<IPT<double> >    slidersN;
    for (size_t ii=0; ii<labels.size(); ++ii) {
        IPT<double>       val;
        if (relStore.empty())
            val = makeIPT(initVal);
        else
            val = makeSavedIPT(initVal,relStore+labels[ii]);
        ret.valNs.push_back(val);
        ret.sliders.push_back(guiSlider(val,labels[ii],range,tickSpacing));
        slidersN.push_back(val);
    }
    ret.valsN = linkCollate(slidersN);
    return ret;
}

}

// */
