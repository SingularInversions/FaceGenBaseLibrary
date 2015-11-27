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
fgGuiText(FgDgn<FgString> node,uint minWidth)
{
    FgGuiApiText    gt;
    gt.content = node;
    gt.updateFlagIdx = g_gg.addUpdateFlag(node);
    gt.minWidth = minWidth;
    return fgsp(gt);
}

FgGuiPtr
fgGuiText(FgString txt,uint minWidth)
{
    FgGuiApiText    gt;
    gt.content = g_gg.addNode(txt);
    gt.updateFlagIdx = g_gg.addUpdateFlag(gt.content);
    gt.minWidth = minWidth;
    return fgsp(gt);
}

static
FgString
getText(FgDgn<FgString> n)
{return g_gg.getVal(n); }

static
void
setText(FgDgn<FgString> n,FgString v)
{
    g_gg.setVal(n,v);
    g_gg.updateScreen();
}

FgGuiPtr
fgGuiTextEdit(FgDgn<FgString> node,bool wantStretch)
{
    FgGuiApiTextEdit    gtr;
    gtr.updateFlagIdx = g_gg.addUpdateFlag(node);
    gtr.getInput = boost::bind(getText,node);
    gtr.setOutput = boost::bind(setText,node,_1);
    gtr.minWidth = 100;
    gtr.wantStretch = wantStretch;
    return fgsp(gtr);
}

static
FgString
valToText(FgDgn<double> valN)
{
    double te = g_gg.getVal(valN);
    return FgString(fgToStringPrecision(te,6));
}

static
void
textToVal(const FgString & text,FgDgn<double> valN,FgVect2D bounds)
{
    double  userVal = fgFromString<double>(text.as_ascii());
    // Behaviour for non-numerical strings varies even between debug and release, the
    // latter returning very small values for null string:
    if (userVal < bounds[0])
        userVal = bounds[0];
    if (userVal > bounds[1])
        userVal = bounds[1];
    g_gg.setVal(valN,userVal);
    g_gg.updateScreen();
}

FgGuiPtr
fgGuiTextEditFloat(FgDgn<double> valN,FgVect2D bounds)
{
    FgGuiApiTextEdit    te;
    te.updateFlagIdx = g_gg.addUpdateFlag(valN);
    te.getInput = boost::bind(valToText,valN);
    te.setOutput = boost::bind(textToVal,_1,valN,bounds);
    te.minWidth = 80;
    te.wantStretch = false;
    return fgsp(te);
}

// */
