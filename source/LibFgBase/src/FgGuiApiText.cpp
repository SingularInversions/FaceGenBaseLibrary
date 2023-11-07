//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgGuiApi.hpp"
#include "FgBounds.hpp"
#include "FgParse.hpp"

using namespace std;
using namespace std::placeholders;

namespace Fg {

GuiPtr              guiText(NPT<String8> node,Vec2B wantStretch,bool rich)
{
    Str32s              lines = splitLines(toUtf32(node.cref().m_str),0,true);
    size_t              maxWid = clamp<size_t>(cMaxElem(cSizes(lines)),12,90);
    GuiText             gt;
    gt.content = node;
    gt.wantStretch = wantStretch;
    gt.minWidth = cMax(scast<uint>(maxWid),1U);     // min 1 in case of empty line(s)
    gt.minHeight = scast<uint>(lines.size());
    gt.rich = rich;
    return std::make_shared<GuiText>(gt);
}

GuiPtr              guiTextLines(NPT<String8> node,uint minWidth,uint minHeight,Vec2B wantStretch)
{
    GuiText             gt;
    gt.content = node;
    gt.wantStretch = wantStretch;
    gt.minWidth = minWidth;
    gt.minHeight = minHeight;
    return std::make_shared<GuiText>(gt);
}

GuiPtr              guiTextEdit(IPT<String8> node,bool wantStretch)
{
    GuiTextEdit         gtr;
    gtr.updateFlag = makeUpdateFlag(node);
    gtr.getInput = [node](){return node.val(); };
    gtr.setOutput = [node](String8 s){node.set(s); };
    gtr.minWidth = 100;
    gtr.wantStretch = wantStretch;
    return std::make_shared<GuiTextEdit>(gtr);
}

static void         strToSetVal(String8 str,VecD2 bounds,Sfun<void(double)> setVal)
{
    if (str.is_ascii()) {
        Opt<double>         vo = fromStr<double>(str.m_str);
        if (vo.has_value()) {
            double          val = clamp(vo.value(),bounds);
            setVal(val);
        }
    }
};

GuiPtr              guiTextEditFixed(
    DfgFPtr             updateFlag,
    Sfun<double()>      getVal,
    Sfun<void(double)>  setVal,
    VecD2               bounds,
    uint                numFraction)
{
    GuiTextEdit         te;
    te.updateFlag = updateFlag;
    te.minWidth = 50;
    te.wantStretch = false;
    te.getInput = [=]()
    {
        return toStrFixed(getVal(),numFraction);
    };
    te.setOutput = bind(strToSetVal,_1,bounds,setVal);
    return guiMakePtr(te);
}

GuiPtr              guiTextEditFixed(IPT<double> valN,VecD2 bounds,uint numFraction)
{
    auto            getVal = [=](){return valN.val(); };
    auto            setVal = [=](double val){valN.set(val); };
    return guiTextEditFixed(makeUpdateFlag(valN),getVal,setVal,bounds,numFraction);
}

GuiPtr              guiTextEditFloat(
    DfgFPtr             updateFlag,
    Sfun<double()>      getVal,
    Sfun<void(double)>  setVal,
    VecD2               bounds,
    uint                numDigits)
{
    GuiTextEdit         te;
    te.updateFlag = updateFlag;
    te.minWidth = 80;
    te.wantStretch = false;
    te.getInput = [=]()
    {
        return toStrPrec(getVal(),numDigits);
    };
    te.setOutput = bind(strToSetVal,_1,bounds,setVal);
    return guiMakePtr(te);
}

GuiPtr              guiTextEditFloat(IPT<double> valN,VecD2 bounds,uint numDigits)
{
    auto                getVal = [=](){return valN.val(); };
    auto                setVal = [=](double val){valN.set(val); };
    return guiTextEditFloat(makeUpdateFlag(valN),getVal,setVal,bounds,numDigits);
}

}

// */
