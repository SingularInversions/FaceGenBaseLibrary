//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Sept. 20, 2011
//

#include "stdafx.h"

#include "FgGuiApiDialogs.hpp"
#include "FgGuiWin.hpp"
#include "FgScopeGuard.hpp"

using namespace std;

void
fgGuiDialogMessage(
    const FgString & cap,
    const FgString & msg)
{
    MessageBox(
        // Sending the main window handle makes it the OWNER of this window (not parent since
        // this is not a child window but an actual window), which makes this a modal dialog:
        s_fgGuiWin.hwndMain,
        msg.as_wstring().c_str(),
        cap.as_wstring().c_str(),
        MB_OK);
}

static
void
pfdRelease(IFileDialog * pfd)
{pfd->Release(); }

FgValidVal<FgString>
fgGuiDialogFileLoad(
    const FgString &        description,
    const vector<string> &  extensions)
{
    FGASSERT(!extensions.empty());
    FgValidVal<FgString>    ret;
    HRESULT                 hr;
    IFileDialog *           pfd = NULL;
    hr = CoCreateInstance(CLSID_FileOpenDialog,NULL,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&pfd));
    FGASSERTWIN(SUCCEEDED(hr));
    FgScopeGuard            sg(boost::bind(pfdRelease,pfd));
    // Get existing (default) options to avoid overwrite:
    DWORD                   dwFlags;
    hr = pfd->GetOptions(&dwFlags);
    FGASSERTWIN(SUCCEEDED(hr));
    // Only want filesystem items; don't support other shell items:
    hr = pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);
    FGASSERTWIN(SUCCEEDED(hr));
    wstring                 desc = description.as_wstring(),
                            exts = L"*." + FgString(extensions[0]).as_wstring();
    for (size_t ii=1; ii<extensions.size(); ++ii)
        exts += L";*." + FgString(extensions[ii]).as_wstring();
    COMDLG_FILTERSPEC       fs;
    fs.pszName = desc.data();
    fs.pszSpec = exts.data();
    hr = pfd->SetFileTypes(1,&fs);
    FGASSERTWIN(SUCCEEDED(hr));
    // Set the selected file type index (starts at 1):
    hr = pfd->SetFileTypeIndex(1);
    FGASSERTWIN(SUCCEEDED(hr));
    hr = pfd->Show(s_fgGuiWin.hwndMain);    // Blocking call to display dialog
    if (SUCCEEDED(hr)) {                    // A filename was selected
        IShellItem *        psiResult;
        hr = pfd->GetResult(&psiResult);
        if (SUCCEEDED(hr)) {
            PWSTR           pszFilePath = NULL;
            hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH,&pszFilePath);
            if (SUCCEEDED(hr)) {
                ret = FgString(pszFilePath);
                CoTaskMemFree(pszFilePath);
            }
            psiResult->Release();
        }
    }
    return ret;
}

FgValidVal<FgString>
fgGuiDialogFileSave(
    const FgString &    description,
    const string &      extension)
{
    FGASSERT(!extension.empty());
    FgValidVal<FgString>    ret;
    HRESULT                 hr;
    IFileDialog *           pfd = NULL;
    hr = CoCreateInstance(CLSID_FileSaveDialog,NULL,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&pfd));
    FGASSERTWIN(SUCCEEDED(hr));
    FgScopeGuard            sg(boost::bind(pfdRelease,pfd));
    // Get existing (default) options to avoid overwrite:
    DWORD                   dwFlags;
    hr = pfd->GetOptions(&dwFlags);
    FGASSERTWIN(SUCCEEDED(hr));
    // Only want filesystem items; don't support other shell items:
    hr = pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);
    FGASSERTWIN(SUCCEEDED(hr));
    wstring                 desc = description.as_wstring(),
                            exts = L"*." + FgString(extension).as_wstring();
    COMDLG_FILTERSPEC       fs;
    fs.pszName = desc.data();
    fs.pszSpec = exts.data();
    hr = pfd->SetFileTypes(1,&fs);
    FGASSERTWIN(SUCCEEDED(hr));
    // Set the selected file type index (starts at 1):
    hr = pfd->SetFileTypeIndex(1);
    FGASSERTWIN(SUCCEEDED(hr));
    wstring                 ext = FgString(extension).as_wstring();
    hr = pfd->SetDefaultExtension(ext.c_str());
    hr = pfd->Show(s_fgGuiWin.hwndMain);    // Blocking call to display dialog
    if (SUCCEEDED(hr)) {                    // A filename was selected
        IShellItem *        psiResult;
        hr = pfd->GetResult(&psiResult);
        if (SUCCEEDED(hr)) {
            PWSTR           pszFilePath = NULL;
            hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH,&pszFilePath);
            if (SUCCEEDED(hr)) {
                ret = FgString(pszFilePath);
                CoTaskMemFree(pszFilePath);
            }
            psiResult->Release();
        }
    }
    return ret;
}

FgValidVal<FgString>
fgGuiDialogDirSelect()
{
    FgValidVal<FgString>    ret;
    HRESULT                 hr;
    IFileDialog *           pfd = NULL;
    hr = CoCreateInstance(CLSID_FileOpenDialog,NULL,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&pfd));
    FGASSERTWIN(SUCCEEDED(hr));
    FgScopeGuard            sg(boost::bind(pfdRelease,pfd));
    // Get existing (default) options to avoid overwrite:
    DWORD                   dwFlags;
    hr = pfd->GetOptions(&dwFlags);
    FGASSERTWIN(SUCCEEDED(hr));
    // Only want filesystem items; don't support other shell items:
    hr = pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM | FOS_PICKFOLDERS);
    FGASSERTWIN(SUCCEEDED(hr));
    hr = pfd->Show(s_fgGuiWin.hwndMain);    // Blocking call to display dialog
    if (SUCCEEDED(hr)) {                    // A filename was selected
        IShellItem *        psiResult;
        hr = pfd->GetResult(&psiResult);
        if (SUCCEEDED(hr)) {
            PWSTR           pszFilePath = NULL;
            hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH,&pszFilePath);
            if (SUCCEEDED(hr)) {
                FgString    str(pszFilePath);
                CoTaskMemFree(pszFilePath);
                // Ensure the string ends with a delimiter (Windows does not):
                vector<uint>    str2 = str.as_utf32();
                if (str2.back() != '\\')
                    str2.push_back('\\');
                ret = FgString(str2);
            }
            psiResult->Release();
        }
    }
    return ret;
}

static bool s_cancel;

static
bool
progress(HWND hwndPb,bool milestone)
{
    if (s_cancel)
        return true;
    if (milestone)
        SendMessage(hwndPb,PBM_STEPIT,0,0);

    // Message loop single pass:
    MSG         msg;
    while (PeekMessageW(&msg,NULL,0,0,PM_REMOVE)) {
        // WM_QUIT is only sent to main message loop after WM_DESTROY has been
        // sent and processed by all sub-windows:
        if (msg.message == WM_QUIT)
            return true;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return false;
}

struct  Dialog
{
    uint        progressSteps;
    HWND        hwndThis;
    HWND        hwndPb;
    HWND        hwndButton;

    LRESULT
    wndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
    {
        if (message == WM_CREATE) {
            hwndThis = hwnd;
            hwndPb =
                CreateWindowEx(
                    0,PROGRESS_CLASS,(LPTSTR)NULL,
                    WS_CHILD | WS_BORDER | WS_VISIBLE,
                    100,20,300,50,
                    hwndThis,(HMENU)1,s_fgGuiWin.hinst,NULL);
            FGASSERTWIN(hwndPb != 0);
            SendMessage(hwndPb,PBM_SETRANGE,0,MAKELPARAM(0,progressSteps));
            SendMessage(hwndPb,PBM_SETSTEP,(WPARAM)1,0);
            wstring     cancel = L"Cancel";
            hwndButton =
                CreateWindowEx(0,
                    TEXT("button"),     // Standard controls class name for all buttons
                    cancel.c_str(),
                    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                    100,120,300,20,     // Will be sent MOVEWINDOW messages.
                    hwnd,
                    HMENU(0),
                    s_fgGuiWin.hinst,
                    NULL);              // No WM_CREATE parameter
            FGASSERTWIN(hwndButton != 0);
            return 0;
        }
        else if (message == WM_COMMAND)
        {
            WORD    ident = LOWORD(wParam);
            WORD    code = HIWORD(wParam);
            if (code == 0) {
                FGASSERT(ident == 0);
                s_cancel = true;
            }
            return 0;
        }
        return DefWindowProc(hwnd,message,wParam,lParam);
    }
};

void
fgGuiDialogProgress(
    const FgString &        title,
    uint                    progressSteps,
    FgGuiActionProgress     actionProgress)
{
    Dialog      d;
    d.progressSteps = progressSteps;
    HWND        h = fgCreateDialog(title,s_fgGuiWin.hwndMain,&d);
    ShowWindow(h,SW_SHOWNORMAL);
    UpdateWindow(h);
    s_cancel = false;
    actionProgress(boost::bind(progress,d.hwndPb,_1));
    DestroyWindow(h);
}

// */
