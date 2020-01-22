//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
//

#include "stdafx.h"

#include "FgGuiApiText.hpp"
#include "FgGuiWin.hpp"
#include "FgThrowWindows.hpp"

using namespace std;

namespace Fg {

struct  GuiTextEditWin : public GuiBaseImpl
{
    HWND                hwndText = nullptr;
    HWND                hwndThis = nullptr;
    GuiTextEdit         m_api;
    Vec2UI              m_fontDims;
    // Track when keyboard focus is on this edit box so we don't overwrite the user:
    bool                m_keyboardFocus;
    // We have to cache the current text contents to know when a user change has occurred:
    Ustring             m_currText;

    GuiTextEditWin(const GuiTextEdit & api)
        : m_api(api), m_fontDims(16,16), m_keyboardFocus(false)
    {}

    uint
    totalHeight() const
    {return m_fontDims[1]+6; }

    virtual void
    create(HWND parentHwnd,int ident,Ustring const &,DWORD extStyle,bool visible)
    {
        WinCreateChild   cc;
        cc.extStyle = extStyle;
        cc.visible = visible;
        winCreateChild(parentHwnd,ident,this,cc);
    }

    virtual void
    destroy()
    {
        // Automatically destroys children first:
        DestroyWindow(hwndThis);
    }

    virtual Vec2UI
    getMinSize() const
    {return Vec2UI(m_api.minWidth,totalHeight()); }

    virtual Vec2B
    wantStretch() const
    {return Vec2B(m_api.wantStretch,false); }

    virtual void
    updateIfChanged()
    {
        if (!m_keyboardFocus) {
            if (m_api.updateFlag->checkUpdate()) {
                Ustring     txt = m_api.getInput();
                if (txt != m_currText) {
                    m_currText = txt;
                    // Sends WM_COMMAND to *this* window as well as child, then processes
                    // it *before* returning:
                    SetWindowText(hwndText,txt.as_wstring().c_str());
                }
            }
        }
    }

    virtual void
    moveWindow(Vec2I lo,Vec2I sz)
    {MoveWindow(hwndThis,lo[0],lo[1],sz[0],sz[1],FALSE); }

    virtual void
    showWindow(bool s)
    {ShowWindow(hwndThis,s ? SW_SHOW : SW_HIDE); }
    
    LRESULT
    wndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
    {
        if (message == WM_CREATE) {
            hwndThis = hwnd;
            hwndText = CreateWindowEx(0,
                    L"edit",
                    NULL,
                    WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
                    0,0,0,0,            // Will be sent MOVEWINDOW messages.
                    hwnd,
                    HMENU(1),           // Assign identifier 1 to this child window
                    s_guiWin.hinst,
                    NULL);              // No WM_CREATE parameter
            FGASSERTWIN(hwndText != 0);
            TEXTMETRIC      tm;
            GetTextMetrics(GetDC(hwndThis),&tm);
            m_fontDims[0] = tm.tmAveCharWidth;
            m_fontDims[1] = tm.tmHeight + tm.tmExternalLeading;
            m_currText = m_api.getInput();
            m_api.updateFlag->checkUpdate();
            // Sends WM_COMMAND to *this* window as well as child, then processes it
            // *before returning*:
            SetWindowText(hwndText,m_currText.as_wstring().c_str());
            return 0;
        }
        else if (message == WM_SIZE) {  // Sends new size of client area.
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
        else if (message == WM_COMMAND) {
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
                Ustring    txt(&str[0]);
                // Ignore updates from redraw and from 'SetWindowText' (since m_currText is updated
                // before calling those) so that we only call m_api.setOutput() when the user has
                // made actual edits.
                // This is important to avoid random precision changes when internal values are affected
                // by more than one numerical text edit box:
                if (txt != m_currText) {
//fgout << fgnl << "MODIFY TXT: " << txt  << " CURR: " << m_currText << flush;
                    m_currText = txt;
                    m_api.setOutput(txt);
                    if (m_keyboardFocus)        // In this case the change is due to user interaction
                        winUpdateScreen();      // Won't affect this text edit box while focus is on
                }
            }
            else if (nc == EN_SETFOCUS) {
                m_keyboardFocus = true;
            }
            else if (nc == EN_KILLFOCUS) {
                m_keyboardFocus = false;
                Ustring const &    txt = m_api.getInput();
                SetWindowText(hwndText,txt.as_wstring().c_str());
            }
        }
        return DefWindowProc(hwnd,message,wParam,lParam);
    }
};

GuiImplPtr
guiGetOsImpl(const GuiTextEdit & def)
{return GuiImplPtr(new GuiTextEditWin(def)); }

}
