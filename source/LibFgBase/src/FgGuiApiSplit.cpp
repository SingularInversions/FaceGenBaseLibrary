//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: June 10, 2014
//

#include "stdafx.h"

#include "FgGuiApiSplit.hpp"

using namespace std;

static
FgGuiPtrs
getPanes(FgGuiPtrs ps)
{return ps; }

FgGuiPtr
fgGuiSplit(bool horiz,const vector<FgGuiPtr> & panes)
{
    FgGuiApiSplit       ret;
    ret.horiz = horiz;
    ret.updateFlagIdx = g_gg.addNode(0);
    ret.getPanes = boost::bind(getPanes,panes);
    return fgsp(ret);
}

FgGuiPtr
fgGuiSplitScroll(const FgGuiPtrs & panes)
{
    FgGuiApiSplitScroll     ret;
    ret.updateFlagIdx = g_gg.addNode(0);      // dummy node ensures initial one-time setup
    ret.getPanes = boost::bind(getPanes,panes);
    return fgsp(ret);
}

FgGuiPtr
fgGuiSplitScroll(boost::function<FgGuiPtrs(void)> getPanes)
{
    FgGuiApiSplitScroll     ret;
    ret.updateFlagIdx = g_gg.addNode(0);      // dummy node ensures initial one-time setup
    ret.getPanes = getPanes;
    return fgsp(ret);
}

FgGuiPtr
fgGuiSplitScroll(
    uint                                updateNodeIdx,
    boost::function<FgGuiPtrs(void)>    getPanes)
{
    FgGuiApiSplitScroll     ret;
    ret.updateFlagIdx = g_gg.addUpdateFlag(updateNodeIdx);
    ret.getPanes = getPanes;
    return fgsp(ret);
}

// */
