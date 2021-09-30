//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
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
guiRadio(String8s const & labels,IPT<size_t> idxN)
{
    FGASSERT(!labels.empty());
    GuiRadio            api;
    api.labels = labels;
    api.getFn = [idxN]() {return scast<uint>(idxN.val()); };
    api.setFn = [idxN](uint sel) {idxN.set(sel); };
    return make_shared<GuiRadio>(api);
}

GuiVal<String8>
guiRadioLabel(String8s const & labels,IPT<size_t> idxN)
{
    GuiVal<String8>     ret;
    ret.win = guiRadio(labels,idxN);
    ret.valN = link1<size_t,String8>(idxN,[labels](size_t const & idx)
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
