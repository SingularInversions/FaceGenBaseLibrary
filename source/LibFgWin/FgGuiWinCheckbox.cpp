//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     March 19, 2011
//

#include "stdafx.h"

#include "FgGuiApiCheckbox.hpp"
#include "FgGuiWin.hpp"
#include "FgThrowWindows.hpp"
#include "FgMatrixC.hpp"

using namespace std;

struct  FgGuiWinCheckbox : public FgGuiOsBase
{
    FgGuiApiCheckbox    m_api;
    HWND                hwndCheckbox,
                        hwndThis;

    FgGuiWinCheckbox(const FgGuiApiCheckbox & api)
    : m_api(api)
    {}

    virtual void
    create(HWND parentHwnd,int ident,const FgString &,DWORD extStyle,bool visible)
    {
//fgout << fgnl << "FgGuiWinCheckbox::create " << m_api.label << fgpush;
        FgCreateChild   cc;
        cc.extStyle = extStyle;
        cc.visible = visible;
        fgCreateChild(parentHwnd,ident,this,cc);
//fgout << fgpop;
    }

    virtual void
    destroy()
    {
        // Automatically destroys children first:
        DestroyWindow(hwndThis);
    }

    virtual FgVect2UI
    getMinSize() const
    {return FgVect2UI(150,24); }

    virtual FgVect2B
    wantStretch() const
    {return FgVect2B(false,false); }

    virtual void
    updateIfChanged()
    {
        if (g_gg.dg.update(m_api.updateFlagIdx))
            updateCheckbox();
    }

    virtual void
    moveWindow(FgVect2I lo,FgVect2I sz)
    {
//fgout << fgnl << "FgGuiWinCheckbox::moveWindow " << lo << " , " << sz;
        MoveWindow(hwndThis,lo[0],lo[1],sz[0],sz[1],FALSE);
    }

    virtual void
    showWindow(bool s)
    {ShowWindow(hwndThis,s ? SW_SHOW : SW_HIDE); }

    LRESULT
    wndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
    {
        if (msg == WM_CREATE) {
//fgout << fgnl << "FgGuiWinCheckbox::WM_CREATE " << m_api.label;
            hwndThis = hwnd;
            hwndCheckbox =
                CreateWindowEx(0,
                    TEXT("button"),     // Standard controls class name for all buttons
                    m_api.label.as_wstring().c_str(),
                    WS_CHILD | WS_VISIBLE |
                    BS_CHECKBOX,        // Checkbox type button
                    0,0,0,0,            // Will be sent MOVEWINDOW messages.
                    hwnd,
                    HMENU(0),
                    s_fgGuiWin.hinst,
                    NULL);              // No WM_CREATE parameter
            FGASSERTWIN(hwndCheckbox != 0);
            updateCheckbox();
        }
        else if (msg == WM_SIZE) {      // Sends new size of client area:
            int     wid = LOWORD(lParam);
            int     hgt = HIWORD(lParam);
//fgout << fgnl << "FgGuiWinCheckbox::WM_SIZE " << m_api.label << " : " << wid << "," << hgt;
            if (wid*hgt > 0)
                MoveWindow(hwndCheckbox,0,0,wid,hgt,TRUE);
        }
        else if (msg == WM_COMMAND) {
            WORD    ident = LOWORD(wParam);
            WORD    code = HIWORD(wParam);
            if (code == 0) {            // checkbox clicked
                FGASSERT(ident == 0);
                bool val = !g_gg.getVal(m_api.val);
                g_gg.setVal(m_api.val,val);
                g_gg.updateScreen();
            }
        }
        else 
            return DefWindowProc(hwnd,msg,wParam,lParam);
        return 0;
    }

    void
    updateCheckbox()
    {
        bool    val = g_gg.getVal(m_api.val);
        SendMessage(hwndCheckbox,BM_SETCHECK,val ? 1 : 0,0);
    }
};

FgSharedPtr<FgGuiOsBase>
fgGuiGetOsInstance(const FgGuiApiCheckbox & def)
{return FgSharedPtr<FgGuiOsBase>(new FgGuiWinCheckbox(def)); }
