//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgGuiApi.hpp"

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
        if (fname.has_value())
            selection.set(fname.value());
    };
    return guiButton(buttonText,fnLoadFile);
}

}

// */
