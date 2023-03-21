//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
//

#include "stdafx.h"

#include "FgGuiApiGroupbox.hpp"
#include "FgGuiWin.hpp"
#include "FgThrowWindows.hpp"
#include "FgMatrixC.hpp"

using namespace std;

namespace Fg {

const uint padHorz = 8;
const uint padTop = 20;
const uint padBot = 5;

struct  GuiGroupboxWin : public GuiBaseImpl
{
    GuiGroupbox             m_api;
    GuiImplPtr              m_contents;
    HWND                    m_hwndGb;
    String8                m_store;

    GuiGroupboxWin(const GuiGroupbox & api)
    : m_api(api)
    {m_contents = m_api.contents->getInstance(); }

    virtual void
    create(HWND parentHwnd,int,String8 const & store,DWORD extStyle,bool visible)
    {
//fgout << fgnl << "GuiGroupboxWin::create " << m_api.label << fgpush;
        m_store = store;
        int     flags = WS_CHILD;
        if (visible)
            flags = flags | WS_VISIBLE;
        m_hwndGb =
            CreateWindowEx(0,
                TEXT("button"),         // Standard controls class name for all buttons
                m_api.label.as_wstring().c_str(),
                flags | BS_GROUPBOX,    // Groupbox type button
                0,0,0,0,                // Will be sent MOVEWINDOW messages.
                parentHwnd,
                HMENU(0),
                s_guiWin.hinst,
                NULL);                  // No WM_CREATE parameter
        FGASSERTWIN(m_hwndGb != 0);
        m_contents->create(parentHwnd,1,m_store,extStyle,visible);
//fgout << fgpop;
    }

    virtual void
    destroy()
    {
        DestroyWindow(m_hwndGb);
        m_contents->destroy();
    }

    virtual Vec2UI
    getMinSize() const
    {return (m_contents->getMinSize() + Vec2UI(2*padHorz,padTop+padBot)); }

    virtual Vec2B
    wantStretch() const
    {return m_contents->wantStretch(); }

    virtual void
    updateIfChanged()
    {m_contents->updateIfChanged(); }

    virtual void
    moveWindow(Vec2I lo,Vec2I sz)
    {
//fgout << fgnl << "GuiGroupboxWin::moveWindow " << lo << " , " << sz;
        if (sz[0]*sz[1] > 0) {
            MoveWindow(m_hwndGb,lo[0],lo[1],sz[0],sz[1],TRUE);
            Vec2I    clo(padHorz,padTop),
                        csz(sz[0]-2*padHorz,sz[1]-padTop-padBot);
            m_contents->moveWindow(lo+clo,csz);
        }
    }

    virtual void
    showWindow(bool s)
    {
        ShowWindow(m_hwndGb,s ? SW_SHOW : SW_HIDE);
        m_contents->showWindow(s);
    }
};

GuiImplPtr
guiGetOsImpl(const GuiGroupbox & def)
{return GuiImplPtr(new GuiGroupboxWin(def)); }

}
