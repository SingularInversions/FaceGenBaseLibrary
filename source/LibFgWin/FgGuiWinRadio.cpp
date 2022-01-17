//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
//

#include "stdafx.h"

#include "FgGuiApiRadio.hpp"
#include "FgGuiWin.hpp"
#include "FgThrowWindows.hpp"
#include "FgBounds.hpp"

using namespace std;

namespace Fg {

static Vec2UI    s_pad(40,12);

struct  GuiRadioWin : public GuiBaseImpl
{
    GuiRadio            m_api;
    uint                currVal;
    HWND                hwndThis;
    vector<HWND>        m_hwnds;
    vector<Vec2UI>      m_sizes;

    GuiRadioWin(const GuiRadio & api)
        : m_api{api}, currVal{m_api.getFn()}
    {}

    virtual void
    create(HWND parentHwnd,int ident,String8 const &,DWORD extStyle,bool visible)
    {
//fgout << fgnl << "GuiRadioWin::create " << m_api.labels[0];
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
    {
        Vec2UI   sz;
        for (size_t ii=0; ii<m_sizes.size(); ++ii) {
            updateMax_(sz[0],m_sizes[ii][0]);
            sz[1] += m_sizes[ii][1] + s_pad[1];
        }
        sz[0] += s_pad[0];
        return sz;
    }

    virtual Vec2B
    wantStretch() const
    {return Vec2B(false,false); }

    virtual void
    updateIfChanged()
    {
        if (currVal != m_api.getFn())
            updateRadio();
    }

    virtual void
    moveWindow(Vec2I lo,Vec2I sz)
    {
//fgout << fgnl << "GuiRadioWin::moveWindow " << lo << " , " << sz;
        MoveWindow(hwndThis,lo[0],lo[1],sz[0],sz[1],FALSE);
    }

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
//fgout << fgnl << "GuiRadioWin::WM_CREATE " << m_api.labels[0];
                    hwndThis = hwnd;
                    m_sizes.resize(m_api.labels.size());
                    m_hwnds.resize(m_api.labels.size());
                    for (size_t ii=0; ii<m_api.labels.size(); ++ii) {
                        wstring     wstr = m_api.labels[ii].as_wstring();
                        m_hwnds[ii] =
                            CreateWindowEx(0,
                                TEXT("button"),
                                wstr.c_str(),
                                WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON,
                                0,0,0,0,            // Generates WM_SIZE 0 message
                                hwnd,
                                HMENU(ii),
                                s_guiWin.hinst,
                                NULL);              // No WM_CREATE parameter
                        FGASSERTWIN(m_hwnds[ii] != 0);
                        // I couldn't get the 'Button_GetIdealSize' macro to work; here, in WM_SIZE,
                        // with GuiButtonWin, with starting size arguments, whatever. This approach
                        // is more universal than just buttons anyway.
                        // Still only an approximate I think ... docs are complex:
                        SIZE        sz;
                        GetTextExtentPoint32(GetDC(hwndThis),wstr.c_str(),int(wstr.size()),&sz);
                        m_sizes[ii] = Vec2UI(sz.cx,sz.cy);
                    }
                    updateRadio();
                    return 0;
                }
            case WM_SIZE:
                {
//fgout << fgnl << "GuiRadioWin::WM_SIZE " << m_api.labels[0];
                    // Don't care what size params are since these controls stick to min size,
                    // but we still need to ignore Windows' initial zero-size message:
                    if (LOWORD(lParam) * HIWORD(lParam) == 0)
                        return 0;
                    Vec2UI       szTot = getMinSize(),
                                    pos,sz;
                    sz[0] = szTot[0];
                    for (uint ii=0; ii<m_hwnds.size(); ++ii) {
                        sz[1] = m_sizes[ii][1] + s_pad[1];
                        MoveWindow(m_hwnds[ii],pos[0],pos[1],sz[0],sz[1],TRUE);
                        pos[1] += sz[1];
                    }
                    return 0;
                }
            case WM_COMMAND:
                {
                    WORD            ident = LOWORD(wParam);
                    WORD            code = HIWORD(wParam);
                    if (code == 0) {    // radio box clicked
//fgout << fgnl << "GuiRadioWin::WM_COMMAND clicked " << m_api.labels[0];
                        uint            sel = ident;
                        if (sel >= m_api.labels.size())
                            sel = 0;
//fgout << " val: " << sel << " m_api.selection " << m_api.selection;
                        m_api.setFn(sel);
//fgout << fgnl << "Update screen: " << flush << fgpush;
                        winUpdateScreen();
//fgout << fgpop << fgnl << "Updated " << flush;
                    }
                    return 0;
                }
        }
        return DefWindowProc(hwnd,message,wParam,lParam);
    }

    void
    updateRadio()
    {
        currVal = m_api.getFn();
        for (size_t ii=0; ii<m_hwnds.size(); ++ii) {
            if (ii == currVal)
                SendMessage(m_hwnds[ii],BM_SETCHECK,BST_CHECKED,0);
            else
                SendMessage(m_hwnds[ii],BM_SETCHECK,BST_UNCHECKED,0);
        }
    }
};

GuiImplPtr
guiGetOsImpl(const GuiRadio & def)
{return GuiImplPtr(new GuiRadioWin(def)); }

}
