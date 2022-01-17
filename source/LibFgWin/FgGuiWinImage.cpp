//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
//

#include "stdafx.h"

#include "FgGuiApiImage.hpp"
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
    Vec2I               m_posWhenLButtonClicked;
    Vec2I               m_lastPos;                  // Last mouse position in CC
    bool                dragging;

    GuiImageWin(const GuiImage & api)
    : m_api(api), dragging(false)
    {}

    virtual void
    create(HWND parentHwnd,int ident,String8 const &,DWORD extStyle,bool visible)
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

    virtual Vec2UI getMinSize() const {return m_api.minSizeN.val(); }

    virtual Vec2B wantStretch() const {return m_api.wantStretch; }

    virtual void
    updateIfChanged()
    {
        // Avoid flickering due to background repaint if image size hasn't changed:
        if (m_api.updateFlag->checkUpdate()) {
            InvalidateRect(m_hwnd,NULL,TRUE);       // repaint BG
            m_api.updateNofill->checkUpdate();      // clear
        }
        else if (m_api.updateNofill->checkUpdate())
            InvalidateRect(m_hwnd,NULL,FALSE);      // only repaint image
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
            GuiImage::Disp          imgd = m_api.getImgFn(m_size);
            ImgRgba8 const &         img = *imgd.imgPtr;
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
            Vec2I           posIrcs = Vec2I(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
            if (!dragging) {
                if (m_api.clickLeft) {
                    m_api.clickLeft(posIrcs);
                    winUpdateScreen();
                }
            }
            dragging = false;
        }
        else if (msg == WM_MOUSEMOVE) {
            Vec2I    pos = Vec2I(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
            Vec2I    delta = pos-m_lastPos;
            m_lastPos = pos;
            if (wParam == MK_LBUTTON) {
                if (!dragging) {
                    // Add hysteresis to avoid annoying missed clicks due to very slight movement:
                    if (cMaxElem(mapAbs(pos-m_posWhenLButtonClicked)) > 1)
                        dragging = true;
                }
                if (dragging) {
                    if (m_api.dragLeft) {
                        m_api.dragLeft(delta);
                        winUpdateScreen();
                    }
                }
            }
            else if (wParam == MK_RBUTTON) {
                if (m_api.dragRight) {
                    m_api.dragRight(delta);
                    winUpdateScreen();
                }
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
