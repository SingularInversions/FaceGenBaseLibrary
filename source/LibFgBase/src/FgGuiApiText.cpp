//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: August 29, 2014
//

#include "stdafx.h"

#include "FgGuiApiText.hpp"

using namespace std;

FgGuiPtr
fgGuiText(FgDgn<FgString> node)
{
    FgGuiApiText        gtr;
    gtr.content = node;
    gtr.updateFlagIdx = g_gg.addUpdateFlag(node);
    return fgsp(gtr);
}

static
FgString
getText(FgDgn<FgString> n)
{return g_gg.getVal(n); }

static
void
setText(FgDgn<FgString> n,FgString v)
{g_gg.setVal(n,v); }

FgGuiPtr
fgGuiTextEdit(FgDgn<FgString> node)
{
    FgGuiApiTextEdit    gtr;
    gtr.updateFlagIdx = g_gg.addUpdateFlag(node);
    gtr.getInput = boost::bind(getText,node);
    gtr.setOutput = boost::bind(setText,node,_1);
    return fgsp(gtr);
}

FgGuiPtr
fgGuiTextRich(FgDgn<FgString> node)
{
    FgGuiApiTextRich    gtr;
    gtr.content = node;
    gtr.updateFlagIdx = g_gg.addUpdateFlag(node);
    return fgsp(gtr);
}

FgGuiPtr
fgGuiTextRich(FgString txt)
{
    return fgGuiTextRich(g_gg.addNode(txt));
}

// */
