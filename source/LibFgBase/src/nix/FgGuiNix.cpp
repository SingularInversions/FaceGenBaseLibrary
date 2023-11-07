//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Author:      Andrew Beatty
// Date:        March 17, 2011
//

#include "stdafx.h"

#include "FgGuiApi.hpp"
#include "FgGuiApi3d.hpp"
#include "FgGuiApiMain.hpp"
#include "FgSerial.hpp"

using namespace std;

namespace Fg {

void                guiStartImpl(NPT<String8>,GuiPtr,String8 const &,const GuiOptions &)
{
    fgout << fgnl << "GUI not implemented for unix." << std::flush;
}

#define STUB(X) GuiImplPtr guiGetOsImpl(const X &) {return GuiImplPtr(); }

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

void                guiDialogMessage(String8 const &,String8 const &) {throw FgExceptionNotImplemented(); }
Opt<String8>        guiDialogFileLoad(String8 const &,const vector<std::string> &,string const &)
{
    throw FgExceptionNotImplemented();
    return Opt<String8>();
}
String8s            guiDialogFilesLoad(String8 const &,Strings const &,String const &)
{
    throw FgExceptionNotImplemented();
    return {};
}
Opt<String8>        guiDialogFileSave(String8 const &,const std::string &)
{
    throw FgExceptionNotImplemented();
    return Opt<String8>();
}
Opt<String8>        guiDialogDirSelect()
{
    throw FgExceptionNotImplemented();
    return Opt<String8>();
}
bool                guiDialogProgress(String8 const &,uint,WorkerFunc const &)
{
    throw FgExceptionNotImplemented();
    return false;
}
std::function<void(void)> guiDialogSplashScreen()
{
    return std::function<void(void)>();
}
void                guiQuit() {}
void                guiBusyCursor() {}

}

// */
