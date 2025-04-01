//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
//

#include "stdafx.h"

#include "FgGuiApi.hpp"
#include "FgGuiWin.hpp"
#include "FgThrowWindows.hpp"
#include "FgMatrixC.hpp"
#include "FgBounds.hpp"
#include "FgBounds.hpp"

using namespace std;

namespace Fg {

Uints               cRowMinHeights(Img<Vec2UI> const & minSizes)
{
        auto                cMinRowHgt = [&](size_t row) -> uint
        {
            uint                ret {0},
                                rr = scast<uint>(row);
            for (uint cc=0; cc<minSizes.width(); ++cc)
                updateMax_(ret,minSizes.xy(cc,rr)[1]);
            return ret;
        };
        return genSvec(minSizes.height(),cMinRowHgt);
}

Uints               cColMinWidths(Img<Vec2UI> const & minSizes)
{
        auto                cMinColWid = [&](size_t col) -> uint
        {
            uint                ret {0},
                                cc = scast<uint>(col);
            for (uint rr=0; rr<minSizes.height(); ++rr)
                updateMax_(ret,minSizes.xy(cc,rr)[0]);
            return ret;
        };
        return genSvec(minSizes.width(),cMinColWid);
}

Bools               cRowStrets(Img<Arr2B> const & wantStrets)
{
    auto                cRowStret = [&](size_t row)
    {
        uint                rr = scast<uint>(row);
        for (uint cc=0; cc<wantStrets.width(); ++cc)
            if (wantStrets.xy(cc,rr)[1])
                return true;
        return false;
    };
    return genSvec(wantStrets.height(),cRowStret);
};

Bools               cColStrets(Img<Arr2B> const & wantStrets)
{
    auto                cColStret = [&](size_t col)
    {
        uint                cc = scast<uint>(col);
        for (uint rr=0; rr<wantStrets.height(); ++rr)
            if (wantStrets.xy(cc,rr)[0])
                return true;
        return false;
    };
    return genSvec(wantStrets.width(),cColStret);
};

uint                calcPadBetween(Uints const & maxDims)
{
    uint            maxMin(0);
    if (maxDims.size() > 0)
        maxMin = cMaxElem(maxDims);
    return cMin(8U,scast<uint>(maxMin*0.1));
}

uint                calcPadTotal(uint numPanes,uint padBetween)
{
    uint            gaps = cMax(numPanes,1U) - 1U;
    return padBetween * gaps;
}

// fixed version with no 'padBetween', which I now figure should be in the leaf nodes:
Uints               stretchSizes(Uints const & minSizes,Bools const & paneStrets,uint pixSize)
{
    FGASSERT(minSizes.size() == paneStrets.size());
    uint                numStret = cSum(mapCall(paneStrets,[](bool s){return s ? 1UL : 0UL;})),
                        minSize = cSum(minSizes),
                        extra = (pixSize > minSize) ? (pixSize-minSize) : 0,
                        pad = (numStret == 0) ? 0 : extra / numStret;
    auto                fn = [pad](uint m,bool s)
    {
        if (s)
            return m + pad;
        else
            return m;
    };
    return mapCall(minSizes,paneStrets,fn);
}

struct  GuiSplitWin : public GuiBaseImpl
{
    GuiSplit                m_api;              // m_api.panes can be zero size
    Img<GuiImplPtr>         m_panes;            // 1-1 with m_api.panes
    String8                 m_store;
    HWND                    m_hwndParent;
    Vec2I                   m_base {0};         // upper left position of client area (since this class is not a Windows object)
    Vec2I                   m_size {0};         // size of client area

    explicit GuiSplitWin(GuiSplit const & api) : m_api(api)
    {
        auto                    fn = [](GuiPtr const & gp)
        {
            FGASSERT(gp);
            return gp->getInstance();
        };
        m_panes = mapCall(api.panes,fn);
    }

    virtual void        create(HWND parentHwnd,int,String8 const & store,DWORD /*extStyle*/,bool visible)
    {
//fgout << fgnl << "GuiSplitWin::create" << fgpush;
        // Since this is not a win32 window, we create the sub-windows here within the parent WM_CREATE handler.
        // Ignore extStyle since this isn't a win32 window and it's not recursively passed.
        m_store = store;
        m_hwndParent = parentHwnd;
        for (size_t ii=0; ii<m_panes.m_data.size(); ++ii)
            m_panes.m_data[ii]->create(m_hwndParent,int(ii),m_store+"_"+toStr(ii),NULL,visible);
//fgout << fgpop;
    }

    virtual void        destroy()
    {
        for (GuiImplPtr const & ptr : m_panes.m_data)
            ptr->destroy();
    }

    virtual Vec2UI      getMinSize() const      // handles the empty case:
    {
        Img<Vec2UI>         mins = mapCall(m_panes,[](GuiImplPtr const & p){return p->getMinSize();});
        Uints               colMinWidths = cColMinWidths(mins),
                            rowMinHeights = cRowMinHeights(mins);
        Vec2UI              minSize {cSum(colMinWidths),cSum(rowMinHeights)};
        return minSize + m_api.gapTotal();
    }

    virtual Arr2B       wantStretch() const
    {
        Arr2B           ret(false,false);
        for (GuiImplPtr const & ptr : m_panes.m_data)
            ret = mapOr(ret,ptr->wantStretch());
        return ret;
    }

    virtual void        updateIfChanged()
    {
        for (GuiImplPtr const & ptr : m_panes.m_data)
            ptr->updateIfChanged();
    }

    virtual void        moveWindow(Vec2I base,Vec2I size)
    {
//fgout << fgnl << "GuiSplitWin::moveWindow " << base << " , " << size << fgpush;
        m_base = base;
        m_size = size;
        resize();
//fgout << fgpop;
    }

    virtual void        showWindow(bool s)
    {
        for (GuiImplPtr const & ptr : m_panes.m_data)
            ptr->showWindow(s);
    }

    void                resize()
    {
        if (m_size.elemsProduct() > 0) {
            Img<Vec2UI>         mins = mapCall(m_panes,[](GuiImplPtr const & p){return p->getMinSize();});
            Img<Arr2B>          strets = mapCall(m_panes,[](GuiImplPtr const & p){return p->wantStretch();});
            Vec2UI              gapTotal = m_api.gapTotal(),
                                extra = mapMax(mapCast<uint>(m_size) - gapTotal,uint{0});
            Bools               colStrets = cColStrets(strets),
                                rowStrets = cRowStrets(strets);
            Uints               colMinWidths = cColMinWidths(mins),
                                rowMinHeights = cRowMinHeights(mins);
            Uints               colWidths = stretchSizes(colMinWidths,colStrets,extra[0]),
                                rowHeights = stretchSizes(rowMinHeights,rowStrets,extra[1]),
                                colOffsets = integrate(colWidths,m_api.gapSize[0]),
                                rowOffsets = integrate(rowHeights,m_api.gapSize[1]);
            for (uint rr=0; rr<m_panes.height(); ++rr) {
                int                 baseY = m_base[1] + scast<int>(rowOffsets[rr]),
                                    szY = scast<int>(rowHeights[rr]);
                for (uint cc=0; cc<m_panes.width(); ++cc) {
                    int                 baseX = m_base[0] + scast<int>(colOffsets[cc]),
                                        szX = scast<int>(colWidths[cc]);
                    m_panes.xy(cc,rr)->moveWindow({baseX,baseY},{szX,szY});
                }
            }
        }
    }
};

GuiImplPtr          guiGetOsImpl(GuiSplit const & def) {return GuiImplPtr(new GuiSplitWin(def)); }

struct  GuiSplitWinDivider
{
    bool                drag=false;     // Is the user dragging this window ?
    // Last mouse pos in screen coords (since this window is moving !)
    // Only valid when drag == true:
    Vec2I               lastPos{0};
    Vec2UI              parentCursorLo{0};
    Vec2UI              parentCursorHi{0};

    LRESULT             wndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
    {
        // We need to track this because we don't want to adjust if the user
        // initiated the drag in another window. Also it's an easier way of
        // calculating deltas than calling the complicated GetMouseMovePointsEx:
        if (msg == WM_LBUTTONDOWN) {
            if (wParam == MK_LBUTTON) {
                SetCapture(hwnd);
                lastPos = winScreenPos(hwnd,lParam);
                drag = true;
                HWND    pHwnd = GetParent(hwnd);
                POINT   point;
                point.x = parentCursorLo[0];
                point.y = parentCursorLo[1];
                FGASSERTWIN(ClientToScreen(pHwnd,&point));
                RECT    rect;
                rect.left = point.x;
                rect.top = point.y;
                rect.right = parentCursorHi[0] + (point.x - parentCursorLo[0]);
                rect.bottom = parentCursorHi[1] + (point.y - parentCursorLo[1]);
                FGASSERTWIN(ClipCursor(&rect));
            }
        }
        else if (msg == WM_LBUTTONUP) {
            if (drag) {
                ReleaseCapture();
                drag = false;
                ClipCursor(NULL);
            }
        }
        else if (msg == WM_MOUSEMOVE) {
            if (wParam == MK_LBUTTON) {
                if (drag) {
                    Vec2I    pos = winScreenPos(hwnd,lParam);
                    Vec2I    delta = pos-lastPos;
                    lastPos = pos;
                    // SendMessage doesn't return until msg is processed.
                    // We can't just forward the WM_MOUSEMOVE since the coords
                    // aren't translated and we need to specify which divider:
                    SendMessage(
                        GetParent(hwnd),
                        WM_USER,
                        WPARAM(2),
                        MAKELPARAM(WORD(delta[0]),WORD(delta[1])));
                }
            }
        }
        else if (msg == WM_CAPTURECHANGED) {
            drag = false;
            ClipCursor(NULL);
        }
        else
            return DefWindowProc(hwnd,msg,wParam,lParam);
        return 0;
    }
};

struct      GuiSplitAdjWin : public GuiBaseImpl
{
    GuiSplitAdj                 m_api;
    HWND                        hwndThis;   // Must receive user messages from divider
    GuiImplPtr                  m_pane0;
    GuiImplPtr                  m_pane1;
    double                      m_relSize;
    struct  Div
    {
        GuiSplitWinDivider      win;
        HWND                    hwnd;
    };
    Div                         m_div;
    Vec2I                       m_client;
    String8                     m_store;

    static constexpr int dividerThickness = 3;
    static constexpr int borderThickness = 2;

    GuiSplitAdjWin(const GuiSplitAdj & api) :
        m_api(api),
        m_relSize(0.5)  // Default must be fixed since we can't yet call getMinSize of subwindows.
    {
        FGASSERT(api.pane0);
        FGASSERT(api.pane1);
        m_pane0 = api.pane0->getInstance();
        m_pane1 = api.pane1->getInstance();
    }

    ~GuiSplitAdjWin()
    {
        if (!m_store.empty()) {
            try {saveRaw(srlzText(m_relSize),m_store+"split.txt"); }
            catch(...) {}
        }
    }

    virtual void        create(HWND parentHwnd,int ident,String8 const & store,DWORD extStyle,bool visible)
    {
        m_store = store;
//fgout << fgnl << "SplitAdj::create: visible: " << visible << " extStyle: " << extStyle << fgpush;
        if (!m_store.empty()) {
            try {
                double              rs = dsrlzText<double>(loadRawString(m_store+"split.txt"));
                if ((rs > 0.0) && (rs < 1.0))
                    m_relSize = rs;
            }
            catch (...) {}
        }
        WinCreateChild      cc;
        cc.extStyle = extStyle;
        cc.visible = visible;
        // This window type must paint the background (tested) as moving the divider requires a full repaint
        // of each sub-window and the parent window hasn't moved so it does not refresh the background.
        cc.useFillBrush = true;
        winCreateChild(parentHwnd,ident,this,cc);
//fgout << fgpop;
    }

    virtual void        destroy()
    {
        // Automatically destroys children first:
        DestroyWindow(hwndThis);
    }

    Arr<Vec2UI,2>       getMinSizes() const
    {
        return {
            m_pane0->getMinSize() + Vec2UI(2*borderThickness),
            m_pane1->getMinSize() + Vec2UI(2*borderThickness),
        };
    }

    virtual Vec2UI      getMinSize() const
    {
        Arr<Vec2UI,2>       minSizes = getMinSizes();
        uint                sd = getSplitDimIdx(),
                            nd = 1 - sd,
                            maxMin = cMax(minSizes[0][nd],minSizes[1][nd]),
                            sumMin = minSizes[0][sd] + minSizes[1][sd];
        Vec2UI              ret;
        ret[sd] = sumMin + dividerThickness;
        ret[nd] = maxMin;
        return ret;
    }

    virtual Arr2B       wantStretch() const {return mapOr(m_pane0->wantStretch(),m_pane1->wantStretch()); }

    virtual void        updateIfChanged()
    {
//fgout << fgnl << "SplitAdj::updateIfChanged" << fgpush;
        m_pane0->updateIfChanged();
        m_pane1->updateIfChanged();
//fgout << fgpop;
    }

    virtual void        moveWindow(Vec2I lo,Vec2I sz)
    {
//fgout << fgnl << "SplitAdj::moveWindow: " << lo << "," << sz << fgpush;
        MoveWindow(hwndThis,lo[0],lo[1],sz[0],sz[1],FALSE);
//fgout << fgpop;
    }

    virtual void        showWindow(bool s)
    {
//fgout << fgnl << "SplitAdj::showWindow: " << s << fgpush;
        ShowWindow(hwndThis,s ? SW_SHOW : SW_HIDE);
//fgout << fgpop;
    }

    LRESULT             wndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
    {
        if (msg == WM_CREATE) {
//fgout << fgnl << "SplitAdj::WM_CREATE" << fgpush;
            hwndThis = hwnd;
            DWORD       extStyle = WS_EX_CLIENTEDGE;
            m_pane0->create(hwnd,0,m_store+"_0",extStyle);
            m_pane1->create(hwnd,1,m_store+"_1",extStyle);
            WinCreateChild   cc;
            cc.cursor = LoadCursor(NULL,m_api.horiz ? IDC_SIZEWE : IDC_SIZENS);
            m_div.hwnd = winCreateChild(hwnd,2,&m_div.win,cc);
//fgout << fgpop;
        }
        else if (msg == WM_SIZE) {      // Sends new size of client area:
            Arr<Vec2UI,2>       minSizes = getMinSizes();
            m_client = Vec2I(LOWORD(lParam),HIWORD(lParam));
            if (m_client[0] * m_client[1] == 0)
                return 0;
//fgout << fgnl << "SplitAdj::WM_SIZE: " << m_client << fgpush;
            // Set the adjustment cursor bounds:
            uint        sd = getSplitDimIdx(),
                        nd = 1-sd;
            m_div.win.parentCursorLo[nd] = borderThickness;
            m_div.win.parentCursorLo[sd] = minSizes[0][sd];
            m_div.win.parentCursorHi[nd] = m_client[nd] - borderThickness;
            m_div.win.parentCursorHi[sd] = m_client[sd] - minSizes[1][sd];
            resizePanes(minSizes);
//fgout << fgpop;
        }
        else if (msg == WM_USER) {      // Divider has been moved:
            Vec2I               delta(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
            uint                sd = getSplitDimIdx();
            if (delta[sd] != 0)
            {
                int     sizet = m_client[sd] - dividerThickness;
                double  reld = double(delta[sd]) / double(sizet);
                m_relSize += reld;
                resizePanes(getMinSizes());
                InvalidateRect(hwndThis,NULL,TRUE);
            }
        }
//case WM_PAINT:
//fgout << fgnl << "SplitAdj::WM_PAINT";
        else
            return DefWindowProc(hwnd,msg,wParam,lParam);
        return 0;
    }

    uint                getSplitDimIdx() const {return m_api.horiz ? 0 : 1; }

    struct      Pos
    {
        Vec2I    lo;
        Vec2I    sz;
    };
    struct      Layout
    {
        Pos         pane0;
        Pos         divider;
        Pos         pane1;
    };

    Layout              layout(Arr<Vec2UI,2> minSizes)
    {
        Layout      ret;
        uint        sd = getSplitDimIdx(),
                    nd = 1-sd;
        int         sizei = m_client[sd]-dividerThickness;
        double      sized = sizei;
        Vec2I    lo(0),sz(0);
        sz[nd] = m_client[nd];
        sz[sd] = roundT<int>(m_relSize*sized);
        // Adjust relative sizing if window resize hits one of the mins:
        int         min0 = minSizes[0][sd],
                    min1 = minSizes[1][sd];
        if (sz[sd] < min0)
            sz[sd] = min0;
        if (sizei-sz[sd] < min1)
            sz[sd] = sizei - min1;
        m_relSize = double(sz[sd]) / sized;
        ret.pane0.lo = lo;
        ret.pane0.sz = sz;
        lo[sd] = sz[sd];
        sz[sd] = dividerThickness;
        ret.divider.lo = lo;
        ret.divider.sz = sz;
        sz[sd] = sizei - lo[sd];
        lo[sd] += dividerThickness;
        ret.pane1.lo = lo;
        ret.pane1.sz = sz;
        return ret;
    }

    void                resizePanes(Arr<Vec2UI,2> minSizes)
    {
        Layout      lt = layout(minSizes);
        m_pane0->moveWindow(lt.pane0.lo,lt.pane0.sz);
        MoveWindow(m_div.hwnd,lt.divider.lo[0],lt.divider.lo[1],lt.divider.sz[0],lt.divider.sz[1],TRUE);
        m_pane1->moveWindow(lt.pane1.lo,lt.pane1.sz);
    }
};

GuiImplPtr          guiGetOsImpl(const GuiSplitAdj & def) {return GuiImplPtr(new GuiSplitAdjWin(def)); }

// Debug:
ostream &           operator<<(ostream & os,const SCROLLINFO & si)
{
    return os << "min: " << si.nMin
        << " max: " << si.nMax
        << " page: " << si.nPage
        << " pos: " << si.nPos
        << " trackPos " << si.nTrackPos;
}

struct      GuiSplitScrollWin : public GuiBaseImpl
{
    GuiSplitScroll          m_api;
    HWND                    hwndThis;
    Img<GuiImplPtr>         m_panes;
    Bools                   m_panesVisible;
    Vec2I                   m_client{0};        // doesn't include slider
    String8                 m_store;
    SCROLLINFO              m_si;
    int                     m_scrollBarThick = GetSystemMetrics(SM_CXVSCROLL);

    static size_t constexpr minHeight = 256;

    void                initPanes()
    {
        auto                fn = [](GuiPtr const & pane)
        {
            FGASSERT(pane);
            return pane->getInstance();
        };
        m_panes = mapCall(m_api.getPanes(),fn);
        m_panesVisible.resize(m_panes.height(),false);          // will be updated by 'resize()'
    }

    GuiSplitScrollWin(const GuiSplitScroll & api) : m_api(api), hwndThis(0)
    {
        initPanes();
        m_si.cbSize = sizeof(m_si);     // Never changes
        m_si.nMin = 0;                  // Never changes
        m_si.nPos = 0;                  // Initial value
        m_si.nTrackPos = 0;             // This value is always the same as the above value.
    }

    virtual void        create(HWND parentHwnd,int ident,String8 const & store,DWORD extStyle,bool visible)
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

    virtual void        destroy()
    {
        // Automatically destroys children first:
        DestroyWindow(hwndThis);
    }

    virtual Vec2UI      getMinSize() const
    {
        uint                maxMinWidth = 0;
        for (size_t rr=0; rr<m_panes.height(); ++rr) {
            uint                minWidth {0};
            for (size_t cc=0; cc<m_panes.width(); ++cc)
                minWidth += m_panes.xy(cc,rr)->getMinSize()[0];
            updateMax_(maxMinWidth,minWidth);
        }
        maxMinWidth += m_scrollBarThick;
        return {maxMinWidth,minHeight};
    }

    virtual Arr2B       wantStretch() const                     // always want vertical stretch
    {
        for (auto const & pane : m_panes.m_data)
            if (pane->wantStretch()[0])
                return {true,true};
        return {false,true};
    }

    virtual void        updateIfChanged()
    {
//fgout << fgnl << "SplitScroll::updateIfChanged";
        if (m_api.updateFlag->checkUpdate()) {
//fgout << " ... updating" << fgpush;
            for (GuiImplPtr const & p : m_panes.m_data)         // destroy all current panes
                p->destroy();
            initPanes();
            for (size_t ii=0; ii<m_panes.m_data.size(); ++ii)
                m_panes.m_data[ii]->create(hwndThis,int(ii),m_store+"_"+toStr(ii),0UL,false);
            resize();   // New windows must be sent a size
//fgout << fgpop;
        }
        for (size_t rr=0; rr<m_panes.height(); ++rr) {
            if (m_panesVisible[rr])
                for (size_t cc=0; cc<m_panes.width(); ++cc)
                    m_panes.xy(cc,rr)->updateIfChanged();
        }
    }

    virtual void        moveWindow(Vec2I lo,Vec2I sz)
    {
//fgout << fgnl << "SplitScroll::moveWindow: " << lo << "," << sz << fgpush;
        MoveWindow(hwndThis,lo[0],lo[1],sz[0],sz[1],FALSE);
//fgout << fgpop;
    }

    virtual void        showWindow(bool s)
    {
//fgout << fgnl << "SplitScroll::showWindow: " << s << fgpush;
        ShowWindow(hwndThis,s ? SW_SHOW : SW_HIDE);
//fgout << fgpop;
    }

    LRESULT             wndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
    {
        if (msg == WM_CREATE) {
//fgout << fgnl << "SplitScroll::WM_CREATE" << fgpush;
            hwndThis = hwnd;
            initPanes();
            for (size_t ii=0; ii<m_panes.m_data.size(); ++ii)
                m_panes.m_data[ii]->create(hwndThis,int(ii),m_store+"_"+toStr(ii),0UL,false);
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

    void                resize()
    {
        // No point in doing this before we have the client size (ie at first construction):
        if (m_client[1] > 0) {
            uint                drawWidth = m_client[0] - m_scrollBarThick;
            Img<Vec2UI>         minSizes = mapCall(m_panes,[](GuiImplPtr const & p){return p->getMinSize(); });
            Img<Arr2B>          strets = mapCall(m_panes,[](GuiImplPtr const & p){return p->wantStretch(); });
            Bools               colStrets = cColStrets(strets);
            Uints               rowHeights = cRowMinHeights(minSizes),
                                colMinWidths = cColMinWidths(minSizes),
                                colWidths = stretchSizes(colMinWidths,colStrets,drawWidth),
                                hOffsets = integrate(rowHeights),
                                wOffsets = integrate(colWidths);
            FGASSERT(m_si.nPos >= 0);
            uint                visibleStart = m_si.nPos,
                                visibleEnd = visibleStart + m_client[1];
            for (uint rr=0; rr<m_panes.height(); ++rr) {
                if ((hOffsets[rr+1] >= visibleStart) && (hOffsets[rr] <= visibleEnd)) {      // visible
                    for (uint cc=0; cc<m_panes.width(); ++cc) {
                        GuiImplPtr          pane = m_panes.xy(cc,rr);
                        Vec2I               panePos {
                            scast<int>(wOffsets[cc]),
                            scast<int>(hOffsets[rr] - scast<int>(visibleStart))
                        },
                                            paneSz {
                            scast<int>(colWidths[cc]),
                            scast<int>(rowHeights[rr]-m_api.spacing)
                        };
                        pane->moveWindow(panePos,paneSz);
                        if (!m_panesVisible[rr])
                            pane->updateIfChanged();       // may not have been updated when not visible
                        pane->showWindow(true);
                        m_panesVisible[rr] = true;
                    }
                }
                else {                                                                  // not visible
                    for (uint cc=0; cc<m_panes.width(); ++cc) {
                        GuiImplPtr          pane = m_panes.xy(cc,rr);
                        pane->showWindow(false);
                        m_panesVisible[rr] = false;
                    }
                }
            }
            // Windows wants the total range of the scrollable area, not the effective slider range resulting from
            // subtracting the currently displayed range:
            m_si.fMask = SIF_DISABLENOSCROLL | SIF_PAGE | SIF_POS | SIF_RANGE;
            m_si.nMax = hOffsets.back();
            m_si.nPage = m_client[1];
            // Windows will clamp the position and otherwise adjust:
            SetScrollInfo(hwndThis,SB_VERT,&m_si,TRUE);
            m_si.fMask = SIF_ALL;
            GetScrollInfo(hwndThis,SB_VERT,&m_si);
        }
    }
};

GuiImplPtr          guiGetOsImpl(const GuiSplitScroll & def) {return GuiImplPtr(new GuiSplitScrollWin(def)); }

}
