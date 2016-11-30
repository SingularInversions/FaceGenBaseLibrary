//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: June 28, 2014
//

#include "stdafx.h"

#include "FgGuiApi.hpp"

using namespace std;

FgGuiPtr
fgGuiCheckbox(const FgString & label,FgDgn<bool> node)
{
    FgGuiApiCheckbox    cb;
    cb.label = label;
    cb.val = node;
    cb.updateFlagIdx = g_gg.addUpdateFlag(node);
    return fgsp(cb);
}

FgGuiPtr
fgGuiCheckboxes(const FgStrings & labels,FgDgn<vector<bool> > output)
{
    FGASSERT(!labels.empty());
    vector<FgGuiPtr>        wins;
    vector<FgDgn<bool> >    selNs;
    for (size_t ii=0; ii<labels.size(); ++ii) {
        selNs.push_back(g_gg.addNode(true));
        wins.push_back(fgGuiCheckbox(labels[ii],selNs.back()));
    }
    g_gg.addLink(fgLinkCollate<bool>,fgUints(selNs),fgUints(output));
    return fgGuiSplit(false,wins);
}

// */
