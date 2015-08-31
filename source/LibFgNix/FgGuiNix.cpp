//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Author:      Andrew Beatty
// Date:        March 17, 2011
//

#include "stdafx.h"

#include "FgGuiApi3d.hpp"
#include "FgGuiApiButton.hpp"
#include "FgGuiApiCheckbox.hpp"
#include "FgGuiApiGroupbox.hpp"
#include "FgGuiApiImage.hpp"
#include "FgGuiApiRadio.hpp"
#include "FgGuiApiSlider.hpp"
#include "FgGuiApiSplit.hpp"
#include "FgGuiApiTabs.hpp"
#include "FgGuiApiDialogs.hpp"
#include "FgGuiApiText.hpp"
#include "FgGuiApiSelect.hpp"
#include "FgOut.hpp"

using namespace std;

void
fgGuiImplStart(const FgString &,FgGuiPtr,const FgString &,const FgGuiOptions &)
{fgout << fgnl << "GUI not implemented for unix." << std::flush; }

#define STUB(X)                                             \
FgSharedPtr<FgGuiOsBase>                                    \
fgGuiGetOsInstance(const X &)                               \
{return FgSharedPtr<FgGuiOsBase>(); }

STUB(FgGuiApi3d)
STUB(FgGuiApiButton)
STUB(FgGuiApiCheckbox)
STUB(FgGuiApiGroupbox)
STUB(FgGuiApiImage)
STUB(FgGuiApiRadio)
STUB(FgGuiApiSlider)
STUB(FgGuiApiSplit)
STUB(FgGuiApiSplitAdj)
STUB(FgGuiApiSplitScroll)
STUB(FgGuiApiTabs)
STUB(FgGuiApiText)
STUB(FgGuiApiTextEdit)
STUB(FgGuiApiTextRich)
STUB(FgGuiApiSelect)

void
fgGuiDialogMessage(const FgString &,const FgString &)
{fgThrowNotImplemented(); }

FgValidVal<FgString>
fgGuiDialogFileLoad(const FgString &,const vector<std::string> &)
{
    fgThrowNotImplemented();
    return FgValidVal<FgString>();
}

FgValidVal<FgString>
fgGuiDialogFileSave(const FgString &,const std::string &)
{
    fgThrowNotImplemented();
    return FgValidVal<FgString>();
}

FgValidVal<FgString>
fgGuiDialogDirSelect()
{
    fgThrowNotImplemented();
    return FgValidVal<FgString>();
}

void
fgGuiDialogProgress(const FgString &,uint,FgGuiActionProgress)
{fgThrowNotImplemented(); }

void
FgGuiGraph::updateScreen() {}

void
FgGuiGraph::quit() {}

// */
