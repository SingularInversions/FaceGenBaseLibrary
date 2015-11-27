//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     July 2, 2011
//

#include "stdafx.h"

#include "FgGuiApiButton.hpp"
#include "FgGuiWin.hpp"
#include "FgThrowWindows.hpp"
#include "FgMatrixC.hpp"

using namespace std;

struct  FgGuiWinButton : public FgGuiOsBase
{
    FgGuiApiButton      m_api;
    HWND                hwndButton,
                        hwndThis;

    FgGuiWinButton(const FgGuiApiButton & api)
    : m_api(api)
    {}

    virtual void
    create(HWND parentHwnd,int ident,const FgString &,DWORD extStyle,bool visible)
    {
//fgout << fgnl << "Button::create visible: " << visible << " extStyle: " << extStyle << fgpush;
        // We need a parent window in order to catch the button command messages:
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
    {return FgVect2UI(100,24); }

    virtual FgVect2B
    wantStretch() const
    {return FgVect2B(false,false); }

    virtual void
    updateIfChanged()
    {
//fgout << fgnl << "Button::updateIfChanged";
    }

    virtual void
    moveWindow(FgVect2I lo,FgVect2I sz)
    {
//fgout << fgnl << "Button::moveWindow: " << lo << "," << sz;
        MoveWindow(hwndThis,lo[0],lo[1],sz[0],sz[1],FALSE);
    }

    virtual void
    showWindow(bool s)
    {
//fgout << fgnl << "Button::showWindow: " << s;
        ShowWindow(hwndThis,s ? SW_SHOW : SW_HIDE);
    }

    LRESULT
    wndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
    {
        if (msg == WM_CREATE) {
//fgout << fgnl << "Button::WM_CREATE";
            // This is the first place we get the hwnd for this instance since this callback
            // happens before 'fgCreateChild' above returns:
            hwndThis = hwnd;
            hwndButton =
                CreateWindowEx(0,
                    TEXT("button"),     // Standard controls class name for all buttons
                    m_api.label.as_wstring().c_str(),
                    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                    0,0,0,0,            // Will be sent MOVEWINDOW messages.
                    hwnd,
                    HMENU(0),
                    s_fgGuiWin.hinst,
                    NULL);              // No WM_CREATE parameter
            FGASSERTWIN(hwndButton != 0);
        }
        else if (msg == WM_SIZE) {
            int     wid = LOWORD(lParam);
            int     hgt = HIWORD(lParam);
            if (wid*hgt > 0) {
//fgout << fgnl << "Button::WM_SIZE: " << wid << "," << hgt;
                int     buttonHgt = fgMin(int(getMinSize()[1]),hgt),
                        vspace = hgt - buttonHgt;
                MoveWindow(hwndButton,0,vspace/2,wid,buttonHgt,TRUE);
            }
        }
        else if (msg == WM_COMMAND) {
            WORD    ident = LOWORD(wParam);
            WORD    code = HIWORD(wParam);
            if (code == 0) {
                FGASSERT(ident == 0);
                m_api.action();
                g_gg.updateScreen();
            }
        }
//case WM_PAINT:
//fgout << fgnl << "Button::WM_PAINT";
        else
            return DefWindowProc(hwnd,msg,wParam,lParam);
        return 0;
    }
};

FgSharedPtr<FgGuiOsBase>
fgGuiGetOsInstance(const FgGuiApiButton & def)
{return FgSharedPtr<FgGuiOsBase>(new FgGuiWinButton(def)); }
