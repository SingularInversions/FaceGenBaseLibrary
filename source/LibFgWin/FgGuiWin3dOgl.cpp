//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     March 29, 2011
//

#include "stdafx.h"

#include "FgGuiApi3d.hpp"
#include "FgGuiWin.hpp"
#include "FgThrowWindows.hpp"
#include "Fg3dOpenGL.hpp"
#include "FgOpenGL.hpp"
#include "FgDefaultVal.hpp"

using namespace std;

struct  FgGuiWin3dOgl : public FgGuiOsBase
{
    HWND                    m_hwnd;
    FgGuiApi3d              m_api;
    FgVect2UI               m_size;
    HDC                     m_hdc;
    HGLRC                   m_hglrc;
    FgVect2I                m_lastPos;          // Last mouse position in CC (only valid if drag!=None)
    vector<vector<FgOglSurf> > m_oglImgs;       // by mesh, by image, < 0 for no texture
    ULONGLONG               m_lastGestureVal;   // Need to keep last gesture val to calc differences.
    FgValid<uint>           m_bgImgName;        // If valid, OGL 'name' of BG image
    FgVect2UI               m_bgImgDims;

    FgGuiWin3dOgl(const FgGuiApi3d & api)
    : m_api(api)
    {}

    virtual void
    create(HWND parentHwnd,int ident,const FgString &,DWORD extStyle,bool visible)
    {
        FgCreateChild   cc;
        cc.extStyle = extStyle;
        cc.visible = visible;
        cc.useFillBrush = false;
        fgCreateChild(parentHwnd,ident,this,cc);
    }

    virtual void
    destroy()
    {DestroyWindow(m_hwnd); }

    virtual FgVect2UI
    getMinSize() const
    {return FgVect2UI(400,500); }

    virtual FgVect2B
    wantStretch() const
    {return FgVect2B(true,true); }

    virtual void
    updateIfChanged()
    {
        if (g_gg.dg.update(m_api.updateFlagIdx)) {
            // This flips the dirty bit (QS_PAINT) for the render window but Windows will not
            // actually send a WM_PAINT message until the message queue is empty for a fraction
            // of a second (regardless of background paint flag):
            InvalidateRect(m_hwnd,NULL,FALSE);
            // So tell the system we want this updated immediately, for fluid interactive display:
            UpdateWindow(m_hwnd);
        }
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
            initOgl(hwnd);
#if (_MSC_VER >= 1600)  // Gestures not defined in VS08
            // The pinch-to-zoom gesture is enabled by default but not the rotate gesture, which we
            // must explicitly enable:
            GESTURECONFIG   config = {0};
            config.dwWant = GC_ROTATE;
            config.dwID = GID_ROTATE;
            config.dwBlock = 0;
            // This function returned a "not implemented" error on a German Windows 7 64bit SP1 system,
            // so don't throw on error, just continue and presumably no gesture messages will be received:
            SetGestureConfig(hwnd,0,1,&config,sizeof(GESTURECONFIG));
#endif
        }
        else if (msg == WM_SIZE) {
            int         sx = LOWORD(lParam),
                        sy = HIWORD(lParam);
            if (sx*sy > 0) {
                m_size = FgVect2UI(sx,sy);
                glViewport(0,0,m_size[0],m_size[1]);
                g_gg.setVal(m_api.viewportDims,m_size);
            }
        }
        else if (msg == WM_PAINT) {
            PAINTSTRUCT ps;
            // Validates the invalid region. Doesn't erase background
            // in this case since brush is NULL:
            BeginPaint(hwnd,&ps);
            render();
            // One customer had a Windows termination ('the program has stopped working and will now close' -
            // Win 10 Pro, NVIDIA GeForce GTX 760)
            // here, even though 'm_hdc' was valid and a face was already displayed on the screen (just once,
            // when the program first started). Buggy video driver (perhaps OpenGL specific).
            SwapBuffers(m_hdc);
            EndPaint(hwnd,&ps);
        }
        else if ((msg == WM_LBUTTONDOWN) || (msg == WM_RBUTTONDOWN)) {
            m_lastPos = FgVect2I(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
            // This can only be the case for WM_LBUTTONDOWN:
            if (wParam == (MK_SHIFT | MK_CONTROL | MK_LBUTTON)) {
                m_api.markSurfPoint(m_size,m_lastPos,fgOglTransform());
                g_gg.updateScreen();
            }
            if (wParam == (MK_SHIFT | MK_CONTROL | MK_RBUTTON)) {
                m_api.markVertex(m_size,m_lastPos,fgOglTransform());
                g_gg.updateScreen();
            }
            else if ((wParam == (MK_CONTROL | MK_LBUTTON)) || (wParam == (MK_CONTROL | MK_RBUTTON))) {
                m_api.ctlClick(m_size,m_lastPos,fgOglTransform());
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
            FgVect2I    pos = FgVect2I(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
            FgVect2I    delta = pos-m_lastPos;
            if (wParam == MK_LBUTTON) {
                m_api.panTilt(delta);
                g_gg.updateScreen();
            }
            else if (wParam == (MK_LBUTTON | MK_SHIFT)) {
                m_api.translate(delta);
                g_gg.updateScreen();
            }
            else if (wParam == MK_MBUTTON) {
                m_api.translate(delta);
                g_gg.updateScreen();
            }
            else if (wParam == MK_RBUTTON) {
                m_api.scale(delta[1]);
                g_gg.updateScreen();
            }
            else if (wParam == (MK_LBUTTON | MK_CONTROL)) {
                if (m_api.ctlDragAction) {
                    m_api.ctlDrag(true,m_size,delta,fgOglTransform());
                    g_gg.updateScreen();
                }
            }
            else if (wParam == (MK_RBUTTON | MK_CONTROL)) {
                if (m_api.ctlDragAction) {
                    m_api.ctlDrag(false,m_size,delta,fgOglTransform());
                    g_gg.updateScreen();
                }
            }
            else if (wParam == (MK_LBUTTON | MK_RBUTTON)) {
                if (m_api.bothButtonsDragAction) {
                    m_api.bothButtonsDragAction(false,delta);
                    g_gg.updateScreen();
                }
            }
            else if (wParam == (MK_LBUTTON | MK_RBUTTON | MK_SHIFT)) {
                if (m_api.bothButtonsDragAction) {
                    m_api.bothButtonsDragAction(true,delta);
                    g_gg.updateScreen();
                }
            }
            m_lastPos = pos;
        }
        else if (msg == WM_CAPTURECHANGED) {
            ClipCursor(NULL);
        }
        else if (msg == WM_DESTROY) {
            if (m_hglrc) {
                wglMakeCurrent(NULL,NULL);
                wglDeleteContext(m_hglrc);
                m_hglrc = NULL;
            }
            if (m_hdc) {
                ReleaseDC(hwnd,m_hdc);
                m_hdc = NULL;
            }
        }
#if (_MSC_VER >= 1600)  // Gestures not defined in VS08
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
                        g_gg.updateScreen();
                    }
                    m_lastGestureVal = gi.ullArguments;
                    return 0;
                }
                else if (gi.dwID == GID_ROTATE) {   // Enabled in WM_CREATE
                    if (m_lastGestureVal > 0) {
                        m_api.roll((int(gi.ullArguments)-int(m_lastGestureVal)) / 32);  // by experiment
                        g_gg.updateScreen();
                    }
                    m_lastGestureVal = gi.ullArguments;
                    return 0;
                }
            }
        }
#endif
        else
            return DefWindowProc(hwnd,msg,wParam,lParam);
        return 0;
    }

    void
    initOgl(HWND hwnd)
    {
        m_hdc = GetDC(hwnd);
        PIXELFORMATDESCRIPTOR pfd;
        ZeroMemory(&pfd,sizeof(pfd));
        pfd.nSize      = sizeof(pfd);
        pfd.nVersion   = 1;
        pfd.dwFlags    = PFD_DRAW_TO_WINDOW
                       | PFD_SUPPORT_OPENGL
                       | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.cAlphaBits = 8;
        pfd.cDepthBits = 24;
        pfd.cStencilBits = 8;
        int format = ChoosePixelFormat(m_hdc,&pfd);
        FGASSERTWIN(SetPixelFormat(m_hdc,format,&pfd));
        FGASSERTWIN(DescribePixelFormat(m_hdc,format,pfd.nSize,&pfd));
        m_hglrc = wglCreateContext(m_hdc);
        FGASSERTWIN(m_hglrc);
        FGASSERTWIN(wglMakeCurrent(m_hdc,m_hglrc));
        fgOglSetup();
    }

    void
    render()
    {
        if (m_api.bgImgN.valid()) {
            if (g_gg.dg.update(m_api.bgImgUpdateFlag)) {
                if (m_bgImgName.valid()) {
                    fgOglTexRelease(m_bgImgName.val());
                    m_bgImgName.invalidate();
                }
                const FgImgRgbaUb &     img = g_gg.getVal(m_api.bgImgN);
                if (!img.empty()) {
                    FGASSERT(fgIsPow2(img.dims()[0]));
                    FGASSERT(fgIsPow2(img.dims()[1]));
                    m_bgImgDims = g_gg.getVal(m_api.bgImgOrigDimsN);
                    m_bgImgName = fgOglTextureAdd(img);
                }
            }
        }
        const vector<Fg3dMesh> &        meshes = g_gg.getVal(m_api.meshesN);
        if (g_gg.dg.update(m_api.updateTexFlagIdx)) {
            // Release all textures:
            for (size_t ii=0; ii<m_oglImgs.size(); ++ii) {
                vector<FgOglSurf> &     tns = m_oglImgs[ii];
                for (size_t jj=0; jj<tns.size(); ++jj) {
                    if (tns[jj].valid()) {
                        fgOglTexRelease(tns[jj].name.val());
                        tns[jj].name.invalidate();
                    }
                }
            }
            // Load new ones:
            const vector<FgImgs> &      texss = g_gg.getVal(m_api.texssN);
            FGASSERT(texss.size() == meshes.size());
            m_oglImgs.resize(texss.size());
            for (size_t ii=0; ii<m_oglImgs.size(); ++ii) {
                const Fg3dMesh &        mesh = meshes[ii];
                const FgImgs &          texs = texss[ii];
                vector<FgOglSurf> &     tns = m_oglImgs[ii];
                tns.clear();
                for (size_t jj=0; jj<mesh.surfaces.size(); ++jj) {
                    FgOglSurf          oi;
                    if (jj < texs.size()) {
                        const FgImgRgbaUb & img = texs[jj];
                        if (!img.empty()) {
                            oi.name = fgOglTextureAdd(img);
                            if (fgUsesAlpha(img))
                                oi.transparency = true;
                        }
                    }
                    tns.push_back(oi);
                }
            }
        }
        fgOglSetLighting(g_gg.getVal(m_api.light));
        const vector<FgVerts> &         vertss = g_gg.getVal(m_api.vertssN);
        const vector<Fg3dNormals> &     normss = g_gg.getVal(m_api.normssN);
        vector<FgOglRendModel>          rendModels;
        for (size_t ii=0; ii<meshes.size(); ++ii) {
            FgOglRendModel                  rm;
            rm.mesh = &(meshes[ii]);
            rm.verts = &(vertss[ii]);
            rm.norms = &(normss[ii]);
            rm.oglImages = &(m_oglImgs[ii]);
            rendModels.push_back(rm);
        }
        const Fg3dCamera &          camera = g_gg.getVal(m_api.xform);
        const Fg3dRenderOptions &   render = g_gg.getVal(m_api.renderOptions);
        fgOglRender(
            rendModels,
            FgMat44F(camera.modelview.asHomogenous().transpose()),
            camera.frustum,
            render,
            m_bgImgName,
            m_bgImgDims);
    }
};

FgSharedPtr<FgGuiOsBase>
fgGuiGetOsInstance(const FgGuiApi3d & def)
{return FgSharedPtr<FgGuiOsBase>(new FgGuiWin3dOgl(def)); }

// */
