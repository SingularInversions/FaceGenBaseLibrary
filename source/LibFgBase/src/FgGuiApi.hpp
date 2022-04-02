//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Declarative GUI API
// Currently only implemented on Windows

#ifndef FGGUIAPI_HPP
#define FGGUIAPI_HPP

#include "FgGuiApi3d.hpp"
#include "FgGuiApiButton.hpp"
#include "FgGuiApiCheckbox.hpp"
#include "FgGuiApiDialogs.hpp"
#include "FgGuiApiDynamic.hpp"
#include "FgGuiApiGroupbox.hpp"
#include "FgGuiApiImage.hpp"
#include "FgGuiApiRadio.hpp"
#include "FgGuiApiSelect.hpp"
#include "FgGuiApiSlider.hpp"
#include "FgGuiApiSpacer.hpp"
#include "FgGuiApiSplit.hpp"
#include "FgGuiApiTabs.hpp"
#include "FgGuiApiText.hpp"

namespace Fg {

// Gives radio button choice of supported image load/save formats:
GuiVal<String>      guiImageFormat(
    String const &      label,
    String8 const &     store);         // Don't store last choice if empty

// Set the cursor to show the application is busy.
// Resets automatically when application becomes responsive.
void                guiBusyCursor();

}

#endif
