//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgGuiApi.hpp"

using namespace std;


namespace Fg {

GuiPtr
guiCheckbox(const Ustring & label,const IPT<bool> & node)
{
    GuiCheckbox      cb;
    cb.label = label;
    cb.val = node;
    cb.updateFlag = makeUpdateFlag(node);
    return make_shared<GuiCheckbox>(cb);
}

GuiPtr
guiCheckboxes(Ustrings const & labels,vector<IPT<bool> > const & selNs)
{
    FGASSERT(!labels.empty());
    FGASSERT(selNs.size() == labels.size());
    GuiPtrs                   wins;
    for (size_t ii=0; ii<labels.size(); ++ii)
        wins.push_back(guiCheckbox(labels[ii],selNs[ii]));
    return guiSplit(false,wins);
}

GuiVal<Bools>
guiCheckboxes(Ustrings const & labels,const Bools & defaults)
{
    FGASSERT(labels.size() == defaults.size());
    vector<IPT<bool> >          selNs;
    for (size_t ii=0; ii<labels.size(); ++ii) {
        IPT<bool>               selN = makeIPT<bool>(defaults[ii]);
        selNs.push_back(selN);
    }
    GuiVal<Bools>      ret;
    ret.valN = linkCollate(selNs);
    ret.win = guiCheckboxes(labels,selNs);
    return ret;
}

}

// */
