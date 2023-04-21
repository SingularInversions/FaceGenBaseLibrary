//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
//

#include "stdafx.h"
#include "RichEdit.h"
#include "FgGuiApi.hpp"
#include "FgGuiWin.hpp"
#include "FgThrowWindows.hpp"
#include "FgParse.hpp"

using namespace std;

namespace Fg {

struct  GuiTextWin : public GuiBaseImpl
{
    GuiText             m_api;
    HWND                hwndText;
    HWND                hwndThis;
    // Cache text in UTF-16 format for Windows (UCS2 was only used prior to Windows2000):
    wstring             m_content;
    DfgFPtr             m_updateFlag;   // Track changes in above
    // Assigned in WM_CREATE and updated in updateIfChanged(). The min height is calculated using
    // m_maxMinWid below, regardless of actual width, to avoid the complexity of adjusting
    // Y-min dynamically with window width:
    Vec2UI              m_minSize;
    // Long text will have a minimum width of at least the following:
    static const uint   m_maxMinWid = 350;
    static const uint   m_minHgt = 16;

    GuiTextWin(const GuiText & api) :
        m_api(api),
        // Default to something before we've had a chance to work out real vals:
        m_minSize(m_maxMinWid,m_minHgt)
    {
        static HMODULE hmRichEdit = LoadLibrary(L"RichEd20.dll");
        String8 const &        text = m_api.content.cref();
        m_content = text.as_wstring();
        m_updateFlag = makeUpdateFlag(m_api.content);
    }

    virtual void    create(HWND parentHwnd,int ident,String8 const &,DWORD extStyle,bool visible)
    {
//fgout << fgnl << "GuiTextWin::create" << fgpush;
        WinCreateChild   cc;
        cc.extStyle = extStyle;
        cc.visible = visible;
        winCreateChild(parentHwnd,ident,this,cc);
//fgout << fgpop;
    }

    virtual void    destroy()
    {
        // Automatically destroys children (ie hwndText) first:
        DestroyWindow(hwndThis);
    }

    virtual Vec2UI  getMinSize() const
    {
        FGASSERT(m_minSize.cmpntsProduct() > 0);   // This shouldn't be called before it's been assigned.
        return m_minSize;
    }

    virtual Vec2B   wantStretch() const
    {
        bool        horiz = (m_api.wantStretch[0] || (m_minSize[0] > m_maxMinWid));
        return Vec2B(horiz,m_api.wantStretch[1]);
    }
    
    void            updateText()
    {
        BOOL            success = SetWindowTextW(hwndText,m_content.c_str());     // Update text in window
        FGASSERTWIN(success);
        if (m_content.empty())      // Reserve a min lines of max min width:
            m_minSize = Vec2UI(m_maxMinWid,m_minHgt*m_api.minHeight);
        else {                      // Calculate updated text dimensions:
            // Get single line height (16 on my PC):
            RECT        rs = {0,0,0,0};
            int         height = DrawText(GetDC(hwndText),L" ",1,&rs,DT_CALCRECT);
            FGASSERTWIN(height != 0);
            uint        singleLineHeight = uint(rs.bottom-rs.top);
            // Get the size without wraparound (but respecting CRLF).
            // If the width of this is less than m_maxMinWid then we can safely reduce with minimum width:
            RECT        r = {0,0,0,0};
            height = DrawText(GetDC(hwndText),m_content.c_str(),uint(m_content.size()),&r,DT_CALCRECT);
            FGASSERTWIN(height != 0);
            // Just a guess but without this the calculated size is not quite large enough for the edit
            // box to actually render the text without wraparound:
            const uint  richEditBorder = 2;
            m_minSize[0] = uint(r.right - r.left) + richEditBorder;
            m_minSize[1] = uint(r.bottom - r.top) + richEditBorder;
            if (m_minSize[0] > m_maxMinWid) {   // Wraparound required for max min size:
                r.right = m_maxMinWid;
                height = DrawText(GetDC(hwndText),m_content.c_str(),uint(m_content.size()),&r,DT_CALCRECT | DT_WORDBREAK);
                FGASSERTWIN(height != 0);
                m_minSize[0] = m_maxMinWid + richEditBorder;
                m_minSize[1] = r.bottom - r.top + richEditBorder;
            }
            if (m_minSize[0] < m_api.minWidth)
                m_minSize[0] = m_api.minWidth;
            if (singleLineHeight > 0) {         // Shouldn't happen but just in case
                uint        heightInLines = m_minSize[1] / singleLineHeight;
                if (heightInLines < m_api.minHeight)
                    m_minSize[1] = m_api.minHeight * singleLineHeight;
            }
        }
    }

    virtual void    updateIfChanged()
    {
        if (m_updateFlag->checkUpdate()) {
            String8 const &        text = m_api.content.cref();
            m_content = text.as_wstring(); 
            updateText();
            // TODO: if updateText() changes m_minSize we need to return a value (recursively)
            // so that main loop can call a resize on the whole window (which is potentially
            // affected).
        }
    }

    virtual void    moveWindow(Vec2I lo,Vec2I sz) {MoveWindow(hwndThis,lo[0],lo[1],sz[0],sz[1],FALSE); }
    virtual void    showWindow(bool s) {ShowWindow(hwndThis,s ? SW_SHOW : SW_HIDE); }

    LRESULT         wndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
    {
        switch (message)
        {
            case WM_CREATE:
            {
//fgout << fgnl << "GuiTextWin::WM_CREATE";
                hwndThis = hwnd;
                wstring         winClass;
                if (m_api.rich)
                    winClass = RICHEDIT_CLASSW;
                else
                    winClass = L"edit";
                hwndText = 
                    CreateWindowExW(0,
                        winClass.c_str(),
                        NULL,
                        WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE | ES_READONLY,
                        0,0,0,0,
                        hwnd,
                        HMENU(1),           // Assign identifier 1 to this child window
                        s_guiWin.hinst,
                        NULL);              // No WM_CREATE parameter
                FGASSERTWIN(hwndText != 0);
                SendMessage(hwndText,EM_SETBKGNDCOLOR,0,GetSysColor(COLOR_3DFACE));
                SendMessage(hwndText,EM_AUTOURLDETECT,TRUE,0);
                //uint mask = SendMessage(hwndText,EM_GETEVENTMASK,0,0);
                //mask |= ENM_LINK;
                SendMessage(hwndText,EM_SETEVENTMASK,0,ENM_LINK);
                // Call this here to ensure proper sizing from start:
                updateText();
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
};

GuiImplPtr          guiGetOsImpl(const GuiText & def) {return GuiImplPtr(new GuiTextWin(def)); }

}
