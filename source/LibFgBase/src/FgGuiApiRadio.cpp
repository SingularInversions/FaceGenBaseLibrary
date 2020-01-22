//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgGuiApiRadio.hpp"
#include "FgGuiApi.hpp"

using namespace std;
using namespace std::placeholders;


namespace Fg {

GuiPtr
guiRadio(Ustrings const & labels,IPT<size_t> idxN)
{
    FGASSERT(!labels.empty());
    GuiRadio         api;
    api.horiz = false;
    api.labels = labels;
    api.selection = idxN;
    return make_shared<GuiRadio>(api);
}

GuiVal<Ustring>
guiRadioLabel(Ustrings const & labels,IPT<size_t> idxN)
{
    GuiVal<Ustring>     ret;
    ret.win = guiRadio(labels,idxN);
    ret.valN = link1<size_t,Ustring>(idxN,[labels](size_t const & idx)
    {
        if (idx < labels.size())
            return labels[idx];
        else
            return labels[0];
    });
    return ret;
}

}

// */
