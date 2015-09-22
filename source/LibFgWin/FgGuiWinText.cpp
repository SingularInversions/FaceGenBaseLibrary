//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Oct 12, 2011
//

#include "stdafx.h"

#include "FgGuiApiText.hpp"
#include "FgGuiWin.hpp"
#include "FgThrowWindows.hpp"

using namespace std;

struct  FgGuiWinText : public FgGuiOsBase
{
    HWND                m_hwnd;
    FgGuiApiText        m_api;
    FgVect2UI           m_client;
    FgVect2UI           m_fontDims;
    FgVect2UI           m_textDims; // Cached for speed - FgString::count currently brutally slow

    FgGuiWinText(const FgGuiApiText & api)
        : m_api(api), m_fontDims(16,16)
    {
        const FgString & text = g_gg.getVal(m_api.content);
        m_textDims[1] = text.count('\n')+1;
    }

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
    {DestroyWindow(m_hwnd); }

    virtual FgVect2UI
    getMinSize() const
    {
        return FgVect2UI(300,m_textDims[1]*m_fontDims[1]);
    }

    virtual FgVect2B
    wantStretch() const
    {return FgVect2B(true,false); }

    virtual void
    updateIfChanged()
    {
        if (g_gg.dg.update(m_api.updateFlagIdx)) {
            const FgString & text = g_gg.getVal(m_api.content);
            m_textDims[1] = text.count('\n')+1;
            InvalidateRect(m_hwnd,NULL,FALSE);
        }
    }

    virtual void
    moveWindow(FgVect2I lo,FgVect2I sz)
    {MoveWindow(m_hwnd,lo[0],lo[1],sz[0],sz[1],TRUE); }

    virtual void
    showWindow(bool s)
    {ShowWindow(m_hwnd,s ? SW_SHOW : SW_HIDE); }

    LRESULT
    wndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
    {
        switch (message)
        {
            case WM_CREATE:
            {
                m_hwnd = hwnd;
                TEXTMETRIC  tm;
                GetTextMetrics(GetDC(hwnd),&tm);
                m_fontDims[0] = tm.tmAveCharWidth * 80;
                m_fontDims[1] = tm.tmHeight + tm.tmExternalLeading;
                return 0;
            }
            case WM_SIZE:   // Sends new size of client area.
            {
                m_client = FgVect2UI(LOWORD(lParam),HIWORD(lParam));
                return 0;
            }
            case WM_PAINT:
            {
                const FgString & text = g_gg.getVal(m_api.content);
                PAINTSTRUCT     ps;
                HDC             hdc = BeginPaint(hwnd,&ps);
                SetBkColor(hdc,GetSysColor(COLOR_ACTIVEBORDER));
                RECT            rect;
                rect.top = 0;
                rect.bottom = m_client[1];
                rect.left = 0;
                rect.right = m_client[0];
                DrawText(hdc,
                    // Use c_str() to gracefully handle empty text:
                    text.as_wstring().c_str(),
                    int(text.length()),
                    &rect,
                    DT_LEFT | DT_TOP);
                EndPaint(hwnd,&ps);
                return 0;
            }
        }
        return DefWindowProc(hwnd,message,wParam,lParam);
    }
};

FgSharedPtr<FgGuiOsBase>
fgGuiGetOsInstance(const FgGuiApiText & def)
{return FgSharedPtr<FgGuiOsBase>(new FgGuiWinText(def)); }
