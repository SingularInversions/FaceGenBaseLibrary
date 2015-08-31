//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Oct 12, 2011
//

#include "stdafx.h"
#include "RichEdit.h"
#include "FgGuiApiText.hpp"
#include "FgGuiWin.hpp"
#include "FgThrowWindows.hpp"
#include "FgParse.hpp"

using namespace std;

struct  FgGuiWinTextRich : public FgGuiOsBase
{
    FgGuiApiTextRich    m_api;
    HWND                hwndText;
    HWND                hwndThis;
    // Estimated in constructor and assigned in WM_CREATE. Only Y val currently used (X is just an average):
    FgVect2UI           m_fontDims;
    // Cached for speed:  (FgString::count currently brutally slow)
    uint                m_textNumLinesByCr; // Assigned in constructor and in updateIfChanged()
    uint                m_textMaxWidthByCr; // ..
    wstring             m_content;          // ..
    FgVect2UI           m_pixSize;          // Assigned in WM_CREATE. Not yet used.

    FgGuiWinTextRich(const FgGuiApiTextRich & api) :
        m_api(api),
        m_fontDims(8,17)    // Value from a Win7 PC default font plus one pix separator
    {
        static HMODULE hmRichEdit = LoadLibrary(L"RichEd20.dll");
        setText();
    }

    virtual void
    create(HWND parentHwnd,int ident,const FgString &,DWORD extStyle,bool visible)
    {
//fgout << fgnl << "FgGuiWinTextRich::create" << fgpush;
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
    {
        // Max width is a good reading width (wraparound is unavoidable) and min width of
        // 200 is important because the content is dynamic, so could happen to be zero initially:
        uint        xmin = fgMin(400U,fgMax(m_textMaxWidthByCr,200U)),
                    ymin = m_textNumLinesByCr*m_fontDims[1];      // No wraparound estimate
        return FgVect2UI(xmin,ymin);
    }

    virtual FgVect2B
    wantStretch() const
    {return FgVect2B(true,false); }

    virtual void
    updateIfChanged()
    {
        if (g_gg.dg.update(m_api.updateFlagIdx)) {
            setText();
            SetWindowTextW(hwndText,m_content.c_str());
            //InvalidateRect(hwndText,NULL,FALSE);
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
//fgout << fgnl << "FgGuiWinTextRich::WM_CREATE";
                hwndThis = hwnd;
                hwndText = 
                    CreateWindowExW(0,
                        RICHEDIT_CLASSW,
                        L"HI",
                        WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE | ES_READONLY,
                        0,0,0,0,
                        hwnd,
                        NULL,
                        s_fgGuiWin.hinst,
                        NULL);              // No WM_CREATE parameter
                FGASSERTWIN(hwndText != 0);
                SendMessage(hwndText,EM_SETBKGNDCOLOR,0,GetSysColor(COLOR_3DFACE));
                SendMessage(hwndText,EM_AUTOURLDETECT,TRUE,0);
                //uint mask = SendMessage(hwndText,EM_GETEVENTMASK,0,0);
                //mask |= ENM_LINK;
                SendMessage(hwndText,EM_SETEVENTMASK,0,ENM_LINK);
                SetWindowTextW(hwndText,m_content.c_str());

                SIZE        sz;
                GetTextExtentPoint32(GetDC(hwndThis),m_content.c_str(),int(m_content.length()),&sz);
                m_pixSize = FgVect2UI(sz.cx,sz.cy);

                TEXTMETRIC  tm;
                GetTextMetrics(GetDC(hwnd),&tm);
                m_fontDims[0] = tm.tmAveCharWidth;
                m_fontDims[1] = tm.tmHeight + tm.tmExternalLeading;
                return 0;
            }
            case WM_SIZE:   // Sends new size of client area.
            {
                int     wid = LOWORD(lParam);
                int     hgt = HIWORD(lParam);
                if (wid*hgt > 0) {
                    MoveWindow(hwndText,0,0,wid,hgt,TRUE);
                }
                return 0;
            }
            case WM_NOTIFY:
            {
                LPNMHDR pnmh = (LPNMHDR)lParam;
                if (pnmh->code == EN_LINK) {
                    ENLINK *lnk = (ENLINK *)pnmh;
                    if (lnk->msg == WM_LBUTTONDOWN) {
                        SendMessage(pnmh->hwndFrom,EM_EXSETSEL,0,(LPARAM)&lnk->chrg);
                        vector<wchar_t> webLnk(lnk->chrg.cpMax - lnk->chrg.cpMin + 1);
                        SendMessage(pnmh->hwndFrom,EM_GETSELTEXT,0,(LPARAM)&webLnk[0]);
                        // Clear the selection:
                        CHARRANGE chrg;
                        chrg.cpMax = chrg.cpMin = lnk->chrg.cpMin;
                        SendMessage(pnmh->hwndFrom,EM_EXSETSEL,0,(LPARAM)&chrg);
                        // Goto the web link:
                        ShellExecuteW(hwnd,L"open",&webLnk[0],NULL,NULL,SW_SHOWNORMAL);
                        return TRUE;
                    }
                }
            }
        }
        return DefWindowProc(hwnd,message,wParam,lParam);
    }

    void
    setText()
    {
        const FgString &        text = g_gg.getVal(m_api.content);
        vector<uint>            tu = text.as_utf32();
        vector<vector<uint> >   tus = fgSplitLines(tu,true);
        m_textMaxWidthByCr = 0;
        m_textNumLinesByCr = uint(tus.size());
        for (size_t ii=0; ii<tus.size(); ++ii)
            if (tus[ii].size() > m_textMaxWidthByCr)
                m_textMaxWidthByCr = uint(tus[ii].size());
        m_content = text.as_wstring(); 
    }
};

FgSharedPtr<FgGuiOsBase>
fgGuiGetOsInstance(const FgGuiApiTextRich & def)
{return FgSharedPtr<FgGuiOsBase>(new FgGuiWinTextRich(def)); }
