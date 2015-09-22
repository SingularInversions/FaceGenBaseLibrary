//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     March 26, 2011
//

#include "stdafx.h"

#include "FgGuiApiGroupbox.hpp"
#include "FgGuiWin.hpp"
#include "FgThrowWindows.hpp"
#include "FgMatrixC.hpp"

using namespace std;

const uint padHorz = 8;
const uint padTop = 20;
const uint padBot = 5;

struct  FgGuiWinGroupbox : public FgGuiOsBase
{
    FgGuiApiGroupbox            m_api;
    FgSharedPtr<FgGuiOsBase>    m_contents;
    HWND                        m_hwndGb;
    FgString                    m_store;

    FgGuiWinGroupbox(const FgGuiApiGroupbox & api)
    : m_api(api)
    {m_contents = m_api.contents->getInstance(); }

    virtual void
    create(HWND parentHwnd,int,const FgString & store,DWORD extStyle,bool visible)
    {
//fgout << fgnl << "FgGuiWinGroupbox::create " << m_api.label << fgpush;
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
                s_fgGuiWin.hinst,
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

    virtual FgVect2UI
    getMinSize() const
    {return (m_contents->getMinSize() + FgVect2UI(2*padHorz,padTop+padBot)); }

    virtual FgVect2B
    wantStretch() const
    {return m_contents->wantStretch(); }

    virtual void
    updateIfChanged()
    {m_contents->updateIfChanged(); }

    virtual void
    moveWindow(FgVect2I lo,FgVect2I sz)
    {
//fgout << fgnl << "FgGuiWinGroupbox::moveWindow " << lo << " , " << sz;
        if (sz[0]*sz[1] > 0) {
            MoveWindow(m_hwndGb,lo[0],lo[1],sz[0],sz[1],TRUE);
            FgVect2I    clo(padHorz,padTop),
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

FgSharedPtr<FgGuiOsBase>
fgGuiGetOsInstance(const FgGuiApiGroupbox & def)
{return FgSharedPtr<FgGuiOsBase>(new FgGuiWinGroupbox(def)); }
