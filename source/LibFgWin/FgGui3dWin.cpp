//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgGuiApi3d.hpp"
#include "FgDirect3D.hpp"
#include "FgGuiWin.hpp"
#include "FgCoordSystem.hpp"
#include "FgHex.hpp"

using namespace std;

namespace Fg {

// D3D projection from frustum:
static Mat44F
calcProjection(const Frustum & f)
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
    Vec2I               m_lastPos;          // Last mouse position in CC (only valid if drag!=None)
    ULONGLONG           m_lastGestureVal;   // Need to keep last gesture val to calc differences.
    DfgFPtr             m_updateBgImg;      // Update BG image ?
    Mat44F              m_worldToD3ps;      // Transform used for last render (for mouse-surface intersection)
    unique_ptr<D3d>     m_d3d;

    Gui3dWin(const Gui3d & api)
    : m_api(api)
    {
        // Including api.viewBounds, api.pan/tiltDegrees and api.logRelSize in the below caused a feedback
        // issue that broke updates of the other sliders:
        m_updateBgImg = makeUpdateFlag(api.bgImg.imgN);
        m_api.gpuInfo.set(cat(getGpusDescription(),"\n"));
    }

    virtual void
    create(HWND parentHwnd,int ident,Ustring const &,DWORD extStyle,bool visible)
    {
        WinCreateChild   cc;
        cc.extStyle = extStyle;
        cc.visible = visible;
        cc.useFillBrush = false;
        winCreateChild(parentHwnd,ident,this,cc);
        // Don't create D3D here this is parent context.
    }

    virtual void
    destroy()
    {
        // Don't release D3D here - this is parent context and isn't actually called.
        DestroyWindow(m_hwnd);
    }

    virtual Vec2UI
    getMinSize() const
    {return Vec2UI(400,500); }

    virtual Vec2B wantStretch() const
    { return Vec2B(true, true); }

    virtual void
    updateIfChanged()   // Just always render
    {
        // This flips the dirty bit (QS_PAINT) for the render window but Windows will not
        // actually send a WM_PAINT message until the message queue is empty for a fraction
        // of a second (regardless of background paint flag):
        InvalidateRect(m_hwnd,NULL,FALSE);
        // So tell the system we want this updated immediately, for fluid interactive display:
        UpdateWindow(m_hwnd);
    }

    virtual void moveWindow(Vec2I lo, Vec2I sz)
    {MoveWindow(m_hwnd,lo[0],lo[1],sz[0],sz[1],TRUE); }

    virtual void showWindow(bool s)
    {ShowWindow(m_hwnd,s ? SW_SHOW : SW_HIDE); }

    LRESULT
    wndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
    {
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
            m_d3d.reset(new D3d(hwnd));
            m_api.capture->getImg = bind(&Gui3dWin::capture,this,placeholders::_1);
            // The pinch-to-zoom gesture is enabled by default but not the rotate gesture, which we
            // must explicitly enable:
            GESTURECONFIG   config = {0};
            config.dwWant = GC_ROTATE;
            config.dwID = GID_ROTATE;
            config.dwBlock = 0;
            // This function returned a "not implemented" error on a German Windows 7 64bit SP1 system,
            // so don't throw on error, just continue and presumably no gesture messages will be received:
            SetGestureConfig(hwnd,0,1,&config,sizeof(GESTURECONFIG));
        }
        else if (msg == WM_SIZE) {
            uint        wid = LOWORD(lParam),
                        hgt = HIWORD(lParam);
            // In case 0 size sent at creation or minimize:
            if (wid*hgt > 0) {
                m_size = Vec2UI(wid,hgt);
                m_api.viewportDims.set(m_size);
                m_d3d->resize(m_size);
                winUpdateScreen();      // Consumers of m_api.viewportSize need this
            }
        }
        else if (msg == WM_PAINT) {
            PAINTSTRUCT ps;
            // Begin/End Paint required for D3D render or other windows don't render properly,
            // but swapBuffers not required.
            // Validates the invalid region. Doesn't erase background in this case since brush is NULL.
            BeginPaint(hwnd,&ps);
            render();
            // One customer had a Windows termination ('the program has stopped working and will now close' -
            // Win 10 Pro, NVIDIA GeForce GTX 760)
            // here, even though 'm_hdc' was valid and a face was already displayed on the screen (just once,
            // when the program first started). Buggy video driver (perhaps OpenGL specific).
            EndPaint(hwnd,&ps);
        }
        else if (msg == WM_MBUTTONDOWN) {
            if (wParam == (MK_SHIFT | MK_MBUTTON)) {
                Vec2I    pos(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
                const CtrlShiftMiddleClickAction & fn = m_api.shiftMiddleClickActionI.cref();
                if (fn) {
                    fn(m_size,pos,m_worldToD3ps);
                    winUpdateScreen();
                }
            }
        }
        else if ((msg == WM_LBUTTONDOWN) || (msg == WM_RBUTTONDOWN)) {
            m_lastPos = Vec2I(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
            // This can only be the case for WM_LBUTTONDOWN:
            if (wParam == (MK_SHIFT | MK_CONTROL | MK_LBUTTON)) {
                m_api.markSurfPoint(m_size,m_lastPos,m_worldToD3ps);
                winUpdateScreen();
            }
            if (wParam == (MK_SHIFT | MK_CONTROL | MK_RBUTTON)) {
                m_api.markVertex(m_size,m_lastPos,m_worldToD3ps);
                winUpdateScreen();
            }
            else if ((wParam == (MK_CONTROL | MK_LBUTTON)) || (wParam == (MK_CONTROL | MK_RBUTTON))) {
                m_api.ctlClick(m_size,m_lastPos,m_worldToD3ps);
            }
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
        else if ((msg == WM_LBUTTONUP) || (msg == WM_RBUTTONUP)) {
            // Only if both buttons are released:
            if ((wParam & (MK_LBUTTON | MK_RBUTTON)) == 0)
                ClipCursor(NULL);
        }
        else if (msg == WM_MOUSEMOVE) {
            Vec2I    pos = Vec2I(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
            Vec2I    delta = pos-m_lastPos;
            if (wParam == MK_LBUTTON) {
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
            else if (wParam == MK_RBUTTON) {
                m_api.scale(delta[1]);
                winUpdateScreen();
            }
            else if (wParam == (MK_RBUTTON | MK_SHIFT)) {
                const ShiftRightDragAction & fn = m_api.shiftRightDragActionI.cref();
                if (fn) {
                    fn(m_size,pos,m_worldToD3ps);
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
                const BothButtonsDragAction & fn = m_api.bothButtonsDragActionI.cref();
                if (fn) {
                    fn(false,delta);
                    winUpdateScreen();
                }
            }
            else if (wParam == (MK_LBUTTON | MK_RBUTTON | MK_SHIFT)) {
                const BothButtonsDragAction & fn = m_api.bothButtonsDragActionI.cref();
                if (fn) {
                    fn(true,delta);
                    winUpdateScreen();
                }
            }
            else if (wParam == (MK_LBUTTON | MK_SHIFT | MK_CONTROL)) {
                m_api.ctrlShiftLeftDrag(m_size,delta);
                winUpdateScreen();
            }
            else if (wParam == (MK_RBUTTON | MK_SHIFT | MK_CONTROL)) {
                m_api.ctrlShiftRightDrag(m_size,delta);
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
            m_d3d->reset();
            m_d3d.reset();
            if (m_hdc) {
                ReleaseDC(hwnd,m_hdc);
                m_hdc = NULL;
            }
        }
        else if (msg == WM_GESTURE){ 
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
        else
            return DefWindowProc(hwnd,msg,wParam,lParam);
        return 0;
    }

    void renderBackBuffer()
    {
        if (m_updateBgImg->checkUpdate())
            m_d3d->setBgImage(m_api.bgImg);
        const Camera &   camera = m_api.xform.cref();
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
        options.colorBySurface = m_api.colorBySurface.val();
        m_d3d->renderBackBuffer(
            m_api.bgImg,
            m_api.rendMeshesN.cref(),
            m_api.light.cref(),
            worldToD3vs,
            d3vsToD3ps,
            m_size,
            options);
    }

    void render()
    {
        renderBackBuffer();
        m_d3d->showBackBuffer();
    }

    ImgC4UC
    capture(Vec2UI dims)
    {
        m_d3d->resize(dims);
        m_api.viewportDims.set(dims);
        renderBackBuffer();
        ImgC4UC         ret = m_d3d->capture(dims);
        m_api.viewportDims.set(m_size);
        m_d3d->resize(m_size);
        return ret;
    }
};

GuiImplPtr
guiGetOsImpl(const Gui3d & def)
{return GuiImplPtr(new Gui3dWin(def)); }

}

// */
