//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Author:      Andrew Beatty
// Date:        March 17, 2011
//

#include "stdafx.h"

#include "FgGuiApi.hpp"
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
STUB(FgGuiApiDynamic)
STUB(FgGuiApiGroupbox)
STUB(FgGuiApiImage)
STUB(FgGuiApiRadio)
STUB(FgGuiApiSelect)
STUB(FgGuiApiSlider)
STUB(FgGuiApiSpacer)
STUB(FgGuiApiSplit)
STUB(FgGuiApiSplitAdj)
STUB(FgGuiApiSplitScroll)
STUB(FgGuiApiTabs)
STUB(FgGuiApiText)
STUB(FgGuiApiTextEdit)

void
fgGuiDialogMessage(const FgString &,const FgString &)
{fgThrowNotImplemented(); }

FgOpt<FgString>
fgGuiDialogFileLoad(const FgString &,const vector<std::string> &)
{
    fgThrowNotImplemented();
    return FgOpt<FgString>();
}

FgOpt<FgString>
fgGuiDialogFileSave(const FgString &,const std::string &)
{
    fgThrowNotImplemented();
    return FgOpt<FgString>();
}

FgOpt<FgString>
fgGuiDialogDirSelect()
{
    fgThrowNotImplemented();
    return FgOpt<FgString>();
}

void
fgGuiDialogProgress(const FgString &,uint,FgGuiActionProgress)
{fgThrowNotImplemented(); }

boost::function<void(void)>
fgGuiDialogSplashScreen()
{return boost::function<void(void)>(); }

void
FgGuiGraph::updateScreen() {}

void
FgGuiGraph::quit() {}

// */
