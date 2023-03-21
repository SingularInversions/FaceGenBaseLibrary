//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
//
// GuiBaseImpl child objects may or may not be win32 windows. This is important for avoiding
// excessive depth in the tree as win64 can choke on as little as 7 levels. Due to this,
// GuiBaseImpl child objects call a virtual function to create, move or hide/show children, which
// keep track of their own win32 handles.
//
// To avoid duplication, all windows sizing is done through 'moveWindow' which means that
// all windows are created with size 0 (during 'create') which means that all windows receive
// an initial size 0 WM_SIZE message which they must ignore or the initial (startup) display
// can be blank.
//
// Possible todo: switch to idiom of creating all child windows without visible flag, then
// calling ShowWindow after creation. Might avoid all the WM_SIZE(0,0) messages.
//

#ifndef FGGUIWIN_HPP
#define FGGUIWIN_HPP

#include "FgSerial.hpp"
#include "FgThrowWindows.hpp"
#include "FgMatrixC.hpp"
#include "FgSerial.hpp"

namespace Fg {

// Declared in LibFgBase but defined differently in each OS-specific library:
struct      GuiBaseImpl
{
    virtual ~GuiBaseImpl() {};

    // Recursively create the win32 window objects using the idiomatic approach of recursing
    // the creation call within the WM_CREATE handler.
    virtual void        create(
        HWND            hwndParent,
        int             ident,              // WinImpl window index
        String8 const & store,              // Root filename for storing state.
        DWORD           extStyle=NULL,
        bool            visible=true)       // Visible on creation ? See comments for 'showWindow' below.
        = 0;
    // Calls DestroyWindow. Used for dynamic windows - don't want this in destructor.
    virtual void        destroy() = 0;
    // Minimum size must be constant for an object - it cannot depend on dynamic content,
    // since otherwise updating content could force a resize of the entire window.
    // So for windows that contain dynamic sub-windows, we have a problem ...
    // Minimum sizes do not include any padding. It is the responsibility of the
    // parent container window to provide padding between elements:
    virtual Vec2UI      getMinSize() const = 0;
    // Do the window contents BENEFIT from stretching in the given dimensions ?
    // Any window size can always be expanded but the contents do not necessarily expand
    // to fill, or even if they do it may not improve anything (eg. a button) so this
    // propertly allows space to be allocated somewhere more beneficial (if possible):
    virtual Vec2B       wantStretch() const = 0;
    virtual void        updateIfChanged() = 0;
    virtual void        moveWindow(Vec2I lo,Vec2I sz) = 0;
    // Visibility is used by GuiSplitScrollWin to avoid drawing fully off-screen sub-windows.
    // The idea is that destroy/create of these windows would be slower and increase flicker,
    // although this has not been tested:
    virtual void        showWindow(bool) = 0;
    // We sometimes need to pass windows messages on to sub-windows (eg. WM_MOUSEWHEEL) so
    // we need to query their handle. TODO: This approach doesn't work unless we also pass
    // down the relative position of the cursor so that split windows know which handle to
    // return !
    virtual Opt<HWND>   getHwnd() {return Opt<HWND>(); }
};

struct  GuiMainBase
{
    virtual void        updateGui() const = 0;
};

struct      GuiWinStatics
{
    HINSTANCE           hinst;
    HWND                hwndMain;
    GuiMainBase const * guiMainPtr;

    GuiWinStatics() : hinst(0), hwndMain(0) {}
};

extern GuiWinStatics s_guiWin;

LRESULT             winCallCatch(std::function<LRESULT(void)> func,String const & className);

template<class WinImpl>
LRESULT CALLBACK    statWndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
    static String       className = typeid(WinImpl).name();
    if (message == WM_NCCREATE) {
        // The "this" pointer passed to CreateWindowEx is returned here in 'lParam'.
        // Save to the Windows instance user data for retrieval in later calls:
        SetWindowLongPtr(hwnd,0,(LONG_PTR)((LPCREATESTRUCT)lParam)->lpCreateParams);
    }
    // Get object pointer from hwnd:
    WinImpl *           wnd = (WinImpl *)GetWindowLongPtr(hwnd,0);
    if (wnd == 0)   // For before WM_NCCREATE
        return DefWindowProc(hwnd,message,wParam,lParam);
    else
        return winCallCatch(std::bind(&WinImpl::wndProc,wnd,hwnd,message,wParam,lParam),className);
}

struct      WinCreateChild
{
    DWORD           style;
    DWORD           extStyle;
    bool            useFillBrush;
    HCURSOR         cursor;
    // visible on creation ? Note that WS_VISIBLE is recursely applied in the negative case;
    // if a parent window is not visible, it's children are not visible:
    bool            visible;

    WinCreateChild() :
        style(NULL),
        extStyle(NULL),
        useFillBrush(true),
        cursor(LoadCursor(NULL,IDC_ARROW)),
        visible(true)
    {}
};

template<class ChildImpl>
HWND                winCreateChild(
    HWND                parentHwnd,
    // WinImpl window identifier. Must be unique per instantiation for given parent if messages from
    // child to parent need to be processed:
    size_t              ident,
    // Passed back in windows callback function and used to map to object's wndProc:
    ChildImpl *         thisPtr,
    WinCreateChild      opt)
{
    std::string         classNameA = typeid(ChildImpl).name();
    // Different class options mean different classes:
    classNameA += toStr(size_t(opt.cursor)) + "_" + toStr(size_t(opt.useFillBrush));
    std::wstring        className = String8(classNameA).as_wstring();
    FGASSERT(className.size() < 256);     // Windows limit
    WNDCLASSEX          wcl;
    wcl.cbSize = sizeof(wcl);
    if (GetClassInfoEx(s_guiWin.hinst,className.c_str(),&wcl) == 0) {
        wcl.style = CS_HREDRAW | CS_VREDRAW;
        wcl.lpfnWndProc = &statWndProc<ChildImpl>;
        wcl.cbClsExtra = 0;
        wcl.cbWndExtra = sizeof(void *);
        wcl.hInstance = s_guiWin.hinst;
        wcl.hIcon = NULL;
        wcl.hCursor = opt.cursor;
        // COLOR_BTNFACE matches background color used by windows for buttons & controls [Petzold 374].
        // Set to NULL in non-leaf windows to minimize flicker caused by erasing then redrawing.
        wcl.hbrBackground = opt.useFillBrush ? GetSysColorBrush(COLOR_BTNFACE) : NULL,
        wcl.lpszMenuName = NULL;
        wcl.lpszClassName = className.c_str();
        wcl.hIconSm = NULL;
        FGASSERTWIN(RegisterClassEx(&wcl));
    }
    // CreateWindowEx sends WM_CREATE and certain other messages before returning.
    // This is done so that the caller can send messages to the child window immediately
    // after calling this function.
    int                 flags = WS_CHILD | opt.style;   // WS_CHILD == WS_CHILDWINDOW
    // Parent must be visible for child to be visible. Idiomatic approach is to create without
    // visible flag, then call ShowWindow. If created with visible flag, windows sends a WM_SHOWWINDOW:
    if (opt.visible) flags = flags | WS_VISIBLE;
    HWND hwnd = CreateWindowEx(
        opt.extStyle,
        className.c_str(), 
        NULL,
        flags,
        // Zeros for position and size are the idiomatic way to create a child window, which
        // is strange because Windows sends a WM_SIZE message of zeros after creation, perhaps
        // I should be creating not visible then making visible after ?
        0,0,0,0,
        parentHwnd,
        (HMENU)ident,
        // Contrary to MSDN docs, this is used on all WinOSes to disambiguate the class name
        // over different modules [Raymond Chen]:
        s_guiWin.hinst,
        thisPtr);           // Value to be sent as argument with WM_NCCREATE message
    FGASSERTWIN(hwnd != 0);
    return hwnd;
}

template<class WinImpl>
HWND                winCreateDialog(
    String8 const &     title,
    HWND                ownerHwnd,
    WinImpl *           thisPtr)
{
    std::wstring        className = String8("FgDialogClass").as_wstring();
    WNDCLASSEX          wcl;
    wcl.cbSize = sizeof(wcl);
    if (GetClassInfoEx(s_guiWin.hinst,className.c_str(),&wcl) == 0) {
        wcl.style = NULL;
        wcl.lpfnWndProc = &statWndProc<WinImpl>;
        wcl.cbClsExtra = 0;
        wcl.cbWndExtra = sizeof(void *);
        wcl.hInstance = s_guiWin.hinst;
        wcl.hIcon = NULL;
        wcl.hCursor = LoadCursor(NULL,IDC_ARROW);
        wcl.hbrBackground = GetSysColorBrush(COLOR_BTNFACE),
        wcl.lpszMenuName = NULL;
        wcl.lpszClassName = className.c_str();
        wcl.hIconSm = NULL;
        FGASSERTWIN(RegisterClassEx(&wcl));
    }
    RECT                rect;
    GetWindowRect(ownerHwnd,&rect);
    int                 widOwner = rect.right - rect.left,
                        hgtOwner = rect.bottom - rect.top,
                        widDlg = 500,
                        hgtDlg = 200;
    // CreateWindowEx sends WM_CREATE and certain other messages before returning.
    // This is done so that the caller can send messages to the child window immediately
    // after calling this function.
    HWND hwnd = CreateWindowEx(
        NULL,
        className.c_str(), 
        title.as_wstring().c_str(),
            WS_CAPTION                  // The window has a border and title bar
            | WS_VISIBLE
            | WS_POPUP,                 // Necessary for the main window to regain focus after dialog closed
        rect.left + (widOwner-widDlg)/2,
        rect.top + (hgtOwner-hgtDlg)/2,
        widDlg,hgtDlg,
        ownerHwnd,
        NULL,               // Use class menu definition (ie none)
        s_guiWin.hinst,     // as above
        thisPtr);           // as above
    FGASSERTWIN(hwnd != 0);
    return hwnd;
}

Vec2I               winScreenPos(HWND hwnd,LPARAM lParam);
Vec2UI              winNcSize(HWND hwnd);
// Send WM_USER to main hwnd. TODO: make this NOT rely on a global !
void                winUpdateScreen();

}

#endif

// */
