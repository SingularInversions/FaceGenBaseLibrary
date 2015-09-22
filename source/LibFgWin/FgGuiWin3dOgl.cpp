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

struct  OglTexNames
{
    vector<uint>    names;

    OglTexNames() {}

    ~OglTexNames()
    {
        for (size_t ii=0; ii<names.size(); ++ii)
            fgOglTexRelease(names[ii]);
    }

    void
    add(const FgImgRgbaUb & img)
    {names.push_back(fgOglTextureAdd(img)); }
};

struct  FgGuiWin3dOgl : public FgGuiOsBase
{
    HWND                    m_hwnd;
    FgGuiApi3d              m_api;
    FgVect2UI               m_size;
    HDC                     m_hdc;
    HGLRC                   m_hglrc;
    FgVect2I                m_lastPos;      // Last mouse position in CC (only valid if drag!=None)
    vector<vector<int> >    m_texNames;     // by mesh, by image, < 0 for no texture

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
    wndProc(
        HWND    hwnd,
        UINT    message,
        WPARAM  wParam,
        LPARAM  lParam)
    {
        switch (message)
        {
            case WM_CREATE:
                m_hwnd = hwnd;
                initOgl(hwnd);
                return 0;
            case WM_SIZE:
                {
                    int         sx = LOWORD(lParam),
                                sy = HIWORD(lParam);
                    if (sx*sy > 0) {
                        m_size = FgVect2UI(sx,sy);
                        glViewport(0,0,m_size[0],m_size[1]);
                        g_gg.setVal(m_api.viewportDims,m_size);
                    }
                }
                return 0;
            case WM_PAINT:
                {
                    PAINTSTRUCT ps;
                    // Validates the invalid region. Doesn't erase background
                    // in this case since brush is NULL:
                    BeginPaint(hwnd,&ps);
                    render();
                    SwapBuffers(m_hdc);
                    EndPaint(hwnd,&ps);
                }
                return 0;
            case WM_LBUTTONDOWN:
            case WM_RBUTTONDOWN:
                {
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
                return 0;
            case WM_LBUTTONUP:
            case WM_RBUTTONUP:
                // Only if both buttons are released:
                if ((wParam & (MK_LBUTTON | MK_RBUTTON)) == 0)
                    ClipCursor(NULL);
                return 0;
            case WM_MOUSEMOVE:
                {
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
                    m_lastPos = pos;
                }
                return 0;
            case WM_CAPTURECHANGED:
                ClipCursor(NULL);
                return 0;
            case WM_DESTROY:
                if (m_hglrc) {
                    wglMakeCurrent(NULL,NULL);
                    wglDeleteContext(m_hglrc);
                    m_hglrc = NULL;
                }
                if (m_hdc) {
                    ReleaseDC(hwnd,m_hdc);
                    m_hdc = NULL;
                }
                return 0;
        }
        return DefWindowProc(hwnd,message,wParam,lParam);
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
    loadImages()
    {
        // Release all textures:
        for (size_t ii=0; ii<m_texNames.size(); ++ii) {
            const vector<int> &     tns = m_texNames[ii];
            for (size_t jj=0; jj<tns.size(); ++jj)
                if (tns[jj] >= 0)
                    fgOglTexRelease(tns[jj]);
        }
        // Load new ones:
        const vector<FgImgs> &      texss = g_gg.getVal(m_api.texssN);
        m_texNames.resize(texss.size());
        for (size_t ii=0; ii<m_texNames.size(); ++ii) {
            const FgImgs &          texs = texss[ii];
            vector<int> &           tns = m_texNames[ii];
            tns.clear();
            for (size_t jj=0; jj<texs.size(); ++jj) {
                if (texs[jj].empty())
                    tns.push_back(-1);
                else
                    tns.push_back(fgOglTextureAdd(texs[jj]));
            }
        }
    }

    void
    render()
    {
        if (g_gg.dg.update(m_api.updateTexFlagIdx)) {
            loadImages();
        }
        fgOglSetLighting(g_gg.getVal(m_api.light));
        const vector<Fg3dMesh> &        meshes = g_gg.getVal(m_api.meshesN);
        const vector<FgVerts> &         vertss = g_gg.getVal(m_api.vertssN);
        const vector<Fg3dNormals> &     normss = g_gg.getVal(m_api.normssN);
        vector<FgOglRendModel>          rendModels;
        for (size_t ii=0; ii<meshes.size(); ++ii) {
            FgOglRendModel                  rm;
            rm.mesh = &(meshes[ii]);
            rm.verts = &(vertss[ii]);
            rm.norms = &(normss[ii]);
            rm.texNames = &(m_texNames[ii]);
            rm.rendSurfs = vector<FgOglRendSurf>(meshes[ii].surfaces.size());
            rendModels.push_back(rm);
        }
        const Fg3dCamera &          camera = g_gg.getVal(m_api.xform);
        const Fg3dRenderOptions &   render = g_gg.getVal(m_api.renderOptions);
        fgOglRender(
            rendModels,
            FgMat44F(camera.modelview.asHomogenous().transpose()),
            camera.frustum,
            render);
    }
};

FgSharedPtr<FgGuiOsBase>
fgGuiGetOsInstance(const FgGuiApi3d & def)
{return FgSharedPtr<FgGuiOsBase>(new FgGuiWin3dOgl(def)); }

// */
