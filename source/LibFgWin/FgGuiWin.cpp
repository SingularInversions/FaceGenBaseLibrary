//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
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
#include "FgGuiApi.hpp"
#include "FgGuiWin.hpp"
#include "FgThrowWindows.hpp"
#include "FgSystem.hpp"
#include "FgTime.hpp"
#include "FgScopeGuard.hpp"

using namespace std;

namespace Fg {

GuiWinStatics s_guiWin;

struct      GuiWinMain : GuiMainBase
{
    NPTF<String8>           titleNF;
    String8                 m_store;            // directory [and prefix] for state storage
    GuiImplPtr              m_win;
    Vec2UI                  m_size{0};          // Current size of client area (including maximization)
    Svec<GuiKeyHandle>      keyHandlers;
    function<void()>        onUpdate;           // Run on each screen update

    GuiWinMain(NPT<String8> const & t,String8 const & s,GuiImplPtr const & w) :
        titleNF{t}, m_store{s}, m_win{w}
    {}
/*
    map<UINT,uint64>            profile;        // Track time used processing each message

    ~GuiWinMain()
    {
        Svec<UINT>              msgs;
        Svec<uint64>            times;
        for (auto const & p : profile) {
            msgs.push_back(p.first);
            times.push_back(p.second);
        }
        Sizes                   perm = cReverse(sortInds(times));
        double                  tot = cSum(times);
        fgout << fgnl << "Time processing main WndProc messages: " << toStrPercent(tot/totalTime);
        for (size_t ii=0; ii<cMin(10,msgs.size()); ++ii) {
            size_t              idx = perm[ii];
            fgout << fgnl << "Msg: " << msgs[idx] << " time " << toStrPercent(times[idx]/tot);
        }
    }
*/
    void                updateGui() const
    {
    //fgout << fgnl << "s_guiWin.hwndMain: " << s_guiWin.hwndMain << flush;
        if (titleNF.checkUpdate()) {
            String8 const &     title = titleNF.node.val();
            SetWindowTextW(s_guiWin.hwndMain,title.as_wstring().c_str());
        }
        SendMessage(s_guiWin.hwndMain,WM_USER,0,0);
    //    LRESULT     ret = SendMessage(s_guiWin.hwndMain,WM_USER,0,0);
    //fgout << fgnl << "SendMessage returned: " << ret << flush;
    }

    LRESULT             wndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
    {
        LRESULT         ret = 0;
        Timer           timer;
        if (msg == WM_CREATE) {
//fgout << fgnl <<  "Main::WM_CREATE" << fgpush;
            m_win->create(hwnd,0,m_store+"_s");
            m_win->updateIfChanged();
//fgout << fgpop;
        }
        // Enforce minimum windows sizes. This only works on the main window so
        // we must calculate the minimum size from that of all sub-windows:
        else if (msg == WM_GETMINMAXINFO) {
            Vec2UI          min = m_win->getMinSize() + winNcSize(hwnd);
            MINMAXINFO *    mmi = (MINMAXINFO*)lParam;
            POINT           pnt;
            pnt.x = min[0];
            pnt.y = min[1];
            mmi->ptMinTrackSize = pnt;
        }
        else if (msg == WM_SIZE) {
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
            wchar_t         wkey = wchar_t(wParam);
            for (size_t ii=0; ii<keyHandlers.size(); ++ii) {
                GuiKeyHandle    kh = keyHandlers[ii];
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
            // GetWindowRect show the current window placement regardless of mazimization.
            // Its size is larger than m_size (by [16,39] on my system) presumably due to borders & title bar.
            RECT                rc;         // Current window placement
            FGASSERTWIN(GetWindowRect(hwnd,&rc));
            Mat22I              dimsCurr {rc.left,rc.right-rc.left,rc.top,rc.bottom-rc.top};
            // This function gives the window placement for the Win32 non-maximized state:
            WINDOWPLACEMENT     wp;
            wp.length = sizeof(wp);
            FGASSERTWIN(GetWindowPlacement(hwnd,&wp));  // Gets the non-maximezed window rect
            RECT const &        r = wp.rcNormalPosition;
            Mat22I              dimsNorm {r.left,r.right-r.left,r.top,r.bottom-r.top};
            bool                maximized = dimsCurr != dimsNorm;
            try {
                saveRaw(srlzText(dimsNorm),m_store+"main-dims.txt");
                saveRaw(srlzText(maximized),m_store+"main-maximized.txt");
            }
            catch (...) {}
            PostQuitMessage(0);     // Sends WM_QUIT which ends msg loop
        }
        //else if (msg == WM_MOUSEWHEEL) {
        //    Opt<HWND>     child = m_win->getHwnd();
        //    if (child.valid())
        //        SendMessage(child.val(),msg,wParam,lParam);
        //}
        else
            ret = DefWindowProc(hwnd,msg,wParam,lParam);
/*
        uint64          elapsed = timer.elapsedMilliseconds();
        auto            it = profile.find(msg);
        if (it == profile.end())
            profile[msg] = elapsed;
        else
            it->second += elapsed;
*/
        return ret;
    }
};

void                guiStartImpl(
    NPT<String8>                titleN,
    GuiPtr                      gui,
    String8 const &             storeDir,
    GuiOptions const &          options)
{
    FGASSERT(gui);
    s_guiWin.hinst = GetModuleHandle(NULL);
    // Initialize COM. This wasn't necessary on my computer but on some computers the call to CoCreateInstance
    // (for dialog box creation) failed with CO_E_NOTINITIALIZED (0x800401F0). I initially tried with the
    // COINIT_MULTITHREADED argument but this causes the app to hang when file dialog was called. Since I don't
    // use explicit multithreading in the GUI, one init suffices since all callbacks take place from the main thread.
    // I tested this by looking at the return value of CoInitializeEx(NULL,COINIT_APARTMENTTHREADED) where dialogs
    // are invoked (eg. worker func called from button callback) and it always returned S_FALSE indicated that
    // COM was already initialized:
    HRESULT             hr = CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);
    FGASSERTWIN((hr==S_OK)||(hr==S_FALSE));     // Repeat initialization is OK, even though it should not happen here
    ScopeGuard          sg {[](){CoUninitialize();}};
    GuiWinMain          win {titleN,storeDir+"Win",gui->getInstance()};
    win.keyHandlers = options.keyHandlers;
    win.onUpdate = options.onUpdate;
    s_guiWin.guiMainPtr = &win;
    // startTime = getTimeMs();
    // Load common controls DLL:
    INITCOMMONCONTROLSEX    icc;
    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC = ICC_BAR_CLASSES;
    InitCommonControlsEx(&icc);

    // posDims: col vec 0 is position (upper left corner in  windows screen coordinates),
    // col vec 1 is size. Windows screen coordinates:
    // x - right, y - down, origin - upper left corner of MAIN screen.
    // CW_USEDEFAULT doesn't pick the centre of the screen(s) because Microsoft.
    // TODO: use GetSystemMetrics(SM_CXSCREEN) to figure out screen size ... but multi-screens ...
    Mat22I          posDims {CW_USEDEFAULT,options.defaultSize[0],CW_USEDEFAULT,options.defaultSize[1]};
    // Retrieve previously saved main window position if file present and valid:
    try {
        Mat22I          pdTmp = dsrlzText<Mat22I>(loadRawString(win.m_store+"main-dims.txt"));
        Vec2I           pdAbs = mapAbs(pdTmp.subMatrix<2,1>(0,0));
        Vec2I           pdMin = Vec2I(win.m_win->getMinSize());
        if ((pdAbs[0] < 32000) &&   // Windows internal representation limits
            (pdAbs[1] < 32000) &&
            (pdTmp[1] >= pdMin[0]) && (pdTmp[1] < 32000) &&
            (pdTmp[3] >= pdMin[1]) && (pdTmp[3] < 32000))
            posDims = pdTmp;
    }
    catch (...) {}
    // TODO: Testing to see if the remembered window position is visible in a multi-monitor
    // setup is possible but I can't test it right now. Here are functions:
    // Get raw pixel area of primary screen not including taskbar or application toolbars:
    //RECT            screenArea;
    //SystemParametersInfoW(SPI_GETWORKAREA,0,&screenArea,0);
    // FG_HI4(screenArea.left,screenArea.right,screenArea.top,screenArea.bottom);
    // GetMonitorInfoW() for virtual screen coords of all monitors
    // https://stackoverflow.com/questions/18112616/how-do-i-get-the-dimensions-rect-of-all-the-screens-in-win32-api
    wchar_t constexpr   className[] = L"GuiWinMain";
    // The following will give us a handle to the current instance aka 'module',
    // which corresponds to the binary file in which this code is compiled
    // (ie. EXE or a DLL):
    WNDCLASSEX  wcl;
    wcl.cbSize = sizeof(wcl);
    if (GetClassInfoEx(s_guiWin.hinst,className,&wcl) == 0) {
        // 101 is the generated resource number of the icon images (if provided):
        HICON   icon = LoadIcon(s_guiWin.hinst,MAKEINTRESOURCE(101));
        if (icon == NULL)
            icon = LoadIcon(NULL,IDI_APPLICATION);
        // Redraw entire window if width or height is changed. This forces a background paint even when
        // hbrBackground is NULL, using the default brush:
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
//fgout << fgnl << "CreateWindowEx" << fgpush;
    // CreateWindowEx sends WM_CREATE and certain other messages before returning.
    // This is done so that the caller can send messages to the child window immediately
    // after calling this function. Note that the WM_CREATE handler sends 'updateWindow'
    // after creation, so that dynamic windows can be created and setting can be udpated:
    String8 const &     title = win.titleNF.val();
    s_guiWin.hwndMain =
        CreateWindowEx(0,
            className,
            title.as_wstring().c_str(),
            WS_OVERLAPPEDWINDOW,
            posDims[0],posDims[2],
            posDims[1],posDims[3],
            NULL,NULL,
            // Contrary to MSDN docs, this is used on all WinOSes to disambiguate
            // the class name over different modules [Raymond Chen]:
            s_guiWin.hinst,
            &win);      // Value to be sent as argument with WM_NCCREATE message
    FGASSERTWIN(s_guiWin.hwndMain);
//fgout << fgpop;

//fgout << fgnl << "ShowWindow";
    // Retrieve Win32 maximization state:
    bool                maximized = false;
    try {
        bool                maxTmp = dsrlzText<bool>(loadRawString(win.m_store+"main-maximized.txt"));
        maximized = maxTmp;
    }
    catch (...) {}
    // Set the currently selected windows to show, which also causes the WM_SIZE message
    // to be sent (and for the builtin controls, WM_PAINT):
    int             showState = maximized ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL;
    ShowWindow(s_guiWin.hwndMain,showState);
    // The first draw doesn't work properly without this; for some reason the initial
    // window isn't fully invalidated, especially within windows using win32 controls:
    InvalidateRect(s_guiWin.hwndMain,NULL,TRUE);
    MSG         msg;
    HANDLE      dummyEvent = INVALID_HANDLE_VALUE;
    HANDLE const *eventsPtr = (options.events.empty() ? &dummyEvent : &options.events[0].handle);
    for (;;) {
        BOOL        ret = MsgWaitForMultipleObjects(DWORD(options.events.size()),eventsPtr,FALSE,INFINITE,QS_ALLEVENTS);
        if (ret == WAIT_FAILED) {
            DWORD   err = GetLastError();
            fgout << fgnl << "MsgWaitForMultipleObjects failed with last error: " << err;
        }
        int         idx = int(ret) - int(WAIT_OBJECT_0);
//fgout << fgnl << "Event Handling: " << idx;
        if ((idx >= 0) && (idx < int(options.events.size()))) {
            options.events[idx].handler();
        }
        // Get all messages here since MsgWaitForMultipleObjects waits for NEW messages:
        while (PeekMessageW(&msg,NULL,0,0,PM_REMOVE)) {
            // WM_QUIT is only sent to main message loop after WM_DESTROY has been
            // sent and processed by all sub-windows:
            if (msg.message == WM_QUIT) {
                s_guiWin.hwndMain = 0;    // This value may be sent to dialog boxes as owner hwnd.
                return;
            }
            // Translates multi-key combos into appropriate unicode. Intercept application-wide key combos
            // before calling this:
            TranslateMessage(&msg);
//fgout << fgnl << "Msg: " << toHexString(uint16(msg.message));
            // this call takes all of the time in this loop:
            DispatchMessage(&msg);
        }
        // Update the GUI for any changes (triggers WM_PAINT) after message handling here,
        // and NOT inside the windows procedures, since the latter somehow causes 'DispatchMessage' to slow to
        // a crawl, to the point where WM_PAINT message don't even get received any more. There is no advantage
        // to it in any case, since we often can't know what's changed after using callbacks so have to do a full
        // check anyway.
        // Since the renderer just renders regardless rather than checking for a dirty flag (too many inputs to bother),
        // it's very important to NOT update the GUI when we get the paint message, since this will generate another
        // paint message from the renderer causing constant full GPU usage:
        if (msg.message != WM_PAINT)
            win.updateGui();
    }
}

Vec2I               winScreenPos(HWND hwnd,LPARAM lParam)
{
    POINT       coord;
    coord.x = GET_X_LPARAM(lParam);
    coord.y = GET_Y_LPARAM(lParam);
    FGASSERTWIN(ClientToScreen(hwnd,&coord));
    return Vec2I(coord.x,coord.y);
}

Vec2UI              winNcSize(HWND hwnd)
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

LRESULT             winCallCatch(Sfun<LRESULT(void)> func,string const & className)
{
    String8             msg,msgNative;
    try {return func(); }
    catch(FgException const & e) {
        msg = e.englishMessage();   // ends with two CRLFs
        msgNative = e.nativeMessage();
    }
    catch(std::bad_alloc const &)
    {
        msg = "OUT OF MEMORY";
#ifndef FG_64
        if (is64bitOS())
            msg += " (install 64-bit version if possible)";
#endif
        msg += "\n\n";
    }
    catch(std::exception const & e) {msg = String8(e.what()) + "\n\n"; }
    catch(...) { msg = "Unknown exception\n\n"; }
    msg += "While running winCallCatch for " + className;
    if (msgNative.empty())
        msgNative = msg;
    else
        msgNative += "While running winCallCatch for " + className;
    auto                diagnosticFn = [&]()
    {
        if (g_guiDiagHandler.reportError) {
            try {g_guiDiagHandler.reportError(msgNative); }
            catch(...) {}
        }
    };
    thread              diagThread {diagnosticFn};
    String8             caption = "ERROR in " + g_guiDiagHandler.appNameVerBits;
    guiDialogMessage(caption,msg);
    diagThread.join();
    return LRESULT(0);
}

void                setCursor(GuiCursor cursor)
{
    static Svec<pair<GuiCursor,LPWSTR>> toHandle {
        {GuiCursor::arrow,IDC_ARROW},
        {GuiCursor::wait,IDC_WAIT},
        {GuiCursor::translate,IDC_SIZEALL},
        {GuiCursor::grab,IDC_HAND},
        {GuiCursor::scale,IDC_SIZENS},
        {GuiCursor::rotate,IDC_NO},
        {GuiCursor::crosshair,IDC_CROSS},
    };
    LPWSTR          ch = lookupFirstL(toHandle,cursor);
    SetCursor(LoadCursorW(NULL,ch));
}

GuiClickState       clickStateFromWParam(WPARAM wp)
{
    return {
        (wp & MK_LBUTTON) != 0,
        (wp & MK_MBUTTON) != 0,
        (wp & MK_RBUTTON) != 0,
        (wp & MK_SHIFT)   != 0,
        (wp & MK_CONTROL) != 0,
    };
}

void                guiQuit()
{
    // When a WM_CLOSE message is generated by the user clicking the close 'X' button,
    // the default (DefWindowProc) is to do this, which generates a WM_DESTROY:
    DestroyWindow(s_guiWin.hwndMain);
}

void                guiBusyCursor()
{
    SetCursor(LoadCursorW(NULL,IDC_WAIT));
}

struct      GuiSpacerWin : public GuiBaseImpl
{
    GuiSpacer           m_api;

    GuiSpacerWin(const GuiSpacer & api) : m_api(api) {}

    virtual void        create(HWND,int,String8 const &,DWORD,bool) {}
    virtual void        destroy() {}
    virtual Vec2UI      getMinSize() const {return m_api.size; }
    virtual Arr2B       wantStretch() const {return Arr2B(false,false); }
    virtual void        updateIfChanged() {}
    virtual void        moveWindow(Vec2I,Vec2I) {}
    virtual void        showWindow(bool) {}
};

GuiImplPtr          guiGetOsImpl(const GuiSpacer & def) {return GuiImplPtr(new GuiSpacerWin(def)); }

}
