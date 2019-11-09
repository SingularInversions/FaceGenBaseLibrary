//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
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

namespace Fg {

void
guiStartImpl(const Ustring &,GuiPtr,const Ustring &,const GuiOptions &)
{fgout << fgnl << "GUI not implemented for unix." << std::flush; }

#define STUB(X)                                             \
GuiImplPtr                                    \
guiGetOsImpl(const X &)                               \
{return GuiImplPtr(); }

STUB(Gui3d)
STUB(GuiButton)
STUB(GuiCheckbox)
STUB(GuiDynamic)
STUB(GuiGroupbox)
STUB(GuiImage)
STUB(GuiRadio)
STUB(GuiSelect)
STUB(GuiSlider)
STUB(GuiSpacer)
STUB(GuiSplit)
STUB(GuiSplitAdj)
STUB(GuiSplitScroll)
STUB(GuiTabs)
STUB(GuiText)
STUB(GuiTextEdit)

void
guiDialogMessage(const Ustring &,const Ustring &)
{throw FgExceptionNotImplemented(); }

Opt<Ustring>
guiDialogFileLoad(const Ustring &,const vector<std::string> &,const string &)
{
    throw FgExceptionNotImplemented();
    return Opt<Ustring>();
}

Opt<Ustring>
guiDialogFileSave(const Ustring &,const std::string &)
{
    throw FgExceptionNotImplemented();
    return Opt<Ustring>();
}

Opt<Ustring>
guiDialogDirSelect()
{
    throw FgExceptionNotImplemented();
    return Opt<Ustring>();
}

bool
guiDialogProgress(const Ustring &,uint,WorkerFunc)
{throw FgExceptionNotImplemented(); return false; }

std::function<void(void)>
guiDialogSplashScreen()
{return std::function<void(void)>(); }

void
guiQuit() {}

void
winUpdateScreen() {}

}

// */
