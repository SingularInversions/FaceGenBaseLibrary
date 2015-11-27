//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     April 23, 2011
//

#ifndef FGGUIAPIRADIO_HPP
#define FGGUIAPIRADIO_HPP

#include "FgGuiApiBase.hpp"
#include "FgDepGraph.hpp"

struct FgGuiApiRadio : FgGuiApi<FgGuiApiRadio>
{
    bool                    horiz;
    vector<FgString>        labels;     // Can be of size zero
    FgDgn<size_t>           selection;
    // Node idx for updating can be different from selection (must be exclusive):
    uint                    updateIdx;
};

FgGuiPtr
fgGuiRadio(FgDgn<size_t> selN,const vector<FgString> & labels);

template<class T>
FGLINK(fgLinkSelect)
{
    FGLINKARGS(2,1);
    const vector<T> &   objects = inputs[0]->valueRef();
    size_t              sel = inputs[1]->valueRef();
    // Invalid sel value is not an error since invalid selection can be loaded during restore:
    if (sel >= objects.size())
        sel = 0;
    T &                 val = outputs[0]->valueRef();
    val = objects[sel];
}

template<class T>
struct  FgGuiOutput
{
    FgGuiPtr        window;
    FgDgn<T>        output;
};

template<class T>
FgGuiOutput<T>
fgGuiRadioObjects(
    const vector<T> &           objects,
    const vector<FgString> &    labels,
    size_t                      initVal,
    const FgString &            saveLabel="")   // State saved if label assigned
{
    FgGuiOutput<T>              ret;
    FGASSERT(objects.size() == labels.size());
    FGASSERT(initVal < labels.size());
    FgDgn<vector<T> >           objectsN = g_gg.addNode(objects);
    FgDgn<size_t>               selN;
    if (saveLabel.empty())
        selN = g_gg.addNode(initVal);
    else {
        selN = g_gg.addInput(initVal,saveLabel);
        // Reset selection if restored value invalid:
        if (g_gg.getVal(selN) >= labels.size())
            g_gg.setVal(selN,initVal);
    }
    ret.output = g_gg.addNode(T());
    g_gg.addLink(fgLinkSelect<T>,fgUints(objectsN,selN),ret.output);
    ret.window = fgGuiRadio(selN,labels);
    return ret;
}

#endif
