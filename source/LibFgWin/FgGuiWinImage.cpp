//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
//

#include "stdafx.h"

#include "FgGuiApi.hpp"
#include "FgGuiWin.hpp"
#include "FgThrowWindows.hpp"
#include "FgBounds.hpp"

using namespace std;

namespace Fg {

struct  GuiImageWin : public GuiBaseImpl
{
    HWND                m_hwnd;
    GuiImage            m_api;
    Vec2UI              m_size;
    Vec2I               m_lastPos;                  // Last mouse position in CC (move or click)
    GuiCursor           cursorState = GuiCursor::arrow;
    GuiCursor           noDragCursorState = GuiCursor::arrow;
    // shows the click index of the last single click down. This is invalid if no button is down, or if
    // any combination of buttons are hit at the same time, since click actions are not defined when other buttons are down:
    Opt<size_t>         lastSingleClick;
    GuiClickState       lastClickState;
    bool                noMovementSinceClick = false;

    GuiImageWin(GuiImage const & api) : m_api(api) {}

    virtual void    create(HWND parentHwnd,int ident,String8 const &,DWORD extStyle,bool visible)
    {
        WinCreateChild   cc;
        cc.extStyle = extStyle;
        cc.visible = visible;
        winCreateChild(parentHwnd,ident,this,cc);
    }
    virtual void    destroy()
    {
        DestroyWindow(m_hwnd);      // Automatically destroys children first
    }
    virtual Vec2UI  getMinSize() const {return m_api.minSizeN.val(); }
    virtual Vec2B   wantStretch() const {return m_api.wantStretch; }

    virtual void    updateIfChanged()
    {
        // Avoid flickering due to background repaint if image size hasn't changed:
        if (m_api.updateFlag->checkUpdate()) {
            InvalidateRect(m_hwnd,NULL,TRUE);       // repaint BG
            m_api.updateNofill->checkUpdate();      // clear
        }
        else if (m_api.updateNofill->checkUpdate())
            InvalidateRect(m_hwnd,NULL,FALSE);      // only repaint image
    }
    virtual void    moveWindow(Vec2I lo,Vec2I sz)
    {
        MoveWindow(m_hwnd,lo[0],lo[1],sz[0],sz[1],TRUE);
    }
    virtual void    showWindow(bool s)
    {
        ShowWindow(m_hwnd,s ? SW_SHOW : SW_HIDE);
    }
    LRESULT         wndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
    {
        if (msg == WM_CREATE)
            m_hwnd = hwnd;
        else if (msg == WM_SIZE)
            m_size = Vec2UI(LOWORD(lParam),HIWORD(lParam));
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
            GuiImage::Disp          imgd = m_api.getImgFn(m_size);
            ImgRgba8 const &        img = *imgd.imgPtr;
            FGBMI                   bmi;
            memset(&bmi,0,sizeof(bmi));
            BITMAPINFOHEADER &      bmih = bmi.bmiHeader;
            bmih.biSize = sizeof(BITMAPINFOHEADER);
            bmih.biWidth = img.dims()[0];
            bmih.biHeight = -int(img.dims()[1]);
            bmih.biPlanes = 1;                  // Must always be 1
            bmih.biBitCount = 32;
            bmih.biCompression = BI_BITFIELDS;  // Uncompressed
            bmi.redMask = 0xFF;
            bmi.greenMask = 0xFF00;
            bmi.blueMask = 0xFF0000;
            // Windows automatically handles clipping of the image for the bitblt:
            SetDIBitsToDevice(
                hdc,
                imgd.offset[0],imgd.offset[1],
                img.dims()[0],img.dims()[1],
                0,0,
                0,img.dims()[1],
                img.dataPtr(),
                (BITMAPINFO*)&bmi,
                DIB_RGB_COLORS);
            EndPaint(hwnd,&ps);
        }
        else if ((msg == WM_LBUTTONDOWN) || (msg == WM_MBUTTONDOWN) || (msg == WM_RBUTTONDOWN)) {
            GuiClickState       gcs  = clickStateFromWParam(wParam);
            Vec2I               pos (GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
            POINT               point {0,0};
            FGASSERTWIN(ClientToScreen(hwnd,&point));
            RECT                rect;
            rect.left = point.x;
            rect.top = point.y;
            rect.right = point.x + m_size[0];
            rect.bottom = point.y + m_size[1];
            FGASSERTWIN(ClipCursor(&rect));     // Prevent mouse from moving outside this window
            // there is no need to call TrackMouseEvent and handle WM_MOUSELEAVE to reset the cursor shape
            // since ClipCursor ensures that the WM_LBUTTONUP must happen within the client area:
            auto const &        callback = m_api.clickDownFns[gcs.toDragIndex()];
            if (callback)
                cursorState = callback(pos);
            if (gcs.onlyOneButton())
                lastSingleClick = gcs.getActionIndex();
            else
                lastSingleClick.reset();        // click actions not defined if more than one button is used
            noMovementSinceClick = true;
            lastClickState = gcs;
        }
        else if ((msg == WM_LBUTTONUP) || (msg == WM_MBUTTONUP) || (msg == WM_RBUTTONUP)) {
            GuiClickState       gcs  = clickStateFromWParam(wParam);
            if (!gcs.allButtonsUp())    // clicking is only defined for one button at a time
                return 0;
            ClipCursor(NULL);
            Vec2I               posIrcs = Vec2I(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
            size_t              buttonIdx = (msg==WM_LBUTTONUP) ? 0 : (msg==WM_MBUTTONUP ? 1 : 2),
                                keyIdx = gcs.getKeyIndex(),
                                actionIdx = keyIdx*3 + buttonIdx;
            // click actions not defined if more than one button involved, and only triggered if no movement (drag) occured:
            if ((lastSingleClick == actionIdx) && noMovementSinceClick) {
                if (m_api.clickActionFns[actionIdx]) {
                    m_api.clickActionFns[actionIdx](posIrcs);
                }
            }
            cursorState = noDragCursorState;
            lastClickState = gcs;
        }
        // WM_SETCURSOR is sent before each WM_MOUSEMOVE (despite being triggered by it, according to MS docs).
        // The distinction is unclear, other than WM_SETCURSOR gives no position and I guess is where the
        // cursor should be set. Windows automatically handles changing the cursor back and forth if the user
        // leaves the client area then returns:
        else if (msg == WM_SETCURSOR) {
            setCursor(cursorState);
            return TRUE;    // required for this to work. Tells windows that the cursor has been set.
        }
        else if (msg == WM_MOUSEMOVE) {
            Vec2I               pos = Vec2I(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)),
                                delta = pos-m_lastPos;
            // windows can send zero size mousemove messages, I think due to ClipCursor calls in button up/down handler:
            if (cMag(delta) > 0) {
                GuiClickState       gcs  = clickStateFromWParam(wParam);
                if (gcs == lastClickState) {       // need to check here as well since a key state may have changed
                    size_t              idx = gcs.toDragIndex();
                    if (m_api.mouseMoveFns[idx]) {
                        noDragCursorState = m_api.mouseMoveFns[idx](pos,delta);
                        if (idx == 0)
                            cursorState = noDragCursorState;
                    }
                }
                lastClickState = gcs;
                noMovementSinceClick = false;
            }
            m_lastPos = pos;    // only needs to be set here since other messages are redundant wrt mouse movement
        }
        else
            return DefWindowProc(hwnd,msg,wParam,lParam);
        return 0;
    }
};

GuiImplPtr          guiGetOsImpl(const GuiImage & def) {return GuiImplPtr(new GuiImageWin(def)); }

}

// */
