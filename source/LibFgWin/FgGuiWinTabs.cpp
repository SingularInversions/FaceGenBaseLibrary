//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     April 12, 2011
//
// Win32 has no way to dynamically change the background color of specific tabs (ie currently selected).
// The only way to do this is to override the default WM_ERASEBKGND and WM_PAINT and draw them yourself.
// At which point you may as well implement your own tabs.

#include "stdafx.h"

#include "FgGuiApiTabs.hpp"
#include "FgGuiWin.hpp"
#include "FgThrowWindows.hpp"
#include "FgMatrixC.hpp"
#include "FgBounds.hpp"
#include "FgDefaultVal.hpp"
#include "FgMetaFormat.hpp"
#include "FgAlgs.hpp"

using namespace std;

struct  FgGuiWinTabs : public FgGuiOsBase
{
    FgGuiApiTabs                    m_api;
    HWND                            m_tabHwnd;
    HWND                            hwndThis;
    FgGuiOsPtrs                     m_panes;
    uint                            m_currPane;
    FgVect2I                        m_client;
    RECT                            m_dispArea;
    FgString                        m_store;

    FgGuiWinTabs(const FgGuiApiTabs & api)
        : m_api(api)
    {
        FGASSERT(m_api.tabs.size()>0);
        for (size_t ii=0; ii<m_api.tabs.size(); ++ii)
            m_panes.push_back(api.tabs[ii].win->getInstance());
        m_currPane = 0;
    }

    virtual void
    create(HWND parentHwnd,int ident,const FgString & store,DWORD extStyle,bool visible)
    {
//fgout << fgnl << "Tabs::create visible: " << visible << " extStyle: " << extStyle << fgpush;
        m_store = store;
        uint        cp;
        if (fgLoadXml(m_store+".xml",cp,false))
            if (cp < m_panes.size())
                m_currPane = cp;
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
        FgVect2UI   max(0);
        for (size_t ii=0; ii<m_panes.size(); ++ii) {
            const FgGuiTab &    tab = m_api.tabs[ii];
            FgVect2UI           pad(tab.padLeft+tab.padRight,tab.padTop+tab.padBottom);
            max = fgMax(max,m_panes[ii]->getMinSize()+pad);
        }
        return max + FgVect2UI(0,37);
    }

    virtual FgVect2B
    wantStretch() const
    {
        for (size_t ii=0; ii<m_panes.size(); ++ii)
            if (m_panes[ii]->wantStretch()[0])
                return FgVect2B(true,true);
        return FgVect2B(false,true);
    }

    virtual void
    updateIfChanged()
    {
//fgout << fgnl << "Tabs::updateIfChanged" << fgpush;
        m_panes[m_currPane]->updateIfChanged();
//fgout << fgpop;
    }

    virtual void
    moveWindow(FgVect2I lo,FgVect2I sz)
    {
//fgout << fgnl << "Tabs::moveWindow " << lo << "," << sz << fgpush;
        MoveWindow(hwndThis,lo[0],lo[1],sz[0],sz[1],FALSE);
//fgout << fgpop;
    }

    virtual void
    showWindow(bool s)
    {
//fgout << fgnl << "Tabs::showWindow: " << s << fgpush;
        ShowWindow(hwndThis,s ? SW_SHOW : SW_HIDE);
//fgout << fgpop;
    }

    virtual void
    saveState()
    {
        fgSaveXml(m_store+".xml",m_currPane,false);
        for (size_t ii=0; ii<m_panes.size(); ++ii)
            m_panes[ii]->saveState();
    }

    LRESULT
    wndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
    {
        if (message == WM_CREATE) {
//fgout << fgnl << "Tabs::WM_CREATE" << fgpush;
            hwndThis = hwnd;
            // Creating the panes before the tabs fixes the problem of trackbars not being visible
            // on first paint (almost ... top/bottom arrows don't appear). No idea why.
            for (size_t ii=0; ii<m_panes.size(); ++ii) {
                // Set visibility here to avoid sending 'ShowWindow' messages, which also
                // send WM_SIZE. The sizing will all be done after creation when ShowWindow
                // is called from the client level.
                m_panes[ii]->create(hwnd,
                    int(ii+1),  // Child identifiers start at 1 since 0 taken above. Not used anyway.
                    m_store+"_"+fgToString(ii),
                    NULL,
                    ii==m_currPane);
            }
            m_tabHwnd = 
                CreateWindowEx(0,
                    WC_TABCONTROL,
                    L"",
                    WS_CHILD | WS_VISIBLE | TCS_FIXEDWIDTH,
                    0,0,0,0,
                    hwnd,
                    0,      // Identifier 0
                    s_fgGuiWin.hinst,
                    NULL);
            TCITEM  tc = {0};
            tc.mask = TCIF_TEXT;
            for (size_t ii=0; ii<m_panes.size(); ++ii) {
                wstring     wstr = m_api.tabs[ii].label.as_wstring();
                wstr += wchar_t(0);
                tc.pszText = &wstr[0];
                TabCtrl_InsertItem(m_tabHwnd,ii,&tc);
            }
            SendMessage(m_tabHwnd,TCM_SETCURSEL,m_currPane,0);
//fgout << fgpop;
            return 0;
        }
        else if (message == WM_SIZE) {
            m_client = FgVect2I(LOWORD(lParam),HIWORD(lParam));
            if (m_client[0] * m_client[1] > 0) {
//fgout << fgnl << "Tabs::WM_SIZE: " << m_api.tabs[0].label << " : " << m_client << fgpush;
                resize(hwnd);
//fgout << fgpop;
            }
            return 0;
        }
        else if (message == WM_NOTIFY) {
            LPNMHDR lpnmhdr = (LPNMHDR)lParam;
            if (lpnmhdr->code == TCN_SELCHANGE) {
                int     idx = int(SendMessage(m_tabHwnd,TCM_GETCURSEL,0,0));
                // This can apparently be -1 for 'no tab selected':
                if ((idx >= 0) && (size_t(idx) < m_panes.size())) {
//fgout << fgnl << "Tabs::WM_NOTIFY: " << idx << fgpush;
                    if (uint(idx) != m_currPane) {
                        m_panes[m_currPane]->showWindow(false);
                        m_currPane = uint(idx);
                        // Must do update check and resize since these are not done when the
                        // tab is not visible:
                        m_panes[m_currPane]->updateIfChanged();
                        resizeCurrPane();
                        m_panes[m_currPane]->showWindow(true);
                        InvalidateRect(hwndThis,NULL,TRUE);
                    }
//fgout << fgpop;
                }
            }
            return 0;
        }
        else if (message == WM_PAINT) {
//fgout << fgnl << "Tabs::WM_PAINT";
        }
        return DefWindowProc(hwnd,message,wParam,lParam);
    }

    void
    resizeCurrPane()
    {
        const FgGuiTab &    tab = m_api.tabs[m_currPane];
        FgVect2I    lo(m_dispArea.left + tab.padLeft, m_dispArea.top + tab.padTop),
                    hi(m_dispArea.right - tab.padRight,m_dispArea.bottom - tab.padBottom),
                    sz = hi - lo;
        m_panes[m_currPane]->moveWindow(lo,sz);
    }

    void
    resize(HWND)
    {
        MoveWindow(m_tabHwnd,0,0,m_client[0],m_client[1],FALSE);
        m_dispArea.left = 0;
        m_dispArea.top = 0;
        m_dispArea.right = m_client[0];
        m_dispArea.bottom = m_client[1];
        SendMessage(m_tabHwnd,
            TCM_ADJUSTRECT,
            NULL,               // Give me the display area for this window area:
            LPARAM(&m_dispArea));
        resizeCurrPane();
    }
};

FgSharedPtr<FgGuiOsBase>
fgGuiGetOsInstance(const FgGuiApiTabs & api)
{return FgSharedPtr<FgGuiOsBase>(new FgGuiWinTabs(api)); }
