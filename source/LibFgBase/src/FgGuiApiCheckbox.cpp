//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgGuiApi.hpp"

using namespace std;


namespace Fg {

GuiPtr
guiCheckbox(String8 const & label,IPT<bool> const & node)
{
    GuiCheckbox         cb;
    cb.label = label;
    cb.getFn = [node]() {return node.val(); };
    cb.clickFn = [node]()
    {
        bool & sel = node.ref();
        sel = !sel;
    };
    return make_shared<GuiCheckbox>(cb);
}

GuiPtr
guiCheckboxes(String8s const & labels,Svec<IPT<bool> > const & selNs)
{
    FGASSERT(!labels.empty());
    FGASSERT(selNs.size() == labels.size());
    GuiPtrs                   wins;
    for (size_t ii=0; ii<labels.size(); ++ii)
        wins.push_back(guiCheckbox(labels[ii],selNs[ii]));
    return guiSplitV(wins);
}

}

// */
