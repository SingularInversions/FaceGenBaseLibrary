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
#include "FgGuiApiText.hpp"

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
    uint            edgePadding,
    bool            editBox)
{
    FgGuiApiSlider sldr;
    sldr.updateFlagIdx = g_gg.addUpdateFlag(valN);
    sldr.getInput = boost::bind(getInput,valN);
    sldr.setOutput = boost::bind(setOutput,valN,_1);
    sldr.label = label;
    sldr.range = range;
    sldr.tickSpacing = tickSpacing;
    sldr.tickLabels = tl;
    sldr.tockLabels = ul;
    sldr.edgePadding = edgePadding;
    FgGuiPtr        ret = fgGuiPtr(sldr);
    if (editBox)
        ret = fgGuiSplit(true,ret,fgGuiTextEditFixed(valN,range));
    return ret;
}

FgGuiSliders
fgGuiSliders(
    const FgStrings &       labels,
    FgVectD2                range,
    double                  initVal,
    double                  tickSpacing,
    const FgString &        relStore)
{
    FgGuiSliders            ret;
    vector<FgDgn<double> >  slidersN;
    for (size_t ii=0; ii<labels.size(); ++ii) {
        FgDgn<double>       val;
        if (relStore.empty())
            val = g_gg.addNode(initVal);
        else
            val = g_gg.addInput(initVal,relStore+labels[ii]);
        ret.inputInds.push_back(val);
        ret.sliders.push_back(fgGuiSlider(val,labels[ii],range,tickSpacing));
        slidersN.push_back(val);
    }
    ret.outputIdx = g_gg.collate(slidersN);
    return ret;
}

FgStrings
fgNumberedLabels(const FgString & baseLabel,size_t num)
{
    FgStrings       ret;
    ret.reserve(num);
    uint            numDigits = 1;
    size_t          tmp = num;
    while ((tmp/=10) > 9)
        ++numDigits;
    for (size_t ii=0; ii<num; ++ii)
        ret.push_back(baseLabel+fgToStringDigits(ii,numDigits));
    return ret;
}

// */
