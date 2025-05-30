//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgGuiApi3d.hpp"
#include "FgDirect3D.hpp"
#include "FgGuiWin.hpp"

using namespace std;
using namespace std::placeholders;

namespace Fg {

struct  Gui3dWin : public GuiBaseImpl
{
    HWND                m_hwnd;
    Gui3d               m_api;
    Vec2UI              m_size{0};          // Value also stored in m_api.viewportDims but local copy handy
    HDC                 m_hdc;
    Vec2I               m_lastPos{0};       // Last mouse position in win32 client coords
    ULONGLONG           m_lastGestureVal;   // Need to keep last gesture val to calc differences.
    DfFPtr             m_updateBgImg;      // Update BG image ?
    Mat44F              m_worldToD3ps;      // Transform used for last render (for mouse-surface intersection)
    unique_ptr<D3d>     m_d3d;

    Gui3dWin(const Gui3d & api) : m_api(api)
    {
        // Including api.viewBounds, api.pan/tiltDegrees and api.logRelSize in the below caused a feedback
        // issue that broke updates of the other sliders:
        m_updateBgImg = cUpdateFlagT(api.bgImg.imgN);
        m_api.gpuInfo.set(cat(getGpusDescription(),"\n"));
        m_api.capture->func = bind(&Gui3dWin::capture,this,_1,_2);
    }

    virtual void        create(HWND parentHwnd,int ident,String8 const &,DWORD extStyle,bool visible)
    {
        WinCreateChild   cc;
        cc.extStyle = extStyle;
        cc.visible = visible;
        cc.useFillBrush = false;    // Rendering does it's own background fill
        winCreateChild(parentHwnd,ident,this,cc);
        // Don't create D3D here this is parent context.
    }

    virtual void        destroy()
    {
        // Don't release D3D here - this is parent context and isn't actually called.
        DestroyWindow(m_hwnd);
    }

    virtual Vec2UI      getMinSize() const {return {256,256}; }

    virtual Arr2B       wantStretch() const {return {true,true}; }

    virtual void        updateIfChanged()   // Just always render
    {
        // This flips the dirty bit (QS_PAINT) for the render window but Windows will not
        // actually send a WM_PAINT message until the message queue is empty for a fraction
        // of a second (regardless of background paint flag). We don't want to call UpdateWindow
        // however, since that immediately sends the paint message bypassing the message loop,
        // which can cause a lag of several frames as updates are rendered which are already out of date:
        InvalidateRect(m_hwnd,NULL,FALSE);
    }

    virtual void        moveWindow(Vec2I lo, Vec2I sz) {MoveWindow(m_hwnd,lo[0],lo[1],sz[0],sz[1],TRUE); }

    virtual void        showWindow(bool s) {ShowWindow(m_hwnd,s ? SW_SHOW : SW_HIDE); }

    LRESULT             wndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
    {
        Arr<uint,3> const       wmButtonsUp   {WM_LBUTTONUP,WM_MBUTTONUP,WM_RBUTTONUP};
        Arr<uint,3> const       wmButtonsDown {WM_LBUTTONDOWN,WM_MBUTTONDOWN,WM_RBUTTONDOWN};
        if (msg == WM_CREATE) {
            m_hwnd = hwnd;
            m_hdc = GetDC(hwnd);
            PIXELFORMATDESCRIPTOR pfd;
            ZeroMemory(&pfd,sizeof(pfd));
            pfd.nSize      = sizeof(pfd);
            pfd.nVersion   = 1;
            pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
            pfd.iPixelType = PFD_TYPE_RGBA;
            pfd.cColorBits = 32;
            pfd.cDepthBits = 24;
            pfd.cAlphaBits = 8;
            pfd.cStencilBits = 8;
            int format = ChoosePixelFormat(m_hdc,&pfd);
            FGASSERTWIN(format != 0);
            FGASSERTWIN(SetPixelFormat(m_hdc,format,&pfd));
            FGASSERTWIN(DescribePixelFormat(m_hdc,format,pfd.nSize,&pfd));
            m_d3d.reset(new D3d{hwnd,m_api.rendMeshesN,m_api.logRelSize,m_api.tryForTransparency});
            // The pinch-to-zoom gesture is enabled by default but not the rotate gesture, which we
            // must explicitly enable:
            GESTURECONFIG   config = {0};
            config.dwWant = GC_ROTATE;
            config.dwID = GID_ROTATE;
            config.dwBlock = 0;
            // This function returned a "not implemented" error on a German Windows 7 64bit SP1 system,
            // so don't throw on error, just continue and presumably no gesture messages will be received:
            SetGestureConfig(hwnd,0,1,&config,sizeof(GESTURECONFIG));
            if (m_api.fileDragDrop)
                DragAcceptFiles(hwnd,TRUE);
        }
        else if (msg == WM_SIZE) {
            uint        wid = LOWORD(lParam),
                        hgt = HIWORD(lParam);
            // In case 0 size sent at creation or minimize:
            if (wid*hgt > 0) {
                m_size = {wid,hgt};
                m_api.viewportDims.set(m_size);
                try {m_d3d->resize(m_size); }       // This has failed for customers
                catch (ExceptD3dDeviceRemoved const &) {
                    m_d3d.reset(new D3d{m_hwnd,m_api.rendMeshesN,m_api.logRelSize,m_api.tryForTransparency});
                    m_d3d->resize(m_size);
                }
            }
        }
        else if (msg == WM_PAINT) {
            PAINTSTRUCT ps;
            // Begin/End Paint required for D3D render or other windows don't render properly,
            // but swapBuffers not required.
            // Validates the invalid region. Doesn't erase background in this case since brush is NULL.
            BeginPaint(hwnd,&ps);
            try {render(); }
            catch (ExceptD3dDeviceRemoved const &) {
                m_d3d.reset(new D3d{m_hwnd,m_api.rendMeshesN,m_api.logRelSize,m_api.tryForTransparency});
                m_d3d->resize(m_size);
            }
            EndPaint(hwnd,&ps);
        }
        // handle the mouse button down/up messages. modifier key (shift & ctrl) state is only used
        // at mouse message events; modifier key events are NOT handled.
        else if (contains(wmButtonsDown,msg)) {
            size_t              buttonIdx = findFirstIdx(wmButtonsDown,msg);
            // Position below is always the same as m_lastPos (tested) since the mouse had to WM_MOUSEMOVE here first.
            // But get 'pos' from params for clarity:
            Vec2I               pos(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
            m_api.buttonDown(buttonIdx,m_size,pos,m_worldToD3ps);
            captureCursor(hwnd);
        }
        else if (contains(wmButtonsUp,msg)) {
            size_t              buttonIdx = findFirstIdx(wmButtonsUp,msg);
            // since we capture the cursor while any button is down, we are guaranteed to be notified
            // when that button comes back up, so if the mouse enters the client area with a button
            // already down, we can see the last button down is invalid and not call the associated drag
            // (with invalid button down information):
            if (m_api.lastButtonDown[buttonIdx].has_value()) {      // button went down in client area
                LastClick           lc = m_api.lastButtonDown[buttonIdx].value();
                Vec2I               pos {GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)},
                                    delta = pos - lc.pos;
                m_api.lastButtonDown[buttonIdx].reset();
                if (cDot(delta,delta) == 0) {       // This was a click not a drag
                    size_t              ctrl = ((wParam & MK_CONTROL) == 0) ? 0 : 1,
                                        shift = ((wParam & MK_SHIFT) == 0) ? 0 : 1;
                    ClickAction const & action = m_api.clickActions.at(buttonIdx,shift,ctrl);
                    if (action) {                   // If an action is defined for this combo
                        action(m_size,pos,m_worldToD3ps);
                    }
                }
            }
            // Release cursor if all buttons are up:
            if ((wParam & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON)) == 0)
                ClipCursor(NULL);
        }
        else if (msg == WM_MOUSEMOVE) {
            Vec2I           pos = Vec2I(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
            Vec2I           delta = pos - m_lastPos;
            size_t          buttonL = (wParam & MK_LBUTTON) ? 1 : 0,
                            buttonM = (wParam & MK_MBUTTON) ? 1 : 0,
                            buttonR = (wParam & MK_RBUTTON) ? 1 : 0,
                            shift = (wParam & MK_SHIFT) ? 1 : 0,
                            ctrl = (wParam & MK_CONTROL) ? 1 : 0;
            auto const &    action = m_api.dragActions.at(buttonL,buttonM,buttonR,shift,ctrl);
            if (action) {
                // if more than 1 buttons are down, choose the first valid one in LMR order:
                Arr<size_t,3>       buttons {buttonL,buttonM,buttonR};
                for (size_t ii=0; ii<3; ++ii) {
                    if (buttons[ii] == 1) {     // this button is down
                        Opt<LastClick> const &  olc = m_api.lastButtonDown[ii];
                        if (olc.has_value()) {      // and this button was pressed while in the viewport
                            action(m_size,delta,m_worldToD3ps,olc.value());
                            break;                  // only take the action once (if more than 1 button down)
                        }
                    }
                }
            }
            // Need to check that button went down in viewport to avoid random object motion when mouse
            // is dragged into viewport with button already down, and also when file dialogs leak mouse
            // moves into the viewport (MS bug):
            else if ((wParam == MK_LBUTTON) && m_api.lastButtonDown[0].has_value()) {
                m_api.panTilt(delta);
            }
            else if (wParam == (MK_LBUTTON | MK_SHIFT)) {
                m_api.translate(delta);
            }
            else if (wParam == MK_MBUTTON) {
                m_api.translate(delta);
            }
            else if (wParam == (MK_MBUTTON | MK_SHIFT | MK_CONTROL)) {
                if (m_api.shiftCtrlMiddleDragAction)
                    m_api.shiftCtrlMiddleDragAction(m_size,pos,m_worldToD3ps);
            }
            else if (wParam == MK_RBUTTON) {
                m_api.scale(delta[1]);
            }
            else if (wParam == (MK_RBUTTON | MK_SHIFT)) {
                if (m_api.shiftRightDragAction) {
                    m_api.shiftRightDragAction(m_size,pos,m_worldToD3ps);
                }
            }
            else if (wParam == (MK_LBUTTON | MK_CONTROL)) {
                if (m_api.ctlDragAction) {
                    m_api.ctlDrag(true,m_size,delta,m_worldToD3ps);
                }
            }
            else if (wParam == (MK_RBUTTON | MK_CONTROL)) {
                if (m_api.ctlDragAction) {
                    m_api.ctlDrag(false,m_size,delta,m_worldToD3ps);
                }
            }
            else if (wParam == (MK_LBUTTON | MK_RBUTTON)) {
                if (m_api.bothButtonsDragAction) {
                    m_api.bothButtonsDragAction(false,delta);
                }
            }
            else if (wParam == (MK_LBUTTON | MK_RBUTTON | MK_SHIFT)) {
                if (m_api.bothButtonsDragAction) {
                    m_api.bothButtonsDragAction(true,delta);
                }
            }
            else if (wParam == (MK_LBUTTON | MK_SHIFT | MK_CONTROL)) {
                m_api.translateBgImage(m_size,delta);
            }
            else if (wParam == (MK_RBUTTON | MK_SHIFT | MK_CONTROL)) {
                m_api.scaleBgImage(m_size,delta);
            }
            m_lastPos = pos;
        }
        else if (msg == WM_CAPTURECHANGED) {
            ClipCursor(NULL);
        }
        else if (msg == WM_DESTROY) {
            // Release GPU resources for render objects. Not strictly necessary since API releases automatically
            // and will ignore later Release() calls but better to be explicit:
            m_d3d.reset();
            if (m_hdc) {
                ReleaseDC(hwnd,m_hdc);
                m_hdc = NULL;
            }
        }
        else if (msg == WM_GESTURE) {
            GESTUREINFO     gi;
            ZeroMemory(&gi,sizeof(GESTUREINFO));
            gi.cbSize = sizeof(GESTUREINFO);
            if (GetGestureInfo((HGESTUREINFO)lParam,&gi)) {
                if (gi.dwFlags & GF_BEGIN) {
                    // Set to zero to indicate the start of a new gesture, and don't store
                    // current value since it's not valid:
                    m_lastGestureVal = 0;
                }
                else if (gi.dwID == GID_ZOOM) {     // Enabled by default
                    if (m_lastGestureVal > 0) {
                        m_api.scale(int(gi.ullArguments)-int(m_lastGestureVal));
                    }
                    m_lastGestureVal = gi.ullArguments;
                    return 0;
                }
                else if (gi.dwID == GID_ROTATE) {   // Enabled in WM_CREATE
                    if (m_lastGestureVal > 0) {
                        m_api.roll((int(gi.ullArguments)-int(m_lastGestureVal)) / 32);  // by experiment
                    }
                    m_lastGestureVal = gi.ullArguments;
                    return 0;
                }
            }
        }
        else if (msg == WM_DROPFILES) {
            // We should never receive 'WM_DROPFILES' if this function isn't defined but may as well be safe:
            if (m_api.fileDragDrop) {
                HDROP           hDrop = HDROP(wParam);
                WCHAR           buff[512] {};
                // This function can also be used to get information about multiple files,
                // but don't bother handling that, just take the first:
                UINT            ret = DragQueryFile(hDrop,0,buff,511);
                if (ret != 0) {
                    String8         filePath {&buff[0]};    // UTF-16 to UTF-8
                    m_api.fileDragDrop(filePath);
                }
                DragFinish(hDrop);      // Windows releases paths memory
            }
        }
        else
            return DefWindowProc(hwnd,msg,wParam,lParam);
        return 0;
    }

    void                captureCursor(HWND hwnd)
    {
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

    void                renderBackBuffer(bool backgroundTransparent)
    {
        if (m_updateBgImg->checkUpdate())
            m_d3d->setBgImage(m_api.bgImg);
        Camera const &   camera = m_api.cameraN.val();
        // A negative determinant matrix as D3D is a LEFT handed coordinate system:
        Mat33D                      oecsToD3vs = 
            {
                1, 0, 0,
                0, 1, 0,
                0, 0,-1,
            };
        Mat44F                      worldToD3vs = Mat44F(asHomogMat(oecsToD3vs * camera.modelview.asAffine())),
                                    d3vsToD3ps = camera.frustum.asD3dProjection();
        m_worldToD3ps = d3vsToD3ps * worldToD3vs;
        RendOptions               options = m_api.renderOptions.val();
        m_d3d->renderBackBuffer(
            m_api.bgImg,
            m_api.lightingN.val(),
            worldToD3vs,
            d3vsToD3ps,
            m_size,
            options,
            scast<float>(m_api.texModStrengthN.val()),
            backgroundTransparent);
    }

    void                render()
    {
        renderBackBuffer(false);
        m_d3d->showBackBuffer();
    }

    // This function is called by client so don't attempt to re-start GPU instance here, just return black image:
    ImgRgba8            capture(Vec2UI dims,bool backgroundTransparent)
    {
        ImgRgba8             ret;
        try {
            m_d3d->resize(dims);
            m_api.viewportDims.set(dims);
            renderBackBuffer(backgroundTransparent);
            ret = m_d3d->capture(dims);
            m_api.viewportDims.set(m_size);
            m_d3d->resize(m_size);
        }
        catch (ExceptD3dDeviceRemoved const &) {
            ret.resize(dims,Rgba8{0});
        }
        return ret;
    }
};

GuiImplPtr          guiGetOsImpl(const Gui3d & def) {return GuiImplPtr(new Gui3dWin(def)); }

}

// */
