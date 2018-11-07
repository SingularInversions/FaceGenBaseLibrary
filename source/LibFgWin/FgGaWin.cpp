//
// Copyright (c) 2018 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     18.08.11
//
// We target D3D 11.0 (which uses DXGI 1.1) as it is the highest version mostly guaranteed in Windows 7
// (v11.1 API is avail for Win7 but not necessarily supported by hardware and doesn't have anything we need)

#include "stdafx.h"

#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>

#include "FgGaBase.hpp"
#include "FgMatrix.hpp"
#include "FgFileSystem.hpp"
#include "FgThrowWindows.hpp"
#include "FgMetaFormat.hpp"
#include "FgHex.hpp"
#include "FgSystemInfo.hpp"

//#pragma comment (lib,"d3d11.lib")
//#pragma comment (lib,"d3dcompiler.lib")

using namespace std;
using namespace DirectX;

namespace FgGa {
/*
HINSTANCE               g_hInst = nullptr;
HWND                    g_hWnd = nullptr;

struct  D3DDevice
{
    ID3D11Device*           devPtr = nullptr;
    ID3D11DeviceContext*    contextPtr = nullptr;
    IDXGISwapChain*         swapChainPtr = nullptr;

    explicit
    D3DDevice(FgVect2UI dims)
    {
        D3D_DRIVER_TYPE driverTypes[] = {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,       // High performance software renderer supported on Win8+ (only D3D10 on Win7)
            D3D_DRIVER_TYPE_REFERENCE,  // Reference software renderer is very slow.
        };
        HRESULT         hr = S_OK;
        for(UINT ii=0; ii<3; ++ii) {
            // Just because DirectX 11.0 API is present, doesn't necessarily mean that this hardware supports
            // that feature level:
            D3D_FEATURE_LEVEL       featureLevel = D3D_FEATURE_LEVEL_11_0;
            hr = D3D11CreateDevice(
                nullptr,                // Use the first video adapter (card) in EnumAdapters list
                driverTypes[ii],nullptr,0,
                &featureLevel,1,
                D3D11_SDK_VERSION, &devPtr,
                nullptr,    // Since we only gave one feature level choice, we don't need to know which was chosen.
                &contextPtr);
            if(SUCCEEDED(hr))
                break;
        }
        if(FAILED(hr))
            fgThrowWindows("Microsoft Direct3D 11 not found",hr);
        // Obtain DXGI factory from device
        IDXGIFactory1* dxgiFactory = nullptr; {
            IDXGIDevice* dxgiDevice = nullptr;
            hr = devPtr->QueryInterface( __uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice) );
            if (SUCCEEDED(hr)) {
                IDXGIAdapter* adapter = nullptr;
                hr = dxgiDevice->GetAdapter(&adapter);
                if (SUCCEEDED(hr)) {
                    hr = adapter->GetParent( __uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory) );
                    adapter->Release();
                }
                dxgiDevice->Release();
            }
        }
        FGASSERTWIN(!FAILED(hr));
        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferDesc.Width = dims[0];      // Don't know why D3D want size here AND during view setup...
        sd.BufferDesc.Height = dims[1];
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.BufferCount = 1;                 // Number of back buffers (so 2 including front buffer)
        sd.OutputWindow = g_hWnd;
        sd.Windowed = TRUE;
        hr = dxgiFactory->CreateSwapChain(devPtr,&sd,&swapChainPtr);
        // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
        dxgiFactory->MakeWindowAssociation( g_hWnd, DXGI_MWA_NO_ALT_ENTER );
        dxgiFactory->Release();
        FGASSERTWIN(!FAILED(hr));
    }

    ~D3DDevice()
    {
        if(swapChainPtr)    swapChainPtr->Release();
        if(contextPtr)      contextPtr->Release();
        if(devPtr)          devPtr->Release();
    }
};

struct  D3DView
{
    ID3D11RenderTargetView*     viewPtr = nullptr;

    D3DView(const D3DDevice & dev,FgVect2UI dims)
    {
        ID3D11Texture2D*    pBackBuffer = nullptr;
        HRESULT             hr;
        hr = dev.swapChainPtr->GetBuffer(0,__uuidof( ID3D11Texture2D ),reinterpret_cast<void**>(&pBackBuffer));
        FGASSERTWIN(!FAILED(hr));
        hr = dev.devPtr->CreateRenderTargetView(pBackBuffer,nullptr,&viewPtr);
        pBackBuffer->Release();
        FGASSERTWIN(!FAILED(hr));
        dev.contextPtr->OMSetRenderTargets( 1, &viewPtr, nullptr );
        D3D11_VIEWPORT vp;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        vp.Width = (FLOAT)dims[0];
        vp.Height = (FLOAT)dims[1];
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        dev.contextPtr->RSSetViewports( 1, &vp );
    }

    ~D3DView()
    {
        if(viewPtr) viewPtr->Release();
    }
};

void render(D3DDevice * dev,D3DView * view)
{
    ID3D11VertexShader*     g_pVertexShader = nullptr;
    ID3D11PixelShader*      g_pPixelShader = nullptr;
    ID3D11InputLayout*      g_pVertexLayout = nullptr;
    ID3D11Buffer*           g_pVertexBuffer = nullptr;

    // Compile the vertex shader
    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
    ID3DBlob* pErrorBlob = nullptr;
    ID3DBlob* pVSBlob = nullptr;
    std::string         src =  "float4 VS(float4 Pos : POSITION) : SV_POSITION{return Pos;}";
    HRESULT             hr =
        D3DCompile(&src[0],src.size(),nullptr,nullptr,nullptr,"VS","vs_4_0",dwShaderFlags,0,&pVSBlob,&pErrorBlob);
    FGASSERTWIN(!FAILED(hr));

	// Create the vertex shader
	hr = dev->devPtr->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pVertexShader );
	if( FAILED( hr ) )
	{	
		pVSBlob->Release();
        return;
	}

    // Input Layout:
    D3D11_INPUT_ELEMENT_DESC layout =
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	hr = dev->devPtr->CreateInputLayout(
        &layout,1,pVSBlob->GetBufferPointer(),pVSBlob->GetBufferSize(), &g_pVertexLayout);
	pVSBlob->Release();
	if(FAILED(hr))
        return;
    dev->contextPtr->IASetInputLayout( g_pVertexLayout );

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
    src =  "float4 PS(float4 Pos : SV_POSITION) : SV_Target{return float4(1.0f,1.0f,0.0f,1.0f); }";
    hr = D3DCompile(&src[0],src.size(),nullptr,nullptr,nullptr,"PS","ps_4_0",dwShaderFlags,0,&pPSBlob,&pErrorBlob);
    if( FAILED( hr ) ) {
        MessageBox( nullptr,L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
        return;
    }

	// Create the pixel shader
	hr = dev->devPtr->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShader );
	pPSBlob->Release();
    if( FAILED( hr ) )
        return;

    // Create vertex buffer
    FgVerts         verts = {{0.0f,0.5f,0.5f},{0.5f,-0.5f,0.5f},{-0.5f,-0.5f,0.5f}};
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = verts.size() * 12;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData = {};
    InitData.pSysMem = &verts[0][0];
    hr = dev->devPtr->CreateBuffer( &bd, &InitData, &g_pVertexBuffer );
    if( FAILED( hr ) )
        return;

    // Set vertex buffer
    UINT stride = 12;
    UINT offset = 0;
    dev->contextPtr->IASetVertexBuffers( 0, 1, &g_pVertexBuffer, &stride, &offset );

    // Set primitive topology
    dev->contextPtr->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    // Clear the back buffer 
    dev->contextPtr->ClearRenderTargetView(view->viewPtr, Colors::MidnightBlue );

    // Render a triangle
	dev->contextPtr->VSSetShader( g_pVertexShader, nullptr, 0 );
	dev->contextPtr->PSSetShader( g_pPixelShader, nullptr, 0 );
    dev->contextPtr->Draw( 3, 0 );

    // Present the information rendered to the back buffer to the front buffer (the screen)
    dev->swapChainPtr->Present( 0, 0 );

    if( g_pVertexBuffer ) g_pVertexBuffer->Release();
    if( g_pVertexLayout ) g_pVertexLayout->Release();
    if( g_pVertexShader ) g_pVertexShader->Release();
    if( g_pPixelShader ) g_pPixelShader->Release();
}

void CleanupDevice()
{
    //if(dev.contextPtr) dev.contextPtr->ClearState();
}

static wchar_t fgGuiWinMain[] = L"FgGuiWinMain";

FgVect2UI
fgNcSize(HWND hwnd)
{
    // Calculate how much space Windows is taking for the NC area.
    // I was unable to get similar values using GetSystemMetrics (eg. for
    // main windows NC size was (8,27) while GSM reported SM_C*BORDER=1
    // and SM_CYCAPTION=19
    RECT        rectW,rectC;
    FGASSERTWIN(GetWindowRect(hwnd,&rectW));
    FGASSERTWIN(GetClientRect(hwnd,&rectC));
    return
        FgVect2UI(
            (rectW.right-rectW.left)-(rectC.right-rectC.left),
            (rectW.bottom-rectW.top)-(rectC.bottom-rectC.top));
}

struct  FgGuiKeyHandle
{
    char        key;        // Only visible keys handled for now
    FgFunc      handler;
};

template<class WinImpl>
LRESULT CALLBACK
fgStatWndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
    static string   className = typeid(WinImpl).name();
    if (message == WM_NCCREATE) {
        // The "this" pointer passed to CreateWindowEx is returned here in 'lParam'.
        // Save to the Windows instance user data for retrieval in later calls:
        SetWindowLongPtr(hwnd,0,(LONG_PTR)((LPCREATESTRUCT)lParam)->lpCreateParams);
    }
    // Get object pointer from hwnd:
    WinImpl * wnd = (WinImpl *)GetWindowLongPtr(hwnd,0);
    if (wnd == 0)   // For before WM_NCCREATE
        return DefWindowProc(hwnd,message,wParam,lParam);
    else
        return fgWinCallCatch(std::bind(&WinImpl::wndProc,wnd,hwnd,message,wParam,lParam),className);
}

struct  FgGuiWinMain
{
    FgString                    m_title;
    FgString                    m_store;        // Base filename for state storage
    // Cache current pixel size from WM_SIZE messages here rather than calling GetClientRect every time:
    FgVect2UI                   m_size = FgVect2UI(0);      // Initial invalid value
    vector<HANDLE>              eventHandles;   // Client event handles to trigger message loop
    vector<FgFunc>              eventHandlers;  // Respective event handlers
    vector<FgGuiKeyHandle>      keyHandlers;
    unique_ptr<D3DDevice>       devPtr;
    unique_ptr<D3DView>         viewPtr;

    void
    start()
    {
        // The following will give us a handle to the current instance aka 'module',
        // which corresponds to the binary file in which this code is compiled
        // (ie. EXE or a DLL):
        WNDCLASSEX  wcl;
        wcl.cbSize = sizeof(wcl);
        if (GetClassInfoEx(g_hInst,fgGuiWinMain,&wcl) == 0) {
            // 101 is the fgb-generated resource number of the icon images (if provided):
            HICON   icon = LoadIcon(g_hInst,MAKEINTRESOURCE(101));
            if (icon == NULL)
                icon = LoadIcon(NULL,IDI_APPLICATION);
            wcl.style = CS_HREDRAW | CS_VREDRAW;
            wcl.lpfnWndProc = &fgStatWndProc<FgGuiWinMain>;
            wcl.cbClsExtra = 0;
            wcl.cbWndExtra = sizeof(void *);
            wcl.hInstance = g_hInst;
            wcl.hIcon = icon;
            wcl.hCursor = LoadCursor(NULL,IDC_ARROW);
            wcl.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
            wcl.lpszMenuName = NULL;
            wcl.lpszClassName = fgGuiWinMain;
            wcl.hIconSm = NULL;
            FGASSERTWIN(RegisterClassEx(&wcl));
        }
        // Retrieve previously saved window state if present and valid:
        // posDims: col vec 0 is position (upper left corner in  windows screen coordinates),
        // col vec 1 is size. Windows screen coordinates:
        // x - right, y - down, origin - upper left corner of MAIN screen.
        FgMat22I        posDims(CW_USEDEFAULT,1200,CW_USEDEFAULT,800),
                        pdTmp;
        if (fgLoadXml(m_store+".xml",pdTmp,false)) {
            FgVect2I    pdAbs = fgAbs(pdTmp.subMatrix<2,1>(0,0));
            FgVect2I    pdMin = FgVect2I(800,400);
            if ((pdAbs[0] < 32000) &&   // Windows internal representation limits
                (pdAbs[1] < 32000) &&
                (pdTmp[1] >= pdMin[0]) && (pdTmp[1] < 32000) &&
                (pdTmp[3] >= pdMin[1]) && (pdTmp[3] < 32000))
                posDims = pdTmp;
        }
//fgout << fgnl << "CreateWindowEx" << fgpush;
        // CreateWindowEx sends WM_CREATE and certain other messages before returning.
        // This is done so that the caller can send messages to the child window immediately
        // after calling this function. Note that the WM_CREATE handler sends 'updateWindow'
        // after creation, so that dynamic windows can be created and setting can be udpated:
        g_hWnd =
            CreateWindowEx(0,
                fgGuiWinMain,
                m_title.as_wstring().c_str(),
                WS_OVERLAPPEDWINDOW,
                posDims[0],posDims[2],
                posDims[1],posDims[3],
                NULL,NULL,
                // Contrary to MSDN docs, this is used on all WinOSes to disambiguate
                // the class name over different modules [Raymond Chen]:
                g_hInst,
                this);      // Value to be sent as argument with WM_NCCREATE message
        FGASSERTWIN(g_hWnd);
//fgout << fgpop;

//fgout << fgnl << "ShowWindow";
        // Set the currently selected windows to show, which also causes the WM_SIZE message
        // to be sent:
        ShowWindow(g_hWnd,SW_SHOWNORMAL);

        // Now that we have a size we can set up D3D:
        FGASSERT(m_size.cmpntsProduct() > 0);
        devPtr = unique_ptr<D3DDevice>(new D3DDevice(m_size));
        viewPtr = unique_ptr<D3DView>(new D3DView(*devPtr,m_size));

        // The first draw doesn't work properly without this; for some reason the initial
        // window isn't fully invalidated, especially within windows using win32 controls:
        InvalidateRect(g_hWnd,NULL,TRUE);
        MSG         msg;
        HANDLE      dummyEvent = INVALID_HANDLE_VALUE;
        HANDLE      *eventsPtr = (eventHandles.empty() ? &dummyEvent : &eventHandles[0]);
        for (;;) {
            // This blocking call waits for Windows messages but also allows for other messages such as
            // video frame events:
            BOOL        ret = MsgWaitForMultipleObjects(DWORD(eventHandles.size()),eventsPtr,FALSE,INFINITE,QS_ALLEVENTS);
            if (ret == WAIT_FAILED) {
                DWORD   err = GetLastError();
                fgout << fgnl << "MsgWaitForMultipleObjects failed with last error: " << err;
            }
            int         idx = int(ret) - int(WAIT_OBJECT_0);
//fgout << fgnl << "Event Handling: " << idx;
            if ((idx >= 0) && (idx < int(eventHandles.size()))) {
                eventHandlers[idx]();
//fgout << fgnl << "SendMessage WM_USER" << fgpush;
                //g_gg.updateScreen();
//fgout << fgpop;
            }
            // Get all messages here since MsgWaitForMultipleObjects waits for NEW messages:
            while (PeekMessageW(&msg,NULL,0,0,PM_REMOVE)) {
                // WM_QUIT is only sent to main message loop after WM_DESTROY has been
                // sent and processed by all sub-windows:
                if (msg.message == WM_QUIT)
                    return;
                // Translates multi-key combos into appropriate unicode. Intercept application-wide
                // special combos before calling this:
                TranslateMessage(&msg);
                // In the case of a window move or resize, this function DOES NOT RETURN once it
                // receives the button down message, until the move/resize is complete:
                DispatchMessage(&msg);
            }
        }

        CleanupDevice();

    }

    LRESULT
    wndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
    {
        if (msg == WM_CREATE) {
//fgout << fgnl <<  "Main::WM_CREATE" << fgpush;
            //m_win->create(hwnd,0,m_store+"_s");
            //m_win->updateIfChanged();
//fgout << fgpop;
        }
        // Enforce minimum windows sizes. This only works on the main window so
        // we must calculate the minimum size from that of all sub-windows:
        else if (msg == WM_GETMINMAXINFO) {
            FgVect2UI       min = FgVect2UI(800,400) + fgNcSize(hwnd);
            MINMAXINFO *    mmi = (MINMAXINFO*)lParam;
            POINT           pnt;
            pnt.x = min[0];
            pnt.y = min[1];
            mmi->ptMinTrackSize = pnt;
        }
        else if (msg == WM_SIZE) {
            uint            wid = LOWORD(lParam),
                            hgt = HIWORD(lParam);
            // Doesn't send 0 size at creation (like sub-windows), but:
            // Size zero is sent when window is minimized, which for unknown reasons causes windows
            // to not repaint after restore, even after resizing to non-zero and calling Invalidate
            // on entire window. So we must avoid zero sizing:
            if (wid*hgt > 0) {
                m_size[0] = wid;
                m_size[1] = hgt;
                if (devPtr) {       // Don't execute before D3D is set up:
                    // From: https://stackoverflow.com/questions/28095798/how-to-change-window-size-in-directx-11-desktop-application
                    HRESULT         hr = S_OK;
                    devPtr->contextPtr->OMSetRenderTargets(0,nullptr,nullptr);      // Unbind
                    if (viewPtr) viewPtr.reset();                                   // Clear viewport
                    devPtr->contextPtr->Flush();                                    // Wait for complete
                    hr = devPtr->swapChainPtr->ResizeBuffers(
                        1,                                      // We only have 1 buffer
                        m_size[0],m_size[1],
                        DXGI_FORMAT_UNKNOWN,                    // Don't change the format
                        0);
                    viewPtr = unique_ptr<D3DView>(new D3DView(*devPtr,m_size));     // New viewport
                }
            }
        }
        else if (msg == WM_PAINT) {
            PAINTSTRUCT     ps;
            HDC             hdc = BeginPaint( hwnd, &ps );
            render(devPtr.get(),viewPtr.get());
            EndPaint( hwnd, &ps );
        }
        // R. Chen: WM_WINDOWPOSCHANGED catches all possible size/move changes (added later).
        // Sent for each move and also for maximize/minimize/restore:
        //else if (msg == WM_MOVE)
        //    fgout << "WM_MOVE";
        // Only sent after initial showwindow call:
        //else if (msg == WM_SHOWWINDOW)
        //    fgout << "WM_SHOWWINDOW";
        //else if (msg == WM_QUERYOPEN)     // Queries window state before restore
        //    fgout << "WM_QUERYOPEN";
        else if (msg == WM_CHAR) {
            wchar_t     wkey = wchar_t(wParam);
            for (size_t ii=0; ii<keyHandlers.size(); ++ii) {
                FgGuiKeyHandle  kh = keyHandlers[ii];
                if (wkey == wchar_t(kh.key))
                    kh.handler();
            }
        }
        // This msg is sent by FgGuiGraph::updateScreen() which is called whenever an 
        // input has been changed:
        else if (msg == WM_USER) {
//fgout << fgnl << "Main::WM_USER " << flush << fgpush;
            //m_win->updateIfChanged();
//fgout << fgnl << "updateIfChanged done " << flush << fgpop;
        }
        else if (msg == WM_DESTROY) {       // User is closing application
            WINDOWPLACEMENT     wp;
            wp.length = sizeof(wp);
            // Don't use GetWindowRect here as it's affected by minimization and maximization:
            FGASSERTWIN(GetWindowPlacement(hwnd,&wp));
            const RECT &        rect = wp.rcNormalPosition;
            FgMat22I dims(rect.left,rect.right-rect.left,rect.top,rect.bottom-rect.top);
            fgSaveXml(m_store+".xml",dims,false);
            //m_win->saveState();
            PostQuitMessage(0);     // Sends WM_QUIT which ends msg loop
        }
        //else if (msg == WM_MOUSEWHEEL) {
        //    FgOpt<HWND>     child = m_win->getHwnd();
        //    if (child.valid())
        //        SendMessage(child.val(),msg,wParam,lParam);
        //}
        else
            return DefWindowProc(hwnd,msg,wParam,lParam);
        return 0;
    }
};

//void
//FgGuiGraph::updateScreenImpl()
//{
//    SendMessage(g_hWnd,WM_USER,0,0);
////    LRESULT     ret = SendMessage(g_hWnd,WM_USER,0,0);
////fgout << fgnl << "SendMessage returned: " << ret << flush;
//}
//
//void
//FgGuiGraph::quit()
//{
//    // When a WM_CLOSE message is generated by the user clicking the close 'X' button,
//    // the default (DefWindowProc) is to do this, which of course generates a WM_DESTROY:
//    DestroyWindow(g_hWnd);
//}
*/
void
startMain(
    WinPtr                  win,
    const FgString &        appNameVer)
{
    //FgString                title = "FgGaTest";
    //g_hInst = GetModuleHandle(NULL);
    //FgGuiWinMain    gui;
    //gui.m_title = title;
    //gui.m_store = fgDirUserAppDataLocalRoot() + title + "/win_";
    //gui.start();
    //g_hWnd = nullptr;       // This value may be sent to dialog boxes as owner hwnd.
}
/*
// Set up this data structure for application error handling (eg. report to server):
struct  FgGuiDiagHandler
{
    FgString                        appNameVer;     // Full name of application plus version
    // Client-defined error reporting. Can be null.
    // Accepts error message, returns true if reported, false otherwise (so default dialog can be shown):
    std::function<bool(FgString)> reportError;
    FgString                        reportSuccMsg;  // Displayed if 'reportError' returns true.
    // Prepended to error message and displayed if 'reportError' == NULL or 'reportError' returns false:
    FgString                        reportFailMsg;
};

FgGuiDiagHandler        g_guiDiagHandler;

void
fgGuiDialogMessage(
    const FgString & cap,
    const FgString & msg)
{
    MessageBox(
        // Sending the main window handle makes it the OWNER of this window (not parent since
        // this is not a child window but an actual window), which makes this a modal dialog:
        g_hWnd,
        msg.as_wstring().c_str(),
        cap.as_wstring().c_str(),
        MB_OK);
}

LRESULT
fgWinCallCatch(std::function<LRESULT(void)> func,const string & className)
{
    FgString    msg;
    try
    {
        return func();
    }
    catch(FgException const & e)
    {
        msg = "FG Exception: " + e.no_tr_message();
    }
    catch(std::bad_alloc const &)
    {
        msg = "OUT OF MEMORY ";
#ifndef FG_64
        if (fg64bitOS())
            msg += "(install 64-bit version if possible) ";
#endif
    }
    catch(std::exception const & e)
    {
        msg = "std::exception: " + FgString(e.what());
    }
    catch(...)
    {
        msg = "Unknown type: ";
    }
    msg = "ERROR @ fgWinCallCatch: " + msg;
    FgString        caption = "ERROR",
                    sysInfo;
    try
    {
        sysInfo = "\n" + g_guiDiagHandler.appNameVer + " " + fgBitsString() + "bit\n"
            + fgSystemInfo() + "\n" + className + "\n";
        if ((g_guiDiagHandler.reportError) && g_guiDiagHandler.reportError(msg+sysInfo))
            fgGuiDialogMessage(caption,g_guiDiagHandler.reportSuccMsg+"\n"+msg);
        else
            fgGuiDialogMessage(caption,g_guiDiagHandler.reportFailMsg+"\n"+msg+sysInfo);
        return LRESULT(0);
    }
    catch(FgException const & e)
    {
        msg += "FG Exception: " + e.no_tr_message();
    }
    catch(std::exception const & e)
    {
        msg += "std::exception: " + FgString(e.what());
    }
    catch(...)
    {
        msg += "Unknown type: ";
    }
    fgGuiDialogMessage(caption,g_guiDiagHandler.reportFailMsg+"\n"+msg+sysInfo);
    return LRESULT(0);
}
*/
}
