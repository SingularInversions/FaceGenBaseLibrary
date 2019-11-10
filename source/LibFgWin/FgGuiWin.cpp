//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
//
// DESIGN NOTES:
//
// Since Windows GUI doesn't draw to a backbuffer, flicker during resize is inevitable.
// This can in theory be minimized by 'WS_CLIPCHILDREN', however this flag also makes it
// effectively impossible to ensure that everything is redrawn properly. Again, in theory
// you could do so by recursively calling InvalidateWindow for all child windows, but this
// doesn't seem to work. These problem are mostly apparent in tab sub-windows.
//
// The standard way of handling WM_SIZE is to invalidate the window after resizing.
// Windows defers creating WM_PAINT messages a short while to avoid extra work from multiple
// invalidations or other changes. To bypass this when animation is required, use UpdateWindow
// for immediate redraw.
//
// When MoveWindow is called, Windows only generates a WM_SIZE message if there is a change.
// Calling a MoveWindow on itself doesn't work because the parent window itself hasn't moved and
// Windows will thus ignore the call.
//

#include "stdafx.h"

#include "FgMatrixC.hpp"
#include "FgGuiApiBase.hpp"
#include "FgGuiWin.hpp"
#include "FgThrowWindows.hpp"
#include "FgMetaFormat.hpp"
#include "FgHex.hpp"
#include "FgSystemInfo.hpp"
#include "FgGuiApiDialogs.hpp"

using namespace std;

namespace Fg {

GuiWinStatics s_guiWin;

struct  GuiWinMain
{
    Ustring                     m_title;
    Ustring                     m_store;        // Base filename for state storage
    GuiImplPtr                  m_win;
    Vec2UI                      m_size;         // Current size of client area
    vector<HANDLE>              eventHandles;   // Client event handles to trigger message loop
    vector<FgFnVoid2Void>       eventHandlers;  // Respective event handlers
    vector<GuiKeyHandle>        keyHandlers;
    function<void()>            onUpdate;       // Run on each screen update

    void
    start()
    {
        // Load common controls DLL:
        INITCOMMONCONTROLSEX    icc;
        icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
        icc.dwICC = ICC_BAR_CLASSES;
        InitCommonControlsEx(&icc);

        wchar_t constexpr   className[] = L"GuiWinMain";
        // The following will give us a handle to the current instance aka 'module',
        // which corresponds to the binary file in which this code is compiled
        // (ie. EXE or a DLL):
        WNDCLASSEX  wcl;
        wcl.cbSize = sizeof(wcl);
        if (GetClassInfoEx(s_guiWin.hinst,className,&wcl) == 0) {
            // 101 is the fgb-generated resource number of the icon images (if provided):
            HICON   icon = LoadIcon(s_guiWin.hinst,MAKEINTRESOURCE(101));
            if (icon == NULL)
                icon = LoadIcon(NULL,IDI_APPLICATION);
            wcl.style = CS_HREDRAW | CS_VREDRAW;
            wcl.lpfnWndProc = &statWndProc<GuiWinMain>;
            wcl.cbClsExtra = 0;
            wcl.cbWndExtra = sizeof(void *);
            wcl.hInstance = s_guiWin.hinst;
            wcl.hIcon = icon;
            wcl.hCursor = LoadCursor(NULL,IDC_ARROW);
            wcl.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
            wcl.lpszMenuName = NULL;
            wcl.lpszClassName = className;
            wcl.hIconSm = NULL;
            FGASSERTWIN(RegisterClassEx(&wcl));
        }
        // Retrieve previously saved window state if present and valid:
        // posDims: col vec 0 is position (upper left corner in  windows screen coordinates),
        // col vec 1 is size. Windows screen coordinates:
        // x - right, y - down, origin - upper left corner of MAIN screen.
        Mat22I        posDims(CW_USEDEFAULT,1400,CW_USEDEFAULT,900),
                        pdTmp;
        if (fgLoadXml(m_store+".xml",pdTmp,false)) {
            Vec2I    pdAbs = mapAbs(pdTmp.subMatrix<2,1>(0,0));
            Vec2I    pdMin = Vec2I(m_win->getMinSize());
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
        s_guiWin.hwndMain =
            CreateWindowEx(0,
                className,
                m_title.as_wstring().c_str(),
                WS_OVERLAPPEDWINDOW,
                posDims[0],posDims[2],
                posDims[1],posDims[3],
                NULL,NULL,
                // Contrary to MSDN docs, this is used on all WinOSes to disambiguate
                // the class name over different modules [Raymond Chen]:
                s_guiWin.hinst,
                this);      // Value to be sent as argument with WM_NCCREATE message
        FGASSERTWIN(s_guiWin.hwndMain);
//fgout << fgpop;

//fgout << fgnl << "ShowWindow";
        // Set the currently selected windows to show, which also causes the WM_SIZE message
        // to be sent (and for the builtin controls, WM_PAINT):
        ShowWindow(s_guiWin.hwndMain,SW_SHOWNORMAL);
        // The first draw doesn't work properly without this; for some reason the initial
        // window isn't fully invalidated, especially within windows using win32 controls:
        InvalidateRect(s_guiWin.hwndMain,NULL,TRUE);
        MSG         msg;
        HANDLE      dummyEvent = INVALID_HANDLE_VALUE;
        HANDLE      *eventsPtr = (eventHandles.empty() ? &dummyEvent : &eventHandles[0]);
        for (;;) {
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
                winUpdateScreen();
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
//fgout << fgnl << "Message Dispatch: " << fgAsHex(msg.message);
                DispatchMessage(&msg);
            }
        }
    }

    LRESULT
    wndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
    {
        if (msg == WM_CREATE) {
//fgout << fgnl <<  "Main::WM_CREATE" << fgpush;
            m_win->create(hwnd,0,m_store+"_s");
            m_win->updateIfChanged();
//fgout << fgpop;
        }
        // Enforce minimum windows sizes. This only works on the main window so
        // we must calculate the minimum size from that of all sub-windows:
        else if (msg == WM_GETMINMAXINFO) {
            Vec2UI       min = m_win->getMinSize() + winNcSize(hwnd);
            MINMAXINFO *    mmi = (MINMAXINFO*)lParam;
            POINT           pnt;
            pnt.x = min[0];
            pnt.y = min[1];
            mmi->ptMinTrackSize = pnt;
        }
        else if (msg == WM_SIZE) {  // Doesn't send 0 size like sub-windows get at creation.
            m_size[0] = LOWORD(lParam);
            m_size[1] = HIWORD(lParam);
            // Size zero is sent when window is minimized, which for unknown reasons causes windows
            // to not repaint after restore, even after resizing to non-zero and calling Invalidate
            // on entire window. So we must avoid zero sizing:
            if (m_size[0]*m_size[1] > 0) {
//fgout << fgnl <<  "Main::WM_SIZE: " << m_size << fgpush;
                m_win->moveWindow(Vec2I(0),Vec2I(m_size));
//fgout << fgpop;
            }
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
                GuiKeyHandle  kh = keyHandlers[ii];
                if (wkey == wchar_t(kh.key))
                    kh.handler();
            }
        }
        // This msg is sent by updateScreen():
        else if (msg == WM_USER) {
            if (onUpdate)
                onUpdate();
//fgout << fgnl << "Main::WM_USER " << flush << fgpush;
            m_win->updateIfChanged();
//fgout << fgnl << "updateIfChanged done " << flush << fgpop;
        }
        else if (msg == WM_DESTROY) {       // User is closing application
            WINDOWPLACEMENT     wp;
            wp.length = sizeof(wp);
            // Don't use GetWindowRect here as it's affected by minimization and maximization:
            FGASSERTWIN(GetWindowPlacement(hwnd,&wp));
            const RECT &        rect = wp.rcNormalPosition;
            Mat22I dims(rect.left,rect.right-rect.left,rect.top,rect.bottom-rect.top);
            fgSaveXml(m_store+".xml",dims,false);
            m_win->saveState();
            PostQuitMessage(0);     // Sends WM_QUIT which ends msg loop
        }
        //else if (msg == WM_MOUSEWHEEL) {
        //    Opt<HWND>     child = m_win->getHwnd();
        //    if (child.valid())
        //        SendMessage(child.val(),msg,wParam,lParam);
        //}
        else
            return DefWindowProc(hwnd,msg,wParam,lParam);
        return 0;
    }
};

void
guiStartImpl(
    Ustring const &                title,
    GuiPtr                        gui,
    Ustring const &                store,
    GuiOptions const &            options)
{
    s_guiWin.hinst = GetModuleHandle(NULL);
    GuiWinMain        win;
    win.m_title = title;
    win.m_store = store + "Win";
    win.m_win = gui->getInstance();
    for (GuiEvent const & event : options.events) {
        win.eventHandles.push_back(event.handle);
        win.eventHandlers.push_back(event.handler);
    }
    win.keyHandlers = options.keyHandlers;
    win.onUpdate = options.onUpdate;
    win.start();
    s_guiWin.hwndMain = 0;    // This value may be sent to dialog boxes as owner hwnd.
}

Vec2I
winScreenPos(HWND hwnd,LPARAM lParam)
{
    POINT       coord;
    coord.x = GET_X_LPARAM(lParam);
    coord.y = GET_Y_LPARAM(lParam);
    FGASSERTWIN(ClientToScreen(hwnd,&coord));
    return Vec2I(coord.x,coord.y);
}

Vec2UI
winNcSize(HWND hwnd)
{
    // Calculate how much space Windows is taking for the NC area.
    // I was unable to get similar values using GetSystemMetrics (eg. for
    // main windows NC size was (8,27) while GSM reported SM_C*BORDER=1
    // and SM_CYCAPTION=19
    RECT        rectW,rectC;
    FGASSERTWIN(GetWindowRect(hwnd,&rectW));
    FGASSERTWIN(GetClientRect(hwnd,&rectC));
    return
        Vec2UI(
            (rectW.right-rectW.left)-(rectC.right-rectC.left),
            (rectW.bottom-rectW.top)-(rectC.bottom-rectC.top));
}

LRESULT
winCallCatch(std::function<LRESULT(void)> func,const string & className)
{
    Ustring    msg;
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
        msg = "std::exception: " + Ustring(e.what());
    }
    catch(...)
    {
        msg = "Unknown type: ";
    }
    msg = "ERROR @ winCallCatch: " + msg;
    Ustring        caption = "ERROR",
                    sysInfo;
    try
    {
        sysInfo = "\n" + g_guiDiagHandler.appNameVer + " " + fgBitsString() + "bit\n"
            + fgOsName() + "\n" + className + "\n";
        if ((g_guiDiagHandler.reportError) && g_guiDiagHandler.reportError(msg+sysInfo))
            guiDialogMessage(caption,g_guiDiagHandler.reportSuccMsg+"\n"+msg);
        else
            guiDialogMessage(caption,g_guiDiagHandler.reportFailMsg+"\n"+msg+sysInfo);
        return LRESULT(0);
    }
    catch(FgException const & e)
    {
        msg += "FG Exception: " + e.no_tr_message();
    }
    catch(std::exception const & e)
    {
        msg += "std::exception: " + Ustring(e.what());
    }
    catch(...)
    {
        msg += "Unknown type: ";
    }
    guiDialogMessage(caption,g_guiDiagHandler.reportFailMsg+"\n"+msg+sysInfo);
    return LRESULT(0);
}

void winUpdateScreen()
{
//fgout << fgnl << "s_guiWin.hwndMain: " << s_guiWin.hwndMain << flush;
    SendMessage(s_guiWin.hwndMain,WM_USER,0,0);
//    LRESULT     ret = SendMessage(s_guiWin.hwndMain,WM_USER,0,0);
//fgout << fgnl << "SendMessage returned: " << ret << flush;
}

void
guiQuit()
{
    // When a WM_CLOSE message is generated by the user clicking the close 'X' button,
    // the default (DefWindowProc) is to do this, which generates a WM_DESTROY:
    DestroyWindow(s_guiWin.hwndMain);
}

}
