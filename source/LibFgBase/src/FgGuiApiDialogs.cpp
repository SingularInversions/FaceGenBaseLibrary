//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgGuiApiDialogs.hpp"

using namespace std;


namespace Fg {

GuiPtr              guiLoadButton(
    String8 const &             buttonText,
    String8 const &             fileTypesDescription,
    Strings const &             extensions,
    String const &              storeID,
    IPT<String8> const &        selection)
{
    auto                fnLoadFile = [=]()
    {
        Opt<String8>     fname = guiDialogFileLoad(fileTypesDescription,extensions,storeID);
        if (fname.valid())
            selection.set(fname.val());
    };
    return guiButton(buttonText,fnLoadFile);
}

}

// */
