//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
//

#include "stdafx.h"

#include "FgGuiApiButton.hpp"
#include "FgGuiWin.hpp"
#include "FgThrowWindows.hpp"
#include "FgMatrixC.hpp"

using namespace std;

namespace Fg {

struct  GuiButtonWin : public GuiBaseImpl
{
    GuiButton           m_api;
    HWND                hwndButton,
                        hwndThis;

    GuiButtonWin(const GuiButton & api) : m_api(api) {}

    virtual void        create(HWND parentHwnd,int ident,String8 const &,DWORD extStyle,bool visible)
    {
//fgout << fgnl << "Button::create visible: " << visible << " extStyle: " << extStyle << fgpush;
        // We need a parent window in order to catch the button command messages:
        WinCreateChild   cc;
        cc.extStyle = extStyle;
        cc.visible = visible;
        winCreateChild(parentHwnd,ident,this,cc);
//fgout << fgpop;
    }
    virtual void        destroy()
    {
        DestroyWindow(hwndThis);        // Automatically destroys children first
    }
    virtual Vec2UI      getMinSize() const {return Vec2UI(100,24); }
    virtual Vec2B       wantStretch() const {return Vec2B(false,false); }
    virtual void        updateIfChanged()
    {
//fgout << fgnl << "Button::updateIfChanged";
    }
    virtual void        moveWindow(Vec2I lo,Vec2I sz)
    {
//fgout << fgnl << "Button::moveWindow: " << lo << "," << sz;
        MoveWindow(hwndThis,lo[0],lo[1],sz[0],sz[1],FALSE);
    }
    virtual void        showWindow(bool s)
    {
//fgout << fgnl << "Button::showWindow: " << s;
        ShowWindow(hwndThis,s ? SW_SHOW : SW_HIDE);
    }
    LRESULT             wndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
    {
        if (msg == WM_CREATE) {
//fgout << fgnl << "Button::WM_CREATE";
            // This is the first place we get the hwnd for this instance since this callback
            // happens before 'winCreateChild' above returns:
            hwndThis = hwnd;
            hwndButton = CreateWindowEx(0,
                TEXT("button"),     // Standard controls class name for all buttons
                m_api.label.as_wstring().c_str(),
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                0,0,0,0,            // Will be sent MOVEWINDOW messages.
                hwnd,
                HMENU(0),
                s_guiWin.hinst,
                NULL);              // No WM_CREATE parameter
            FGASSERTWIN(hwndButton != 0);
        }
        else if (msg == WM_SIZE) {
            int             wid = LOWORD(lParam);
            int             hgt = HIWORD(lParam);
            if (wid*hgt > 0) {
//fgout << fgnl << "Button::WM_SIZE: " << wid << "," << hgt;
                int             buttonHgt = cMin(int(getMinSize()[1]),hgt),
                                vspace = hgt - buttonHgt;
                MoveWindow(hwndButton,0,vspace/2,wid,buttonHgt,TRUE);
            }
        }
        else if (msg == WM_COMMAND) {
            WORD            ident = LOWORD(wParam);
            WORD            code = HIWORD(wParam);
            if (code == 0) {
                FGASSERT(ident == 0);
                m_api.action();
                winUpdateScreen();
            }
        }
//case WM_PAINT:
//fgout << fgnl << "Button::WM_PAINT";
        else
            return DefWindowProc(hwnd,msg,wParam,lParam);
        return 0;
    }
};

GuiImplPtr guiGetOsImpl(const GuiButton & def) {return GuiImplPtr(new GuiButtonWin(def)); }

}
