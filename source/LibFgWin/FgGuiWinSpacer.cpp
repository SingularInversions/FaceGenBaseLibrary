//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
//

#include "stdafx.h"

#include "FgGuiApiSpacer.hpp"
#include "FgGuiWin.hpp"
#include "FgThrowWindows.hpp"
#include "FgMatrixC.hpp"
#include "FgBounds.hpp"
#include "FgMetaFormat.hpp"

using namespace std;

namespace Fg {

struct  GuiSpacerWin : public GuiBaseImpl
{
    GuiSpacer              m_api;

    GuiSpacerWin(const GuiSpacer & api) :
        m_api(api)
    {}

    virtual void
    create(HWND,int,Ustring const &,DWORD,bool)
    {}

    virtual void
    destroy()
    {}

    virtual Vec2UI
    getMinSize() const
    {return m_api.size; }

    virtual Vec2B
    wantStretch() const
    {return Vec2B(false,false); }

    virtual void
    updateIfChanged()
    {}

    virtual void
    moveWindow(Vec2I,Vec2I)
    {}

    virtual void
    showWindow(bool)
    {}

    virtual void
    saveState()
    {}
};

GuiImplPtr
guiGetOsImpl(const GuiSpacer & def)
{return GuiImplPtr(new GuiSpacerWin(def)); }

}
