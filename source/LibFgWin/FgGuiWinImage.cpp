//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     May 6, 2011
//

#include "stdafx.h"

#include "FgGuiApiImage.hpp"
#include "FgGuiWin.hpp"
#include "FgThrowWindows.hpp"

using namespace std;

struct  FgGuiWinImage : public FgGuiOsBase
{
    HWND            m_hwnd;
    FgGuiApiImage   m_api;
    FgVect2UI       m_size;
    FgVect2I        m_lastPos;  // Last mouse position in CC
    bool            dragging;

    FgGuiWinImage(const FgGuiApiImage & api)
    : m_api(api), dragging(false)
    {}

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
        DestroyWindow(m_hwnd);
    }

    virtual FgVect2UI
    getMinSize() const
    {
        if (m_api.allowMouseCtls)
            return FgVect2UI(100,100);
        const FgImgRgbaUb & img = g_gg.getVal(m_api.imgN);
        return img.dims();
    }

    // This is a fixed window image display, not a stretchable one:
    virtual FgVect2B
    wantStretch() const
    {
        if (m_api.allowMouseCtls)
            return FgVect2B(true,true);
        return FgVect2B(false,false);
    }

    virtual void
    updateIfChanged()
    {
        // Avoid flickering due to background repaint if image size hasn't changed:
        uint    change = m_api.update();
        if (change == 2)
            InvalidateRect(m_hwnd,NULL,TRUE);
        else if (change == 1)
            InvalidateRect(m_hwnd,NULL,FALSE);
    }

    virtual void
    moveWindow(FgVect2I lo,FgVect2I sz)
    {MoveWindow(m_hwnd,lo[0],lo[1],sz[0],sz[1],TRUE); }

    virtual void
    showWindow(bool s)
    {ShowWindow(m_hwnd,s ? SW_SHOW : SW_HIDE); }

    LRESULT
    wndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
    {
        if (msg == WM_CREATE) {
            m_hwnd = hwnd;
        }
        else if (msg == WM_SIZE) {
            m_size = FgVect2UI(LOWORD(lParam),HIWORD(lParam));
        }
        else if (msg == WM_PAINT) {
            HDC         hdc;
            PAINTSTRUCT ps;
            // Validates the invalid region. Doesn't erase background
            // in this case since brush is NULL:
            hdc = BeginPaint(hwnd,&ps);
            struct  FGBMI
            {
                BITMAPINFOHEADER    bmiHeader;
                DWORD               redMask;
                DWORD               greenMask;
                DWORD               blueMask;
            };
            FgGuiImageDisp          imgd = m_api.disp(m_size);
            FGBMI                   bmi;
            memset(&bmi,0,sizeof(bmi));
            BITMAPINFOHEADER &      bmih = bmi.bmiHeader;
            bmih.biSize = sizeof(BITMAPINFOHEADER);
            bmih.biWidth = imgd.width;
            bmih.biHeight = -int(imgd.height);
            bmih.biPlanes = 1;                  // Must always be 1
            bmih.biBitCount = 32;
            bmih.biCompression = BI_BITFIELDS;  // Uncompressed
            bmi.redMask = 0xFF;
            bmi.greenMask = 0xFF00;
            bmi.blueMask = 0xFF0000;
            // Windows automatically handles clipping of the image for the bitblt:
            SetDIBitsToDevice(
                hdc,
                imgd.offsetx,imgd.offsety,
                imgd.width,imgd.height,
                0,0,
                0,imgd.height,
                imgd.dataPtr,
                (BITMAPINFO*)&bmi,
                DIB_RGB_COLORS);
            EndPaint(hwnd,&ps);
        }
        else if (msg == WM_LBUTTONDOWN) {
            m_lastPos = FgVect2I(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
            POINT   point;
            point.x = 0;
            point.y = 0;
            FGASSERTWIN(ClientToScreen(hwnd,&point));
            RECT    rect;
            rect.left = point.x;
            rect.top = point.y;
            rect.right = point.x + m_size[0];
            rect.bottom = point.y + m_size[1];
            FGASSERTWIN(ClipCursor(&rect));     // Prevent mouse from moving outside this window
        }
        else if (msg == WM_LBUTTONUP) {
            ClipCursor(NULL);
            FgVect2I    pos = FgVect2I(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
            if (!dragging) {
                m_api.click(pos);
                g_gg.updateScreen();
            }
            dragging = false;
        }
        else if (msg == WM_MOUSEMOVE) {
            FgVect2I    pos = FgVect2I(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
            FgVect2I    delta = pos-m_lastPos;
            m_lastPos = pos;
            if (wParam == MK_LBUTTON) {
                dragging = true;
                m_api.move(delta);
                g_gg.updateScreen();
            }
            else if (wParam == MK_RBUTTON) {
                m_api.zoom(delta[1]);
                g_gg.updateScreen();
            }
        }
        else
            return DefWindowProc(hwnd,msg,wParam,lParam);
        return 0;
    }
};

FgSharedPtr<FgGuiOsBase>
fgGuiGetOsInstance(const FgGuiApiImage & def)
{return FgSharedPtr<FgGuiOsBase>(new FgGuiWinImage(def)); }

// */
