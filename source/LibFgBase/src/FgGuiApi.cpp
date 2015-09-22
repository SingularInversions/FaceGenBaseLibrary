//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: Sept. 10, 2015
//

#include "stdafx.h"
#include "FgGuiApi.hpp"
#include "FgCommand.hpp"

using namespace std;

void
fgGuiTestmScroll(const FgArgs &)
{
    FgString        store = fgDirUserAppDataLocalFaceGen("Base","GUI Testm Trackbar");
    g_gg = FgGuiGraph(store);
    string          text;
    for (size_t ii=0; ii<100; ++ii)
        text += "Line " + fgToStringDigits(ii,3) + "\n";
    FgGuiTab        tab1 = fgGuiTab("Tab 1",fgGuiSplitScroll(fgSvec(fgGuiTextRich(text)))),
                    tab2 = fgGuiTab("Tab 2",fgGuiSplitScroll(fgSvec(fgGuiTextRich(text))));
    fgGuiImplStart(
        FgString("FG GUI manual test"),
        fgGuiTabs(fgSvec(tab1,tab2)),
        store);
}

// */
