//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
//

#include "stdafx.h"

#include "FgGuiApiRadio.hpp"
#include "FgGuiWin.hpp"
#include "FgThrowWindows.hpp"

using namespace std;

namespace Fg {

static Vec2UI    s_pad(40,12);

struct  GuiRadioWin : public GuiBaseImpl
{
    GuiRadio            m_api;
    DfgFPtr             updateFlag;
    HWND                hwndThis;
    vector<HWND>        m_hwnds;
    vector<Vec2UI>   m_sizes;

    GuiRadioWin(const GuiRadio & api)
    : m_api(api), updateFlag(makeUpdateFlag(m_api.selection))
    {}

    virtual void
    create(HWND parentHwnd,int ident,Ustring const &,DWORD extStyle,bool visible)
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
        uint        dimAcc = (m_api.horiz ? 0 : 1),
                    dimMax = 1 - dimAcc;
        for (size_t ii=0; ii<m_sizes.size(); ++ii) {
            setIfGreater(sz[dimMax],m_sizes[ii][dimMax]);
            sz[dimAcc] += m_sizes[ii][dimAcc] + s_pad[dimAcc];
        }
        sz[dimMax] += s_pad[dimMax];
        return sz;
    }

    virtual Vec2B
    wantStretch() const
    {return Vec2B(false,false); }

    virtual void
    updateIfChanged()
    {
        if (updateFlag->checkUpdate())
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
                    uint            dimAcc = (m_api.horiz ? 0 : 1),
                                    dimMax = 1 - dimAcc;
                    sz[dimMax] = szTot[dimMax];
                    for (uint ii=0; ii<m_hwnds.size(); ++ii) {
                        sz[dimAcc] = m_sizes[ii][dimAcc] + s_pad[dimAcc];
                        MoveWindow(m_hwnds[ii],pos[0],pos[1],sz[0],sz[1],TRUE);
                        pos[dimAcc] += sz[dimAcc];
                    }
                    return 0;
                }
            case WM_COMMAND:
                {
                    WORD    ident = LOWORD(wParam);
                    WORD    code = HIWORD(wParam);
                    if (code == 0) {    // radio box clicked
//fgout << fgnl << "GuiRadioWin::WM_COMMAND clicked " << m_api.labels[0];
                        size_t      sel = ident;
                        if (sel >= m_api.labels.size())
                            sel = 0;
//fgout << " val: " << sel << " m_api.selection " << m_api.selection;
                        m_api.selection.set(sel);
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
        size_t              val = m_api.selection.val();
        if (val >= m_api.labels.size())
            val = 0;
        for (size_t ii=0; ii<m_hwnds.size(); ++ii) {
            if (ii == val)
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
