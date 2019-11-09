//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
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

namespace Fg {

struct  GuiImageWin : public GuiBaseImpl
{
    HWND                m_hwnd;
    GuiImage            m_api;
    Vec2UI           m_size;
    Vec2I            m_posWhenLButtonClicked;
    Vec2I            m_lastPos;                  // Last mouse position in CC
    bool                dragging;

    GuiImageWin(const GuiImage & api)
    : m_api(api), dragging(false)
    {}

    virtual void
    create(HWND parentHwnd,int ident,const Ustring &,DWORD extStyle,bool visible)
    {
        WinCreateChild   cc;
        cc.extStyle = extStyle;
        cc.visible = visible;
        winCreateChild(parentHwnd,ident,this,cc);
    }

    virtual void
    destroy()
    {
        // Automatically destroys children first:
        DestroyWindow(m_hwnd);
    }

    virtual Vec2UI
    getMinSize() const
    {
        if (m_api.allowMouseCtls)
            return Vec2UI(100,100);
        const ImgC4UC & img = m_api.imgN.cref();
        return img.dims();
    }

    virtual Vec2B
    wantStretch() const
    {
        if (m_api.allowMouseCtls)
            return Vec2B(true,true);
        return Vec2B(false,false);
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
    moveWindow(Vec2I lo,Vec2I sz)
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
            m_size = Vec2UI(LOWORD(lParam),HIWORD(lParam));
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
            GuiImageDisp            imgd = m_api.disp(m_size);
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
            m_posWhenLButtonClicked = Vec2I(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
            m_lastPos = m_posWhenLButtonClicked;
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
            Vec2I    pos = Vec2I(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
            if (!dragging) {
                m_api.click(pos);
                winUpdateScreen();
            }
            dragging = false;
        }
        else if (msg == WM_MOUSEMOVE) {
            Vec2I    pos = Vec2I(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
            Vec2I    delta = pos-m_lastPos;
            m_lastPos = pos;
            if (wParam == MK_LBUTTON) {
                if (dragging == false) {
                    // Add hysteresis to avoid annoying missed clicks due to very slight movement:
                    if (fgMaxElem(mapAbs(pos-m_posWhenLButtonClicked)) > 1)
                        dragging = true;
                }
                if (dragging) {
                    m_api.move(delta);
                    winUpdateScreen();
                }
            }
            else if (wParam == MK_RBUTTON) {
                m_api.zoom(delta[1]);
                winUpdateScreen();
            }
        }
        else
            return DefWindowProc(hwnd,msg,wParam,lParam);
        return 0;
    }
};

GuiImplPtr
guiGetOsImpl(const GuiImage & def)
{return GuiImplPtr(new GuiImageWin(def)); }

}

// */
