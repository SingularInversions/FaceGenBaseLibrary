//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
//

#include "stdafx.h"

#include "FgGuiApiSplit.hpp"
#include "FgGuiWin.hpp"
#include "FgThrowWindows.hpp"
#include "FgMatrixC.hpp"
#include "FgBounds.hpp"
#include "FgMetaFormat.hpp"

using namespace std;

// Debug:
ostream &
operator<<(ostream & os,const SCROLLINFO & si)
{
    return os << "min: " << si.nMin
        << " max: " << si.nMax
        << " page: " << si.nPage
        << " pos: " << si.nPos
        << " trackPos " << si.nTrackPos;
}

namespace Fg {

struct  GuiSplitScrollWin : public GuiBaseImpl
{
    GuiSplitScroll              m_api;
    HWND                        hwndThis;
    GuiImplPtrs                 m_panes;
    // Cache current visibility of panes to avoid excessive update calls:
    vector<FatBool>              m_panesVisible;
    Vec2I                    m_client;   // doesn't include slider
    String8                    m_store;
    SCROLLINFO                  m_si;

    GuiSplitScrollWin(const GuiSplitScroll & api) :
        m_api(api), hwndThis(0)
    {
        m_si.cbSize = sizeof(m_si);     // Never changes
        m_si.nMin = 0;                  // Never changes
        m_si.nPos = 0;                  // Initial value
        m_si.nTrackPos = 0;             // This value is always the same as the above value.
    }

    virtual void
    create(HWND parentHwnd,int ident,String8 const & store,DWORD extStyle,bool visible)
    {
//fgout << fgnl << "SplitScroll::create: visible: " << visible << " extStyle: " << extStyle << " ident: " << ident << fgpush;
        m_store = store;
        WinCreateChild   cc;
        cc.extStyle = extStyle;
        cc.style = WS_VSCROLL;
        cc.visible = visible;
        winCreateChild(parentHwnd,ident,this,cc);
//fgout << fgpop;
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
        // Set the minimum scrollable size as the smaller of the maximum element
        // and a fixed maximum min:
        Vec2UI   ret = m_api.minSize;
        // Add scroll bar width:
        ret[0] += GetSystemMetrics(SM_CXVSCROLL);
        return ret;
    }

    virtual Vec2B
    wantStretch() const
    {
        for (size_t ii=0; ii<m_panes.size(); ++ii)
            if (m_panes[ii]->wantStretch()[0])
                return Vec2B(true,true);
        return Vec2B(false,true);
    }

    virtual void
    updateIfChanged()
    {
//fgout << fgnl << "SplitScroll::updateIfChanged";
        if (m_api.updateFlag->checkUpdate()) {
//fgout << " ... updating" << fgpush;
            // call DestroyWindow in all created sub-windows:
            for (size_t ii=0; ii<m_panes.size(); ++ii)
                m_panes[ii]->destroy();
            GuiPtrs            panes = m_api.getPanes();
            m_panes.resize(panes.size());
            m_panesVisible.resize(panes.size());
            for (size_t ii=0; ii<m_panes.size(); ++ii) {
                m_panes[ii] = panes[ii]->getInstance();
                m_panes[ii]->create(hwndThis,int(ii),m_store+"_"+toStr(ii),0UL,false);
                m_panesVisible[ii] = false;
            }
            resize();   // New windows must be sent a size
//fgout << fgpop;
        }
        for (size_t ii=0; ii<m_panes.size(); ++ii)
            if (m_panesVisible[ii])
                m_panes[ii]->updateIfChanged();
    }

    virtual void
    moveWindow(Vec2I lo,Vec2I sz)
    {
//fgout << fgnl << "SplitScroll::moveWindow: " << lo << "," << sz << fgpush;
        MoveWindow(hwndThis,lo[0],lo[1],sz[0],sz[1],FALSE);
//fgout << fgpop;
    }

    virtual void
    showWindow(bool s)
    {
//fgout << fgnl << "SplitScroll::showWindow: " << s << fgpush;
        ShowWindow(hwndThis,s ? SW_SHOW : SW_HIDE);
//fgout << fgpop;
    }

    LRESULT
    wndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
    {
        if (msg == WM_CREATE) {
//fgout << fgnl << "SplitScroll::WM_CREATE" << fgpush;
            hwndThis = hwnd;
            GuiPtrs   panes = m_api.getPanes();
            m_panes.resize(panes.size());
            m_panesVisible.resize(panes.size());
            for (size_t ii=0; ii<m_panes.size(); ++ii) {
                m_panes[ii] = panes[ii]->getInstance();
                m_panes[ii]->create(hwndThis,int(ii),m_store+"_"+toStr(ii),0UL,false);
                m_panesVisible[ii] = false;
            }
            m_api.updateFlag->checkUpdate();
//fgout << fgpop;
        }
        else if (msg == WM_SIZE) {
            m_client = Vec2I(LOWORD(lParam),HIWORD(lParam));
            if (m_client[0] * m_client[1] > 0) {
//fgout << fgnl << "SplitScroll::WM_SIZE: " << m_client << fgpush;
                resize();
//fgout << fgpop;
            }
        }
        else if (msg == WM_VSCROLL) {
//fgout << "SplitScroll::WM_VSCROLL";
            int     tmp = m_si.nPos;
            // Get the current state, esp. trackbar drag position:
            m_si.fMask = SIF_ALL;
            GetScrollInfo(hwnd,SB_VERT,&m_si);
            int     wprml = LOWORD(wParam);
            if (wprml == SB_TOP)
                m_si.nPos = m_si.nMin;
            else if (wprml == SB_BOTTOM)
                m_si.nPos = m_si.nMax;
            else if (wprml == SB_LINEUP)
                m_si.nPos -= 20;
            else if (wprml == SB_LINEDOWN)
                m_si.nPos += 20;
            else if (wprml == SB_PAGEUP)
                m_si.nPos -= m_client[1];
            else if (wprml == SB_PAGEDOWN)
                m_si.nPos += m_client[1];
            else if (wprml == SB_THUMBTRACK)
                m_si.nPos = m_si.nTrackPos;
            m_si.fMask = SIF_POS;
            SetScrollInfo(hwnd,SB_VERT,&m_si,TRUE);
            // Windows may clamp the position:
            GetScrollInfo(hwnd,SB_VERT,&m_si);
            if (m_si.nPos != tmp) {
                resize();
                InvalidateRect(hwndThis,NULL,TRUE);
            }
        }
        else if (msg == WM_MOUSEWHEEL) {
            short zDelta = (short)HIWORD(wParam);
            if (zDelta < 0)
                SendMessage(hwnd,WM_VSCROLL,SB_LINEDOWN,NULL);
            else
                SendMessage(hwnd,WM_VSCROLL,SB_LINEUP,NULL);
        }
//        else if (msg == WM_PAINT) {
//fgout << fgnl << "SplitScroll::WM_PAINT";
//        }
        else
            return DefWindowProc(hwnd,msg,wParam,lParam);
        return 0;
    }

    void
    resize()
    {
        // No point in doing this before we have the client size (ie at first construction):
        if (m_client[1] > 0) {
            Vec2I    pos(0),
                        sz = m_client;
            sz[0] -= 5;     // Leave space between content and slider
            pos[1] = int(m_api.spacing) - m_si.nPos;
            for (size_t ii=0; ii<m_panes.size(); ++ii) {
                sz[1] = m_panes[ii]->getMinSize()[1];
                if ((pos[1] > m_client[1]) || (pos[1]+sz[1] < 0)) {
                    m_panes[ii]->showWindow(false);
                    m_panesVisible[ii] = false;
                }
                else {
                    m_panes[ii]->moveWindow(pos,sz);
                    // Wasn't updated if not visible during previous changes:
                    if (m_panesVisible[ii] == false)
                        m_panes[ii]->updateIfChanged();
                    m_panes[ii]->showWindow(true);
                    m_panesVisible[ii] = true;
                }
                pos[1] += sz[1] + m_api.spacing;
            }
            // Note that Windows wants the total range of the scrollable area,
            // not the effective slider range resulting from subtracting the 
            // currently displayed range:
            m_si.fMask = SIF_DISABLENOSCROLL | SIF_PAGE | SIF_POS | SIF_RANGE;
            uint        totalHeight = 0;
            for (size_t ii=0; ii<m_panes.size(); ++ii)
                totalHeight += m_panes[ii]->getMinSize()[1] + m_api.spacing;
            m_si.nMax = totalHeight;
            m_si.nPage = m_client[1];
            // Windows will clamp the position and otherwise adjust:
            SetScrollInfo(hwndThis,SB_VERT,&m_si,TRUE);
            m_si.fMask = SIF_ALL;
            GetScrollInfo(hwndThis,SB_VERT,&m_si);
        }
    }
};

GuiImplPtr
guiGetOsImpl(const GuiSplitScroll & def)
{return GuiImplPtr(new GuiSplitScrollWin(def)); }

}
