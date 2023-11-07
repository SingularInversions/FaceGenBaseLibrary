//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
//
// Win32 has no way to dynamically change the background color of specific tabs (ie currently selected).
// The only way to do this is to override the default WM_ERASEBKGND and WM_PAINT and draw them yourself.
// At which point you may as well implement your own tabs.

#include "stdafx.h"

#include "FgGuiApi.hpp"
#include "FgGuiWin.hpp"
#include "FgThrowWindows.hpp"
#include "FgMatrixC.hpp"
#include "FgBounds.hpp"

using namespace std;

namespace Fg {

struct  GuiTabsWin : public GuiBaseImpl
{
    GuiTabs             m_api;
    HWND                m_tabHwnd;
    HWND                hwndThis;
    GuiImplPtrs         m_panes;
    uint                m_currPane;
    Vec2I               m_client;
    RECT                m_dispArea;
    String8             m_store;

    GuiTabsWin(GuiTabs const & api) : m_api{api}
    {
        FGASSERT(!m_api.tabs.empty());
        for (GuiTabDef const & tabDef : m_api.tabs) {
//fgout << fgnl << tabDef.label << fgpush;
            FGASSERT(tabDef.win);
            m_panes.push_back(tabDef.win->getInstance());
//fgout << fgpop;
        }
        m_currPane = 0;
    }

    ~GuiTabsWin()
    {
        // only save when program closes, not when tab is destroyed (which happens every tab change):
        if (!m_store.empty()) {
            try {saveRaw(srlzText(m_currPane),m_store+"currPane.txt"); }
            catch(...) {}
        }
    }

    virtual void        create(HWND parentHwnd,int ident,String8 const & store,DWORD extStyle,bool visible)
    {
//fgout << fgnl << "Tabs::create visible: " << visible << " extStyle: " << extStyle << fgpush;
        // only want to load from file the first time this window is created, then we just remember the user's setting.
        // can't do this in ctor since we don't yet have 'store':
        if (m_store.empty()) {
            if (!store.empty()) {
                try {
                    uint            cp = dsrlzText<uint>(loadRawString(store+"currPane.txt"));
                    if (cp < m_panes.size())
                        m_currPane = cp;
                }
                catch (...) {}
            }
        }
        m_store = store;
        WinCreateChild      cc;
        cc.extStyle = extStyle;
        cc.visible = visible;
        // Without this, tab outline shadowing can disappear:
        cc.useFillBrush = true;
        winCreateChild(parentHwnd,ident,this,cc);
//fgout << fgpop;
    }

    virtual void        destroy()
    {
        // Automatically destroys children first:
        DestroyWindow(hwndThis);
    }

    virtual Vec2UI      getMinSize() const
    {
        Vec2UI              max {0};
        FGASSERT(m_panes.size() == m_api.tabs.size());
        for (size_t ii=0; ii<m_panes.size(); ++ii) {
            GuiTabDef const &   tab = m_api.tabs[ii];
//fgout << fgpush;
            Vec2UI              pad {tab.padLeft+tab.padRight,tab.padTop+tab.padBottom},
                                min = m_panes[ii]->getMinSize();
//fgout << fgpop;
//fgout << fgnl << tab.label << " " << min;
            max = mapMax(max,min+pad);
        }
        return max + Vec2UI{0,37};
    }

    virtual Vec2B       wantStretch() const
    {
        for (size_t ii=0; ii<m_panes.size(); ++ii)
            if (m_panes[ii]->wantStretch()[0])
                return Vec2B(true,true);
        return Vec2B(false,true);
    }

    virtual void        updateIfChanged()
    {
//fgout << fgnl << "Tabs::updateIfChanged" << fgpush;
        m_panes[m_currPane]->updateIfChanged();
//fgout << fgpop;
    }

    virtual void        moveWindow(Vec2I lo,Vec2I sz)
    {
//fgout << fgnl << "Tabs::moveWindow " << lo << "," << sz << fgpush;
        MoveWindow(hwndThis,lo[0],lo[1],sz[0],sz[1],FALSE);
//fgout << fgpop;
    }

    virtual void        showWindow(bool s)
    {
//fgout << fgnl << "Tabs::showWindow: " << s << fgpush;
        ShowWindow(hwndThis,s ? SW_SHOW : SW_HIDE);
//fgout << fgpop;
    }

    LRESULT             wndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
    {
        if (msg == WM_CREATE) {
//fgout << fgnl << "Tabs::WM_CREATE" << fgpush;
            hwndThis = hwnd;
            // Creating the panes before the tabs fixes the problem of trackbars not being visible
            // on first paint (almost ... top/bottom arrows don't appear). No idea why.
            // Used to create all tab windows here and turn visibility on and off with selection but as of
            // Windows 10 this approach causes huge latency when moving main window so now we create/destroy
            // each time tab is changed. Used to be the case that WM_SIZE was not sent non-visible windows,
            // but either that is not the case any more or MS has managed to do something even stupider ...
            FGASSERT(m_currPane < m_panes.size());
            m_panes[m_currPane]->create(hwnd,
                int(m_currPane+1),  // Child identifiers start at 1 since 0 taken above. Not used anyway.
                m_store+"_"+toStr(m_currPane),
                NULL,
                true);
            m_tabHwnd = 
                CreateWindowEx(0,
                    WC_TABCONTROL,
                    L"",
                    WS_CHILD | WS_VISIBLE,
                    0,0,0,0,
                    hwnd,
                    0,      // Identifier 0
                    s_guiWin.hinst,
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
        else if (msg == WM_SIZE) {
            m_client = Vec2I(LOWORD(lParam),HIWORD(lParam));
            if (m_client[0] * m_client[1] > 0) {
//fgout << fgnl << "Tabs::WM_SIZE: " << m_api.tabs[0].label << " : " << m_client << fgpush;
                resize(hwnd);
//fgout << fgpop;
            }
            return 0;
        }
        else if (msg == WM_NOTIFY) {
            LPNMHDR lpnmhdr = (LPNMHDR)lParam;
            if (lpnmhdr->code == TCN_SELCHANGE) {
                int     idx = int(SendMessage(m_tabHwnd,TCM_GETCURSEL,0,0));
                // This can apparently be -1 for 'no tab selected':
                if ((idx >= 0) && (size_t(idx) < m_panes.size())) {
//fgout << fgnl << "Tabs::WM_NOTIFY: " << idx << fgpush;
                    if (uint(idx) != m_currPane) {
                        m_panes[m_currPane]->destroy();
                        m_currPane = uint(idx);
                        String8             subStore = m_store + "_" + toStr(m_currPane);
                        m_panes[m_currPane]->create(hwnd,int(m_currPane)+1,subStore,NULL,true);
                        resizeCurrPane();                           // Always required after creation
                        m_panes[m_currPane]->updateIfChanged();     // Required to update win32 state (eg. sliders)
                        InvalidateRect(hwndThis,NULL,TRUE);         // Tested to be necessary
                    }
//fgout << fgpop;
                }
            }
            return 0;
        }
        else if (msg == WM_PAINT) {
//fgout << fgnl << "Tabs::WM_PAINT";
        }
        return DefWindowProc(hwnd,msg,wParam,lParam);
    }

    void                resizeCurrPane()
    {
        GuiTabDef const &   tab = m_api.tabs[m_currPane];
        Vec2I               lo (m_dispArea.left + tab.padLeft, m_dispArea.top + tab.padTop),
                            hi (m_dispArea.right - tab.padRight,m_dispArea.bottom - tab.padBottom),
                            sz = hi - lo;
        m_panes[m_currPane]->moveWindow(lo,sz);
    }

    void                resize(HWND)
    {
        // The repaint TRUE argument is only necessary when going from maximized to normal window size,
        // for some reason the tabs are repainted anyway in other situations:
        MoveWindow(m_tabHwnd,0,0,m_client[0],m_client[1],TRUE);
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

GuiImplPtr          guiGetOsImpl(const GuiTabs & api) {return GuiImplPtr(new GuiTabsWin(api)); }

}
