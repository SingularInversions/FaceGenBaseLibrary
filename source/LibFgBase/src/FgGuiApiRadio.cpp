//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: June 28, 2014
//

#include "stdafx.h"

#include "FgGuiApiRadio.hpp"

using namespace std;

FgGuiPtr
fgGuiRadio(FgDgn<FgString> selN,const FgStrings & vals,const FgStrings & labels)
{
    FgGuiApiRadio       ret;
    FGASSERT(!vals.empty());
    FGASSERT(labels.size() == vals.size());
    if (!fgContains(vals,g_gg.getVal(selN)))    // eg. Input stored value no longer valid
        g_gg.setVal(selN,vals[0]);
    ret.horiz = false;
    ret.vals = vals;
    ret.labels = labels;
    ret.selection = selN;
    ret.updateIdx = g_gg.addUpdateFlag(selN.idx());
    return fgsp(ret);
}

static
void
valToIdx(const FgStrings &       vals,
    const vector<const FgVariant*> &    inputs, 
    const vector<FgVariant*> &          outputs)
{
    FGASSERT(!vals.empty());
    FGASSERT(inputs.size() == 1);
    FGASSERT(outputs.size() == 1);
    const FgString &    in = inputs[0]->valueRef();
    size_t              idx = fgFindFirstIdx(vals,in);
    if (idx == vals.size())
        idx = 0;
    outputs[0]->set(idx);
}

FgGuiRadio
fgGuiRadio(const FgStrings & vals,const FgStrings & labels,FgString store,FgString defVal)
{
    FgGuiRadio          ret;
    FGASSERT(!vals.empty());
    FGASSERT(vals.size() == labels.size());
    if (defVal.empty())
        defVal = vals[0];
    else
        FGASSERT(fgContains(vals,defVal));
    if (store.empty())
        ret.strN = g_gg.addNode(defVal);
    else
        ret.strN = g_gg.addInput(defVal,store);
    ret.idxN = g_gg.addNode(size_t(0));
    g_gg.addLink(boost::bind(valToIdx,vals,_1,_2),ret.strN,ret.idxN);
    ret.win = fgGuiRadio(ret.strN,vals,labels);
    return ret;
}

// */
