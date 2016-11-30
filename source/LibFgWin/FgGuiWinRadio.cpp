//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     April 25, 2011
//

#include "stdafx.h"

#include "FgGuiApiRadio.hpp"
#include "FgGuiWin.hpp"
#include "FgThrowWindows.hpp"

using namespace std;

static FgVect2UI    s_pad(40,12);

struct  FgGuiWinRadio : public FgGuiOsBase
{
    FgGuiApiRadio       m_api;
    HWND                hwndThis;
    vector<HWND>        m_hwnds;
    vector<FgVect2UI>   m_sizes;

    FgGuiWinRadio(const FgGuiApiRadio & api)
    : m_api(api)
    {}

    virtual void
    create(HWND parentHwnd,int ident,const FgString &,DWORD extStyle,bool visible)
    {
//fgout << fgnl << "FgGuiWinRadio::create " << m_api.labels[0];
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
    {
        FgVect2UI   sz;
        uint        dimAcc = (m_api.horiz ? 0 : 1),
                    dimMax = 1 - dimAcc;
        for (size_t ii=0; ii<m_sizes.size(); ++ii) {
            fgSetIfGreater(sz[dimMax],m_sizes[ii][dimMax]);
            sz[dimAcc] += m_sizes[ii][dimAcc] + s_pad[dimAcc];
        }
        sz[dimMax] += s_pad[dimMax];
        return sz;
    }

    virtual FgVect2B
    wantStretch() const
    {return FgVect2B(false,false); }

    virtual void
    updateIfChanged()
    {
        if (g_gg.dg.update(m_api.updateIdx))
            updateRadio();
    }

    virtual void
    moveWindow(FgVect2I lo,FgVect2I sz)
    {
//fgout << fgnl << "FgGuiWinRadio::moveWindow " << lo << " , " << sz;
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
//fgout << fgnl << "FgGuiWinRadio::WM_CREATE " << m_api.labels[0];
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
                                s_fgGuiWin.hinst,
                                NULL);              // No WM_CREATE parameter
                        FGASSERTWIN(m_hwnds[ii] != 0);
                        // I couldn't get the 'Button_GetIdealSize' macro to work; here, in WM_SIZE,
                        // with FgGuiWinButton, with starting size arguments, whatever. This approach
                        // is more universal than just buttons anyway.
                        // Still only an approximate I think ... docs are complex:
                        SIZE        sz;
                        GetTextExtentPoint32(GetDC(hwndThis),wstr.c_str(),int(wstr.length()),&sz);
                        m_sizes[ii] = FgVect2UI(sz.cx,sz.cy);
                    }
                    updateRadio();
                    return 0;
                }
            case WM_SIZE:
                {
//fgout << fgnl << "FgGuiWinRadio::WM_SIZE " << m_api.labels[0];
                    // Don't care what size params are since these controls stick to min size,
                    // but we still need to ignore Windows' initial zero-size message:
                    if (LOWORD(lParam) * HIWORD(lParam) == 0)
                        return 0;
                    FgVect2UI       szTot = getMinSize(),
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
//fgout << fgnl << "FgGuiWinRadio::WM_COMMAND clicked " << m_api.labels[0];
                        size_t      sel = ident;
                        if (sel >= m_api.vals.size())
                            sel = 0;
//fgout << " val: " << sel << " m_api.selection " << m_api.selection;
                        g_gg.setVal(m_api.selection,m_api.vals[sel]);
//fgout << fgnl << "Update screen: " << flush << fgpush;
                        g_gg.updateScreen();
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
        const FgString &    sel = g_gg.getVal(m_api.selection);
        size_t              val = fgFindFirstIdx(m_api.vals,sel);
        if (val >= m_api.vals.size())
            val = 0;
        for (size_t ii=0; ii<m_hwnds.size(); ++ii) {
            if (ii == val)
                SendMessage(m_hwnds[ii],BM_SETCHECK,BST_CHECKED,0);
            else
                SendMessage(m_hwnds[ii],BM_SETCHECK,BST_UNCHECKED,0);
        }
    }
};

FgPtr<FgGuiOsBase>
fgGuiGetOsInstance(const FgGuiApiRadio & def)
{return FgPtr<FgGuiOsBase>(new FgGuiWinRadio(def)); }
