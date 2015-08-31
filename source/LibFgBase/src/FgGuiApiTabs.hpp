//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     April 12, 2011
//

#ifndef FGGUIAPITABS_HPP
#define FGGUIAPITABS_HPP

#include "FgGuiApiBase.hpp"
#include "FgStdString.hpp"
#include "FgStdVector.hpp"

struct  FgGuiTab
{
    FgString        label;
    FgGuiPtr        win;
    uint            padLeft;        // pixels ...
    uint            padRight;
    uint            padTop;
    uint            padBottom;

    FgGuiTab()
        : padLeft(1), padRight(1), padTop(1), padBottom(1)
    {}

    FgGuiTab(const FgString & l,FgGuiPtr w)
    : label(l), win(w), padLeft(1), padRight(1), padTop(1), padBottom(1)
    {}

    FgGuiTab(const FgString & l,bool spacer,FgGuiPtr w)
    :   label(l), win(w),
        padLeft(spacer ? 5 : 1), padRight(spacer ? 5 : 1),
        padTop(spacer ? 10 : 1), padBottom(1)
    {}
};

inline
FgGuiTab
fgGuiTab(const string & l,FgGuiPtr w)
{return FgGuiTab(fgTr(l),w); }

inline
FgGuiTab
fgGuiTab(const string & l,bool s,FgGuiPtr w)
{return FgGuiTab(fgTr(l),s,w); }

struct  FgGuiApiTabs : FgGuiApi<FgGuiApiTabs>
{
    vector<FgGuiTab>    tabs;

    explicit
    FgGuiApiTabs(const vector<FgGuiTab> & t) :
        tabs(t)
    {}
};

inline
FgGuiPtr
fgGuiTabs(const vector<FgGuiTab> & tabs)
{return fgnew<FgGuiApiTabs>(tabs); }

#endif
