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
fgGuiText(FgDgn<FgString> node,uint minWidth,bool rich)
{
    FgGuiApiText    gt;
    gt.content = node;
    gt.updateFlagIdx = g_gg.addUpdateFlag(node);
    gt.minWidth = minWidth;
    gt.rich = rich;
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
valToTextFixed(FgDgn<double> valN,uint numFraction)
{
    double          val = g_gg.getVal(valN);
    return fgToFixed(val,numFraction);
}

static
void
textToVal(FgDgn<double> valN,FgVectD2 clip,FgFuncD2D t2v,const FgString & str)
{
    // Behaviour for non-numerical strings varies even between debug and release, the
    // latter returning very small values for null string:
    double          val = fgFromString<double>(str.as_ascii());
    if (t2v)
        val = t2v(val);
    val = fgClip(val,clip[0],clip[1]);
    g_gg.setVal(valN,val);
}

FgGuiPtr
fgGuiTextEditFixed(FgDgn<double> valN,FgVectD2 bounds,uint numFraction)
{
    FgGuiApiTextEdit    te;
    te.updateFlagIdx = g_gg.addUpdateFlag(valN);
    te.minWidth = 50;
    te.wantStretch = false;
    te.getInput = boost::bind(valToTextFixed,valN,numFraction);
    te.setOutput = boost::bind(textToVal,valN,bounds,FgFuncD2D(),_1);
    return fgGuiPtr(te);
}

static
FgString
valToTextFloat(FgDgn<double> valN,uint numDigits,FgFuncD2D v2t)
{
    double te = g_gg.getVal(valN);
    if (v2t)
        te = v2t(te);
    return FgString(fgToStringPrecision(te,numDigits));
}

FgGuiPtr
fgGuiTextEditFloat(FgDgn<double> valN,FgVectD2 bounds,uint numDigits,FgFuncD2D v2t,FgFuncD2D t2v)
{
    FgGuiApiTextEdit    te;
    te.updateFlagIdx = g_gg.addUpdateFlag(valN);
    te.getInput = boost::bind(valToTextFloat,valN,numDigits,v2t);
    te.setOutput = boost::bind(textToVal,valN,bounds,t2v,_1);
    te.minWidth = 80;
    te.wantStretch = false;
    return fgsp(te);
}

// */
