//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

//

#include "stdafx.h"

#include "FgGuiApiText.hpp"

using namespace std;
using namespace std::placeholders;

namespace Fg {

GuiPtr
guiText(NPT<Ustring> node,uint minWidth,bool rich)
{
    GuiText    gt;
    gt.content = node;
    gt.wantStretch[0] = true;
    gt.minWidth = minWidth;
    gt.rich = rich;
    return std::make_shared<GuiText>(gt);
}

GuiPtr
guiTextLines(NPT<Ustring> node,uint minHeight,bool wantStretchVert)
{
    GuiText    gt;
    gt.content = node;
    gt.wantStretch[0] = true;
    gt.wantStretch[1] = wantStretchVert;
    gt.minHeight = minHeight;
    return std::make_shared<GuiText>(gt);
}

GuiPtr
guiText(Ustring txt,uint minWidth)
{
    GuiText    gt;
    gt.content = makeIPT(txt);
    gt.minWidth = minWidth;
    return std::make_shared<GuiText>(gt);
}

GuiPtr
guiTextEdit(IPT<Ustring> node,bool wantStretch)
{
    GuiTextEdit    gtr;
    gtr.updateFlag = makeUpdateFlag(node);
    gtr.getInput = [node](){return node.val(); };
    gtr.setOutput = [node](Ustring s){node.set(s); };
    gtr.minWidth = 100;
    gtr.wantStretch = wantStretch;
    return std::make_shared<GuiTextEdit>(gtr);
}

static
void
textToVal2(IPT<double> valN,VecD2 clip,FgFuncD2D t2v,const Ustring & str)
{
    double          val = 0.0;
    try {val = fgFromString<double>(str.as_ascii()); }
    catch (...) {}
    if (t2v)
        val = t2v(val);
    val = clampBounds(val,clip[0],clip[1]);
    valN.set(val);
}

GuiPtr
guiTextEditFixed(IPT<double> valN,VecD2 bounds,uint numFraction)
{
    GuiTextEdit    te;
    te.updateFlag = makeUpdateFlag(valN);
    te.minWidth = 50;
    te.wantStretch = false;
    te.getInput = [valN,numFraction](){return fgToFixed(valN.val(),numFraction); };
    te.setOutput = std::bind(textToVal2,valN,bounds,FgFuncD2D(),_1);
    return guiMakePtr(te);
}

static
Ustring
valToTextFloat2(IPT<double> valN,uint numDigits,FgFuncD2D v2t)
{
    double      te = valN.val();
    if (v2t)
        te = v2t(te);
    return Ustring(fgToStringPrecision(te,numDigits));
}

GuiPtr
guiTextEditFloat(IPT<double> valN,VecD2 bounds,uint numDigits,FgFuncD2D v2t,FgFuncD2D t2v)
{
    GuiTextEdit      te;
    te.updateFlag = makeUpdateFlag(valN);
    te.getInput = std::bind(valToTextFloat2,valN,numDigits,v2t);
    te.setOutput = std::bind(textToVal2,valN,bounds,t2v,_1);
    te.minWidth = 80;
    te.wantStretch = false;
    return std::make_shared<GuiTextEdit>(te);
}

}

// */
