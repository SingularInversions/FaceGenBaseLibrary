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
#include "FgTime.hpp"

using namespace std;

void
fgGuiTestmScroll(const FgArgs &)
{
    FgString        store = fgDirUserAppDataLocalFaceGen("Base","GUI Testm Trackbar");
    g_gg = FgGuiGraph(store);
    string          text1,text2;
    for (size_t ii=0; ii<100; ++ii)
        text1 += "Tab 1 Line " + fgToStringDigits(ii,3) + "\n";
    for (size_t ii=0; ii<10; ++ii)
        text2 += "Tab 2 Line " + fgToStringDigits(ii,3) + "\n";
    FgGuiTab        tab1 = fgGuiTab("Tab 1",fgGuiSplitScroll(fgSvec(fgGuiText(text1)))),
                    tab2 = fgGuiTab("Tab 2",fgGuiSplitScroll(fgSvec(fgGuiText(text2))));
    fgGuiImplStart(
        FgString("FG GUI testm scroll"),
        fgGuiTabs(fgSvec(tab1,tab2)),
        store);
}

void
fgGuiTestmText(const FgArgs &)
{
    FgString        store = fgDirUserAppDataLocalFaceGen("Base","GUI Testm Trackbar");
    g_gg = FgGuiGraph(store);
    string          text1 = "This is short",
                    text2 =
        "This is a multiline text which, in all likelihood, needs to be wrapped around, to fit "
        "the space alloted, at least once.\n"
        "And it contains a crlf";
    FgGuiTab        tab1 = fgGuiTab("Tab 1",fgGuiText(text1)),
                    tab2 = fgGuiTab("Tab 2",fgGuiText(text2));
    fgGuiImplStart(
        FgString("FG GUI testm text"),
        fgGuiTabs(fgSvec(tab1,tab2)),
        store);
}

void
fgGuiTestmDialogSplashScreen(const FgArgs &)
{
    std::function<void(void)>     f = fgGuiDialogSplashScreen();
    fgout << fgnl << "Splash screen displayed, waiting 3 seconds ... \n";
    fgSleep(3);
    f();
}

// */
