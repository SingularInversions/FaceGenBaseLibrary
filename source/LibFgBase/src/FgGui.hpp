//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Includes all of the GUI API and provides some commonly-used constructions
//
// GUI functionality is currently only implemented on Windows

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
#include "FgImageIo.hpp"

namespace Fg {

// radio selection dialog of given formats by their default descriptions:
GuiVal<ImgFormat>   guiImgFormatSelector(
    ImgFormats const &  imgFormats,     // ordered list of formats for radio selection
    String8 const &     store={});      // leave empty to avoid storing user setting

// Set the cursor to show the application is busy.
// Resets automatically when application becomes responsive.
void                guiBusyCursor();

}

#endif
