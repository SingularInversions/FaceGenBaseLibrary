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
#include "FgTransform.hpp"

using namespace std;

namespace Fg {

struct  GuiSliderWin : public GuiBaseImpl
{
    GuiSlider           m_api;
    Affine1D            m_apiToWin;
    LPARAM              m_lastVal;      // Cache the last val returned by windows
    HWND                hwndSlider,
                        hwndThis;
    Vec2UI              m_client;

    GuiSliderWin(const GuiSlider & apiSlider) : m_api(apiSlider)
    {
        m_apiToWin = Affine1D {m_api.range.m[0],m_api.range.m[1],0,double(numTicks)};
    }

    virtual void    create(HWND parentHwnd,int ident,String8 const &,DWORD extStyle,bool visible)
    {
//fgout << fgnl << "Slider::create: visible: " << visible << " extStyle: " << extStyle << " ident: " << ident << fgpush;
        WinCreateChild   cc;
        cc.extStyle = extStyle;
        cc.visible = visible;
        winCreateChild(parentHwnd,ident,this,cc);
//fgout << fgpop;
    }

    virtual void    destroy()
    {
        // Automatically destroys children first:
        DestroyWindow(hwndThis);
    }

    virtual Vec2UI  getMinSize() const
    {
        return Vec2UI(
            minLabelWid() + minSliderWidth + 2*m_api.edgePadding,
            topSpace() + sliderHeight + botSpace());
    }

    virtual Arr2B   wantStretch() const {return Arr2B(true,false); }

    void            setPos(double newVal)
    {
        LPARAM          val = roundT<LPARAM>(m_apiToWin * newVal + 0.5);
        SendMessage(hwndSlider,TBM_SETPOS,
            TRUE,   // Must be true or slider graphics will not be updated.
            val);
        m_lastVal = val;
    }

    virtual void    updateIfChanged()
    {
        if (m_api.updateFlag->checkUpdate()) {
            double      newVal = m_api.getValFn(),
                        oldVal = m_apiToWin.invert(scast<double>(m_lastVal));
            // This message does not need to be sent to the slider that initiated this update as
            // Windows has already updated it, along with its graphic. Also of course no update
            // necessary for sliders that haven't changed. This optimization did little but we leave
            // it in just in case:
            if (newVal != oldVal)
                setPos(newVal);
        }
    }

    virtual void    moveWindow(Vec2I lo,Vec2I sz) {MoveWindow(hwndThis,lo[0],lo[1],sz[0],sz[1],TRUE); }

    virtual void    showWindow(bool s) {ShowWindow(hwndThis,s ? SW_SHOW : SW_HIDE); }

    LRESULT         wndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
    {
        if (msg == WM_CREATE) {
            hwndThis = hwnd;
            hwndSlider =
                CreateWindowEx(0,TRACKBAR_CLASSW,0,
                    WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_AUTOTICKS,
                    0,0,0,0,
                    hwnd,
                    HMENU(0),
                    s_guiWin.hinst,
                    NULL);                  // No WM_CREATE parameter
            FGASSERTWIN(hwndSlider);
            // Trackbars have a bug where rounding issues between the pixel size
            // and tick size can cause LINESIZE steps to get stuck. We get around
            // that, and ensure smooth scrolling, by using a large tick range:
            SendMessage(hwndSlider,TBM_SETRANGEMIN,FALSE,0);
            SendMessage(hwndSlider,TBM_SETRANGEMAX,FALSE,numTicks);
            // Up/down and left/right arrow step size:
            SendMessage(hwndSlider,TBM_SETLINESIZE,0,numTicks/100);
            // page up / page down step size:
            SendMessage(hwndSlider,TBM_SETPAGESIZE,0,numTicks/10);
            VecD2    rn = m_api.range;
            double      rts = m_api.tickSpacing / (rn[1]-rn[0]);
            uint        ts = roundT<uint>(rts * numTicks);
            SendMessage(hwndSlider,TBM_SETTICFREQ,ts,0);
            setPos(m_api.getValFn());
        }
        else if (msg == WM_SIZE) {          // Sends new size of client area.
            m_client = Vec2UI(LOWORD(lParam),HIWORD(lParam));
            if (m_client[0]*m_client[1] > 0) {
                MoveWindow(hwndSlider,
                    m_api.edgePadding,topSpace(),
                    sliderXSize(),sliderHeight,
                    TRUE);
            }
        }
        else if (msg == WM_HSCROLL) {
            // Trackbars don't send their identifier, just their window handle, presumably because the
            // params are full. Two messages are received for each movement of the thumb, presumably
            // a mouse drag and mouse button release in the case of mouse movement, and a key down key
            // up in the case of key presses. Rather than check message types, just use the fool-proof
            // method of only updating when an actual value change has occured:
            HWND        htb = HWND(lParam);
            LRESULT     winPos = SendMessage(htb,TBM_GETPOS,0,0);
            if (winPos != m_lastVal) {
                m_lastVal = winPos;
                double  newVal = m_apiToWin.invert(scast<double>(winPos));
                m_api.setValFn(newVal);
            }
        }
        else if (msg == WM_PAINT) {
            PAINTSTRUCT     ps;
            HDC             hdc = BeginPaint(hwnd,&ps);
            SetBkColor(hdc,GetSysColor(COLOR_3DFACE));
            RECT            rect;
            rect.left = m_api.edgePadding;
            rect.right = m_client[0] - m_api.edgePadding;
            // Write the main label:
            if (!m_api.label.empty()) {
                rect.top = 0;
                rect.bottom = topSpace();
                DrawText(hdc,
                    // Use c_str() to gracefully handle an empty label:
                    m_api.label.as_wstring().c_str(),
                    int(m_api.label.size()),
                    &rect,
                    DT_LEFT | DT_VCENTER | DT_WORDBREAK);
            }
            Affine1D            apiToPix {
                m_api.range.m[0],
                m_api.range.m[1],
                double(m_api.edgePadding + firstTickPad),
                double(m_client[0] - m_api.edgePadding - firstTickPad)
            };
            // Write the tock labels:
            if (!m_api.tockLabels.empty()) {
                rect.top = 0;
                rect.bottom = topSpace();
                GuiTickLabels const & ul = m_api.tockLabels;
                for (size_t ii=0; ii<ul.size(); ++ii) {
                    rect.left = roundT<int>(apiToPix * ul[ii].pos - 0.5 * tockLabelWidth);
                    rect.right = rect.left + tockLabelWidth;
                    DrawText(hdc,
                        ul[ii].label.as_wstring().c_str(),
                        int(ul[ii].label.size()),
                        &rect,
                        DT_CENTER);
                }
            }
            // Write the tick labels:
            if (!m_api.tickLabels.empty()) {
                rect.top = topSpace() + sliderHeight;
                rect.bottom = rect.top + botSpace();
                GuiTickLabels const & tl = m_api.tickLabels;
                for (size_t ii=0; ii<tl.size(); ++ii) {
                    rect.left = roundT<int>(apiToPix * tl[ii].pos - 0.5 * tickLabelWidth);
                    rect.right = rect.left + tickLabelWidth;
                    DrawText(hdc,
                        tl[ii].label.as_wstring().c_str(),
                        int(tl[ii].label.size()),
                        &rect,DT_CENTER);
                }
            }
            EndPaint(hwnd,&ps);
        }
        else
            return DefWindowProc(hwnd,msg,wParam,lParam);
        return 0;
    }

    static const uint numTicks = 1000;
    static const uint minSliderWidth = 200;
    static const uint sliderHeight = 30;
    static const uint tickLabelWidth = 90;
    static const uint tickLabelHeight = 20;
    static const uint labelHeight = 15;
    static const uint tockLabelWidth = 90;
    static const uint tockLabelHeight = 20;
    static const uint firstTickPad = 14;    // Hopefully constant on different windows.

    uint            topSpace() const
    {
        if (m_api.tockLabels.empty()) {
            if (m_api.label.empty())
                return 0;
            else
                return labelHeight;
        }
        else
            return tockLabelHeight;
    }

    uint            minLabelWid() const {return m_api.label.empty() ? 20 : 100; }

    uint            botSpace() const {return (m_api.tickLabels.empty() ? 0 : tickLabelHeight); }

    uint            sliderXSize() const {return (m_client[0] - 2*m_api.edgePadding); }
};

GuiImplPtr          guiGetOsImpl(const GuiSlider & def) {return GuiImplPtr(new GuiSliderWin(def)); }

}
