//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Oct 18, 2011
//

#include "stdafx.h"

#include "FgGuiApiText.hpp"
#include "FgGuiWin.hpp"
#include "FgThrowWindows.hpp"

using namespace std;

struct  FgGuiWinTextEdit : public FgGuiOsBase
{
    HWND                hwndText;
    HWND                hwndThis;
    FgGuiApiTextEdit    m_api;
    FgVect2UI           m_fontDims;
    // Track when keyboard focus is on this edit box so we don't overwrite the user:
    bool                m_keyboardFocus;
    // We have to cache the current text contents to know when a user change has occurred:
    FgString            m_currText;

    FgGuiWinTextEdit(const FgGuiApiTextEdit & api)
        : m_api(api), m_fontDims(16,16), m_keyboardFocus(false)
    {}

    uint
    totalHeight() const
    {return m_fontDims[1]+6; }


    virtual void
    create(HWND parentHwnd,int ident,const FgString &,DWORD extStyle,bool visible)
    {
        FgCreateChild   cc;
        cc.extStyle = extStyle;
        cc.visible = visible;
        fgCreateChild(parentHwnd,ident,this,cc);
    }

    virtual void
    destroy()
    {
        // Automatically destroys children first:
        DestroyWindow(hwndThis);
    }

    virtual FgVect2UI
    getMinSize() const
    {return FgVect2UI(m_api.minWidth,totalHeight()); }

    virtual FgVect2B
    wantStretch() const
    {return FgVect2B(m_api.wantStretch,false); }

    virtual void
    updateIfChanged()
    {
        if (!m_keyboardFocus) {
            if (g_gg.dg.update(m_api.updateFlagIdx)) {
                const FgString &    txt = m_api.getInput();
                SetWindowText(hwndText,txt.as_wstring().c_str());
            }
        }
    }

    virtual void
    moveWindow(FgVect2I lo,FgVect2I sz)
    {MoveWindow(hwndThis,lo[0],lo[1],sz[0],sz[1],FALSE); }

    virtual void
    showWindow(bool s)
    {ShowWindow(hwndThis,s ? SW_SHOW : SW_HIDE); }
    
    LRESULT
    wndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
    {
        switch (message)
        {
            case WM_CREATE:
            {
                hwndThis = hwnd;
                hwndText = 
                    CreateWindowEx(0,
                        TEXT("edit"),
                        NULL,
                        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
                        0,0,0,0,            // Will be sent MOVEWINDOW messages.
                        hwnd,
                        HMENU(1),
                        s_fgGuiWin.hinst,
                        NULL);              // No WM_CREATE parameter
                FGASSERTWIN(hwndText != 0);
                TEXTMETRIC  tm;
                GetTextMetrics(GetDC(hwndThis),&tm);
                m_fontDims[0] = tm.tmAveCharWidth;
                m_fontDims[1] = tm.tmHeight + tm.tmExternalLeading;
                const FgString &    txt = m_api.getInput();
                g_gg.dg.update(m_api.updateFlagIdx);
                SetWindowText(hwndText,txt.as_wstring().c_str());
                m_currText = txt;
                return 0;
            }
            case WM_SIZE:   // Sends new size of client area.
            {
                int     wid = LOWORD(lParam),
                        hgt = HIWORD(lParam),
                        top = 0;
                if (wid*hgt > 0) {
                    // The edit box itself will be kept at single-line height, centred:
                    if (hgt > int(totalHeight())) {
                        top = (hgt - totalHeight()) / 2;
                        hgt = totalHeight();
                    }
                    MoveWindow(hwndText,0,top,wid,hgt,TRUE);
                }
                return 0;
            }
            case WM_COMMAND:
            {
                int     nc = HIWORD(wParam);
                // Windows sends both EN_UPDATE and EN_CHANGE every single time it redraws the
                // screen (WTF) so to avoid excessive recalc we need to check against changes
                // before doing anything:
                if (nc == EN_CHANGE) {
                    int len = GetWindowTextLength(hwndText);
                    // Leave room for NULL:
                    wstring     str(len+1,wchar_t(0));
                    // Windows only retrieves argLen-1 chars then sends a NULL:
                    GetWindowText(hwndText,&str[0],len+1);
                    // Use c string cons to avoid including NULL in string itself:
                    FgString    txt(&str[0]);
                    if (txt != m_currText) {
                        m_currText = txt;
                        m_api.setOutput(txt);
                        if (m_keyboardFocus)        // In this case the change is due to user interaction
                            g_gg.updateScreen();    // Won't affect this text edit box while focus is on
                    }
                }
                else if (nc == EN_SETFOCUS) {
                    m_keyboardFocus = true;
                }
                else if (nc == EN_KILLFOCUS) {
                    m_keyboardFocus = false;
                    const FgString &    txt = m_api.getInput();
                    SetWindowText(hwndText,txt.as_wstring().c_str());
                }
                return 0;
            }
        }
        return DefWindowProc(hwnd,message,wParam,lParam);
    }
};

FgSharedPtr<FgGuiOsBase>
fgGuiGetOsInstance(const FgGuiApiTextEdit & def)
{return FgSharedPtr<FgGuiOsBase>(new FgGuiWinTextEdit(def)); }
