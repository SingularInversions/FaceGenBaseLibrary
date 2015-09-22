//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     March 18, 2011
//

#include "stdafx.h"

#include "FgGuiApiSlider.hpp"
#include "FgGuiWin.hpp"
#include "FgThrowWindows.hpp"
#include "FgAffine1.hpp"

using namespace std;

static const uint numTicks = 1000;
static const uint padSides = 10;         // For tockLabel overhang
static const uint minSliderWidth = 200;
static const uint sliderHeight = 30;
static const uint tickLabelWidth = 90;
static const uint s_tickLabelHeight = 20;
static const uint tockLabelWidth = 90;
static const uint tockLabelHeight = 20;
static const uint firstTickPad = 14;    // Hopefully constant on different windows.

struct  FgGuiWinSlider : public FgGuiOsBase
{
    FgGuiApiSlider      m_api;
    FgAffine1D          m_apiToWin;
    LRESULT             m_lastVal;      // Cache the last val returned by windows
    HWND                hwndSlider,
                        hwndThis;
    FgVect2UI           m_client;

    FgGuiWinSlider(const FgGuiApiSlider & apiSlider)
        : m_api(apiSlider)
    {
        m_apiToWin =
            FgAffine1D(
                m_api.range,
                FgVectD2(0.0,double(numTicks)));
    }

    virtual void
    create(HWND parentHwnd,int ident,const FgString &,DWORD extStyle,bool visible)
    {
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
        return
            FgVect2UI(
                minLabelWid() + minSliderWidth + 2*padSides,
                sliderYPos() + sliderHeight + tickLabelHeight());
    }

    virtual FgVect2B
    wantStretch() const
    {return FgVect2B(true,false); }

    virtual void
    updateIfChanged()
    {
        if (g_gg.dg.update(m_api.updateFlagIdx)) {
            double      newVal = m_api.getInput(),
                        oldVal = m_apiToWin.invXform(m_lastVal);
            // This message does not need to be sent to the slider than initiated this update as
            // Windows has already updated it, along with its graphic. Also of course no update
            // necessary for sliders than haven't changed. This optimization did little but we leave
            // it in just in case:
            if (newVal != oldVal) {
                LPARAM  val = std::floor(m_apiToWin * newVal + 0.5);
                SendMessage(hwndSlider,TBM_SETPOS,
                    TRUE,   // Must be true or slider graphics will not be updated.
                    val);
                m_lastVal = val;
            }
        }
    }

    virtual void
    moveWindow(FgVect2I lo,FgVect2I sz)
    {MoveWindow(hwndThis,lo[0],lo[1],sz[0],sz[1],TRUE); }

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
                hwndThis = hwnd;
                hwndSlider =
                    CreateWindowEx(0,
                        TRACKBAR_CLASSW,
                        0,
                        WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_AUTOTICKS,
                        0,0,0,0,
                        hwnd,
                        HMENU(0),
                        s_fgGuiWin.hinst,
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
                FgVectD2    rn = m_api.range;
                double      rts = m_api.tickSpacing / (rn[1]-rn[0]);
                uint        ts = fgRoundU(rts * numTicks);
                SendMessage(hwndSlider,TBM_SETTICFREQ,ts,0);
                return 0;
            }
        case WM_SIZE:   // Sends new size of client area.
            {
                m_client = FgVect2UI(LOWORD(lParam),HIWORD(lParam));
                if (m_client[0]*m_client[1] > 0) {
                    MoveWindow(hwndSlider,
                        sliderXPos(),sliderYPos(),
                        sliderXSize(),sliderHeight,
                        TRUE);
                }
            }
            return 0;
        case WM_HSCROLL:
            {
                // Trackbars don't send their identifier, just their window handle,
                // presumably because the params are full. Two messages are received
                // for each movement of the thumb, presumably a mouse drag and mouse
                // button release in the case of mouse movement, and a key down key
                // up in the case of key presses. Rather than check message types, just use
                // the fool-proof method of only updating when an actual value change has occured:
                HWND        htb = HWND(lParam);
                LRESULT     winPos = SendMessage(htb,TBM_GETPOS,0,0);
                if (winPos != m_lastVal) {
                    m_lastVal = winPos;
                    double  newVal = m_apiToWin.invXform(winPos);
                    m_api.setOutput(newVal);
                    g_gg.updateScreen();
                }
            }
            return 0;
        case WM_PAINT:
            {
                PAINTSTRUCT     ps;
                HDC             hdc = BeginPaint(hwnd,&ps);
                SetBkColor(hdc,GetSysColor(COLOR_ACTIVEBORDER));
                RECT            rect;

                // Write the main label:
                rect.top = sliderYPos();
                rect.bottom = rect.top + sliderHeight + tickLabelHeight();
                rect.left = padSides;
                rect.right = rect.left + labelXSize();
                DrawText(hdc,
                    // Use c_str() to gracefully handle an empty label:
                    m_api.label.as_wstring().c_str(),
                    int(m_api.label.length()),
                    &rect,
                    DT_LEFT | DT_VCENTER | DT_WORDBREAK);
                FgAffine1D      apiToPix(m_api.range,
                    FgVectD2(padSides + labelXSize() + firstTickPad,m_client[0] - padSides - firstTickPad));
                // Write the tock labels:
                rect.top = 0;
                rect.bottom = rect.top + tockLabelHeight;
                const FgGuiApiTickLabels & ul = m_api.tockLabels;
                for (size_t ii=0; ii<ul.size(); ++ii) {
                    rect.left = 
                        fgRound(
                            apiToPix * ul[ii].pos - 0.5 * tockLabelWidth);
                    rect.right = rect.left + tockLabelWidth;
                    DrawText(
                        hdc,
                        ul[ii].label.as_wstring().c_str(),
                        int(ul[ii].label.length()),
                        &rect,
                        DT_CENTER);
                }

                // Write the tick labels:
                rect.top = sliderYPos()+sliderHeight;
                rect.bottom = rect.top + tickLabelHeight();
                const FgGuiApiTickLabels & tl = m_api.tickLabels;
                for (size_t ii=0; ii<tl.size(); ++ii)
                {
                    rect.left = 
                        fgRound(
                            apiToPix * tl[ii].pos - 0.5 * tickLabelWidth);
                    rect.right = rect.left + tickLabelWidth;
                    DrawText(
                        hdc,
                        tl[ii].label.as_wstring().c_str(),
                        int(tl[ii].label.length()),
                        &rect,
                        DT_CENTER);
                }

                EndPaint(hwnd,&ps);
            }
            return 0;
        }
        return DefWindowProc(hwnd,message,wParam,lParam);
    }

    uint
    minLabelWid() const
    {
        return m_api.label.empty() ? 20 : 100;
    }

    uint
    labelXSize() const
    {
        return
            (m_client[0]-2*padSides) * minLabelWid() / (minLabelWid() + minSliderWidth);
    }

    uint
    tickLabelHeight() const
    {return (m_api.tickLabels.empty() ? 0 : s_tickLabelHeight); }

    uint
    sliderXPos() const
    {return padSides + labelXSize(); }

    uint
    sliderXSize() const
    {return (m_client[0] - 2*padSides - labelXSize()); }

    uint
    sliderYPos() const
    {return (m_api.tockLabels.empty() ? 0 : tockLabelHeight); }
};

FgSharedPtr<FgGuiOsBase>
fgGuiGetOsInstance(const FgGuiApiSlider & def)
{return FgSharedPtr<FgGuiOsBase>(new FgGuiWinSlider(def)); }
