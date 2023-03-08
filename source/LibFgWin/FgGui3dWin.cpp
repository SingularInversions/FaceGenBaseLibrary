//
// Copyright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgGuiApi3d.hpp"
#include "FgDirect3D.hpp"
#include "FgGuiWin.hpp"
#include "FgHex.hpp"

using namespace std;
using namespace std::placeholders;

namespace Fg {

// D3D projection from frustum:
static Mat44F       calcProjection(const Frustum & f)
{
    // Projects from D3VS to D3PS. ie. maps:
    // * Near plane to Z=0, far plane to Z=1
    // * Left/right clip planes to X=-1/1
    // * Bottom/top clip planes to Y=-1/1
    double              a = f.nearDist / f.nearHalfWidth,
                        b = f.nearDist / f.nearHalfHeight,
                        del = f.farDist - f.nearDist,
                        c = f.farDist / del,
                        d = -f.nearDist*f.farDist / del;
    Mat44D            ret
    {{
        a, 0, 0, 0,
        0, b, 0, 0,
        0, 0, c, d,
        0, 0, 1, 0
    }};
    return Mat44F(ret);
}

struct  Gui3dWin : public GuiBaseImpl
{
    HWND                m_hwnd;
    Gui3d               m_api;
    Vec2UI              m_size;             // Value also stored in m_api.viewportDims but local copy handy
    HDC                 m_hdc;
    Vec2I               m_lastPos;              // Last mouse position in win32 client coords
    Arr<Vec2I,3>        lastButtonDownPosLMR;   // Left/Middle/Right buttons in win32 client coords
    ULONGLONG           m_lastGestureVal;   // Need to keep last gesture val to calc differences.
    DfgFPtr             m_updateBgImg;      // Update BG image ?
    Mat44F              m_worldToD3ps;      // Transform used for last render (for mouse-surface intersection)
    unique_ptr<D3d>     m_d3d;
    // Need to track this to avoid random object motion when mouse is dragged into viewport, and also when
    // file dialogs leak mouse moves into the viewport (MS bug):
    Arr<bool,3>         buttonIsDownLMR = {{false,false,false}};

    Gui3dWin(const Gui3d & api) : m_api(api)
    {
        // Including api.viewBounds, api.pan/tiltDegrees and api.logRelSize in the below caused a feedback
        // issue that broke updates of the other sliders:
        m_updateBgImg = makeUpdateFlag(api.bgImg.imgN);
        m_api.gpuInfo.set(cat(getGpusDescription(),"\n"));
    }

    virtual void    create(HWND parentHwnd,int ident,String8 const &,DWORD extStyle,bool visible)
    {
        WinCreateChild   cc;
        cc.extStyle = extStyle;
        cc.visible = visible;
        cc.useFillBrush = false;    // Rendering does it's own background fill
        winCreateChild(parentHwnd,ident,this,cc);
        // Don't create D3D here this is parent context.
    }

    virtual void    destroy()
    {
        // Don't release D3D here - this is parent context and isn't actually called.
        DestroyWindow(m_hwnd);
    }

    virtual Vec2UI  getMinSize() const {return Vec2UI(400,500); }

    virtual Vec2B   wantStretch() const { return Vec2B(true, true); }

    virtual void    updateIfChanged()   // Just always render
    {
        // This flips the dirty bit (QS_PAINT) for the render window but Windows will not
        // actually send a WM_PAINT message until the message queue is empty for a fraction
        // of a second (regardless of background paint flag):
        InvalidateRect(m_hwnd,NULL,FALSE);
        // So tell the system we want this updated immediately, for fluid interactive display:
        UpdateWindow(m_hwnd);
    }

    virtual void    moveWindow(Vec2I lo, Vec2I sz) {MoveWindow(m_hwnd,lo[0],lo[1],sz[0],sz[1],TRUE); }

    virtual void    showWindow(bool s) {ShowWindow(m_hwnd,s ? SW_SHOW : SW_HIDE); }

    LRESULT         wndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
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
            m_d3d.reset(new D3d{hwnd,m_api.rendMeshesN,m_api.logRelSize});
            m_api.capture->func = bind(&Gui3dWin::capture,this,_1,_2);
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
                Vec2UI          newSize{wid,hgt};
                if (newSize != m_size) {
                    m_size = newSize;
                    m_api.viewportDims.set(m_size);
                    try {
                        m_d3d->resize(m_size);
                    }
                    // This has happened to customers (ie. D3D ResizeBuffers can and does return it):
                    catch (ExceptD3dDeviceRemoved const &) {
                        m_d3d.reset(new D3d{hwnd,m_api.rendMeshesN,m_api.logRelSize});
                        m_d3d->resize(m_size);          // Assume it works this time
                    }
                    winUpdateScreen();      // Consumers of m_api.viewportSize need this
                }
            }
        }
        else if (msg == WM_PAINT) {
            PAINTSTRUCT ps;
            // Begin/End Paint required for D3D render or other windows don't render properly,
            // but swapBuffers not required.
            // Validates the invalid region. Doesn't erase background in this case since brush is NULL.
            BeginPaint(hwnd,&ps);
            bool                restartGpu = false;
            try {
                render();
            }
            catch (ExceptD3dDeviceRemoved const &) {
                restartGpu = true;
            }
            // One user had a Windows termination ('the program has stopped working and will now close' -
            // Win 10 Pro, NVIDIA GeForce GTX 760) here, even though 'm_hdc' was valid and a face was already
            // displayed on the screen (just once, when the program first started). Turned out to be a buggy
            // video driver (perhaps OpenGL specific).
            EndPaint(hwnd,&ps);
            if (restartGpu) {
                m_d3d.reset(new D3d{hwnd,m_api.rendMeshesN,m_api.logRelSize});
                m_d3d->resize(m_size);          // Assume it works this time
                winUpdateScreen();
            }
        }
        else if (contains(wmButtonsDown,msg)) {
            size_t              buttonIdx = findFirstIdx(wmButtonsDown,msg);
            // Position below is always the same as m_lastPos (tested) since the mouse had to WM_MOUSEMOVE here first.
            // But get 'pos' from params for clarity:
            Vec2I               pos(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
            buttonIsDownLMR[buttonIdx] = true;
            lastButtonDownPosLMR[buttonIdx] = pos;
            if ((wParam & MK_CONTROL) != 0)
                m_api.ctlClick(m_size,pos,m_worldToD3ps);
            captureCursor(hwnd);
        }
        else if (contains(wmButtonsUp,msg)) {
            size_t              buttonIdx = findFirstIdx(wmButtonsUp,msg);
            if (buttonIsDownLMR[buttonIdx]) {
                buttonIsDownLMR[buttonIdx] = false;    // Before any possible throw
                Vec2I               pos(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)),
                                    delta = pos - lastButtonDownPosLMR[buttonIdx];
                if (cDot(delta,delta) == 0) {       // This was a click not a drag
                    size_t              ctrl = ((wParam & MK_CONTROL) == 0) ? 0 : 1,
                                        shift = ((wParam & MK_SHIFT) == 0) ? 0 : 1;
                    MouseAction const & action = m_api.clickActions.at(buttonIdx,shift,ctrl);
                    if (action) {                   // If an action is defined for this combo
                        action(m_size,pos,m_worldToD3ps);
                        winUpdateScreen();
                    }
                }
            }
            // Release cursor if all buttons are up:
            if ((wParam & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON)) == 0)
                ClipCursor(NULL);
        }
        else if (msg == WM_MOUSEMOVE) {
            Vec2I    pos = Vec2I(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
            Vec2I    delta = pos-m_lastPos;
            if ((wParam == MK_LBUTTON) && buttonIsDownLMR[0]) {
                m_api.panTilt(delta);
                winUpdateScreen();
            }
            else if (wParam == (MK_LBUTTON | MK_SHIFT)) {
                m_api.translate(delta);
                winUpdateScreen();
            }
            else if (wParam == MK_MBUTTON) {
                m_api.translate(delta);
                winUpdateScreen();
            }
            else if (wParam == (MK_MBUTTON | MK_SHIFT | MK_CONTROL)) {
                if (m_api.shiftCtrlMiddleDragAction)
                    m_api.shiftCtrlMiddleDragAction(m_size,pos,m_worldToD3ps);
                winUpdateScreen();
            }
            else if (wParam == MK_RBUTTON) {
                m_api.scale(delta[1]);
                winUpdateScreen();
            }
            else if (wParam == (MK_RBUTTON | MK_SHIFT)) {
                if (m_api.shiftRightDragAction) {
                    m_api.shiftRightDragAction(m_size,pos,m_worldToD3ps);
                    winUpdateScreen();
                }
            }
            else if (wParam == (MK_LBUTTON | MK_CONTROL)) {
                if (m_api.ctlDragAction) {
                    m_api.ctlDrag(true,m_size,delta,m_worldToD3ps);
                    winUpdateScreen();
                }
            }
            else if (wParam == (MK_RBUTTON | MK_CONTROL)) {
                if (m_api.ctlDragAction) {
                    m_api.ctlDrag(false,m_size,delta,m_worldToD3ps);
                    winUpdateScreen();
                }
            }
            else if (wParam == (MK_LBUTTON | MK_RBUTTON)) {
                if (m_api.bothButtonsDragAction) {
                    m_api.bothButtonsDragAction(false,delta);
                    winUpdateScreen();
                }
            }
            else if (wParam == (MK_LBUTTON | MK_RBUTTON | MK_SHIFT)) {
                if (m_api.bothButtonsDragAction) {
                    m_api.bothButtonsDragAction(true,delta);
                    winUpdateScreen();
                }
            }
            else if (wParam == (MK_LBUTTON | MK_SHIFT | MK_CONTROL)) {
                m_api.translateBgImage(m_size,delta);
                winUpdateScreen();
            }
            else if (wParam == (MK_RBUTTON | MK_SHIFT | MK_CONTROL)) {
                m_api.scaleBgImage(m_size,delta);
                winUpdateScreen();
            }
            m_lastPos = pos;
        }
        else if (msg == WM_CAPTURECHANGED) {
            ClipCursor(NULL);
        }
        else if (msg == WM_DESTROY) {
            // Release GPU resources for render objects. Not strictly necessary since API releases automatically
            // and will ignore later Release() calls but better to be explicit:
            m_api.rendMeshesN = makeIPT(RendMeshes());
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
                        winUpdateScreen();
                    }
                    m_lastGestureVal = gi.ullArguments;
                    return 0;
                }
                else if (gi.dwID == GID_ROTATE) {   // Enabled in WM_CREATE
                    if (m_lastGestureVal > 0) {
                        m_api.roll((int(gi.ullArguments)-int(m_lastGestureVal)) / 32);  // by experiment
                        winUpdateScreen();
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
                winUpdateScreen();
            }
        }
        else
            return DefWindowProc(hwnd,msg,wParam,lParam);
        return 0;
    }

    void            captureCursor(HWND hwnd)
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

    void            renderBackBuffer(bool backgroundTransparent)
    {
        if (m_updateBgImg->checkUpdate())
            m_d3d->setBgImage(m_api.bgImg);
        Camera const &   camera = m_api.xform.cref();
        // A negative determinant matrix as D3D is a LEFT handed coordinate system:
        Mat33D                      oecsToD3vs = 
            {
                1, 0, 0,
                0, 1, 0,
                0, 0,-1,
            };
        Mat44F                      worldToD3vs = Mat44F(asHomogMat(oecsToD3vs * camera.modelview.asAffine())),
                                    d3vsToD3ps = calcProjection(camera.frustum);
        m_worldToD3ps = d3vsToD3ps * worldToD3vs;
        RendOptions               options = m_api.renderOptions.cref();
        m_d3d->renderBackBuffer(
            m_api.bgImg,
            m_api.light.cref(),
            worldToD3vs,
            d3vsToD3ps,
            m_size,
            options,
            backgroundTransparent);
    }

    void            render()
    {
        renderBackBuffer(false);
        m_d3d->showBackBuffer();
    }

    // This function is called by client so don't attempt to re-start GPU instance here, just return black image:
    ImgRgba8        capture(Vec2UI dims,bool backgroundTransparent)
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
