//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: April 7, 2011
//

#include "stdafx.h"

#include "FgGuiApiSlider.hpp"
#include "FgGuiApiSplit.hpp"

using namespace std;

FgGuiApiTickLabels
fgGuiApiTickLabels(
    FgVectD2        range,
    double          spacing,
    double          basePos)
{
    FGASSERT((basePos >= range[0]) && (basePos <= range[1]));
    double              pos = basePos - std::floor((basePos - range[0])/spacing) * spacing;
    FgGuiApiTickLabels  ret;
    do {
        FgGuiApiTickLabel   t;
        t.pos = pos;
        t.label = fgToString(pos);
        ret.push_back(t);
        pos += spacing;
    }
    while (pos <= range[1]);
    return ret;
}

static
double
getInput(FgDgn<double> n)
{return g_gg.getVal(n); }

static
void
setOutput(FgDgn<double> n,double v)
{g_gg.setVal(n,v); }

FgGuiPtr
fgGuiSlider(
    FgDgn<double>   valN,
    FgString        label,
    FgVectD2        range,
    double          tickSpacing,
    const FgGuiApiTickLabels & tl,
    const FgGuiApiTickLabels & ul,
    uint            edgePadding)
{
    FgGuiApiSlider ret;
    ret.updateFlagIdx = g_gg.addUpdateFlag(valN);
    ret.getInput = boost::bind(getInput,valN);
    ret.setOutput = boost::bind(setOutput,valN,_1);
    ret.label = label;
    ret.range = range;
    ret.tickSpacing = tickSpacing;
    ret.tickLabels = tl;
    ret.tockLabels = ul;
    ret.edgePadding = edgePadding;
    return fgnew<FgGuiApiSlider>(ret);
}

FgGuiSliders
fgGuiSliders(
    uint                    numSliders,
    FgString                baseLabel,
    FgVectD2                range,
    double                  initVal,
    double                  tickSpacing)
{
    FgGuiSliders            ret;
    uint                    digits = (numSliders > 10 ? 2 : 1);
    vector<FgDgn<double> >  slidersN;
    for (uint ii=0; ii<numSliders; ++ii) {
        FgDgn<double>       val = g_gg.addNode(initVal);
        ret.inputInds.push_back(val);
        ret.sliders.push_back(
            fgGuiSlider(val,
                baseLabel+fgToStringDigits(ii,digits),
                range,
                tickSpacing));
        slidersN.push_back(val);
    }
    ret.outputIdx = g_gg.collate(slidersN);
    return ret;
}

FgGuiWinVal<vector<double> >
fgGuiSliders(
    const FgString &        relStore,
    const vector<FgString> & labels,
    FgVectD2                range,
    double                  initVal,
    double                  tickSpacing)
{
    FgGuiWinVal<vector<double> >    ret;
    FGASSERT(!labels.empty());
    vector<uint>                    inputNs;
    FgGuiPtrs                       sliders;
    for (size_t ii=0; ii<labels.size(); ++ii) {
        FgDgn<double>   valN = g_gg.addInput(initVal,relStore+labels[ii]);
        inputNs.push_back(valN);
        sliders.push_back(fgGuiSlider(valN,labels[ii],range,tickSpacing));
    }
    ret.valN = g_gg.addNode(vector<double>(),relStore.as_ascii());
    g_gg.addLink(fgLinkCollate<double>,inputNs,ret.valN);
    ret.win = fgGuiSplit(false,sliders);
    return ret;
}

// */
