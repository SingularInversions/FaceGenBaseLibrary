//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     March 19, 2011
//

#ifndef FGGUIAPICHECKBOX_HPP
#define FGGUIAPICHECKBOX_HPP

#include "FgGuiApiBase.hpp"
#include "FgDepGraph.hpp"

struct FgGuiApiCheckbox : FgGuiApi<FgGuiApiCheckbox>
{
    FgString        label;
    FgDgn<bool>     val;
    uint            updateFlagIdx;
};

FgGuiPtr
fgGuiCheckbox(const FgString & label,FgDgn<bool> node);

template<class T>
FGLINK(fgLinkCheckboxObject)
{
    FGLINKARGS(2,1);
    bool                sel = inputs[0]->valueRef();
    const T &           obj = inputs[1]->valueRef();
    vector<T> &         out = outputs[0]->valueRef();
    if (sel)
        out = fgSvec(obj);
    else
        out.clear();
}

// Have checkbox select a predefined object by outputting a vector that contains either 0 or 1 instances:
template<class T>
FgGuiPtr
fgGuiCheckboxObject(const FgString & label,FgDgn<bool> input,const T & object,FgDgn<vector<T> > output)
{
    g_gg.addLink(fgLinkCheckboxObject<T>,fgUints(input,g_gg.addNode(object,string("checkbox_")+typeid(T).name())),output);
    return fgGuiCheckbox(label,input);
}

FgGuiPtr
fgGuiCheckboxes(const FgStrings & labels,FgDgn<vector<bool> > output);

#endif
