//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Feb 13, 2007
//

#include "stdafx.h"

#include "FgTypes.hpp"
#include "FgFileSystem.hpp"
#include "FgDiagnostics.hpp"
#include "FgStdString.hpp"
#include "FgThrowWindows.hpp"

using namespace std;

FgDirectoryContents
fgDirectoryContents(const FgString & dirName)
{
    FgPath              dir(dirName);
    if (!dir.base.empty()) {    // User didn't put delimiter after last dir
        dir.dirs.push_back(dir.base);
        dir.base.clear();
    }
    FgDirectoryContents ret;
    WIN32_FIND_DATAW	finddata;
    FgString            spec = dir.str() + "*";
    HANDLE hFind = FindFirstFileW(spec.as_wstring().c_str(),&finddata);
    if (hFind == INVALID_HANDLE_VALUE)
        return ret;
    do {
        if (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            wstring     name(finddata.cFileName);
            if ((name != L".") && (name != L".."))
                ret.dirnames.push_back(FgString(finddata.cFileName));
        }
        else
            ret.filenames.push_back(FgString(finddata.cFileName));
    } while	(FindNextFileW(hFind,&finddata) != 0);
    FindClose(hFind);
    return ret;
}

FgString
fgGetCurrentDir()
{
    wchar_t     buff[MAX_PATH]= {0};
    if (!GetCurrentDirectory(MAX_PATH,buff))
        fgThrowWindows("Unable to get current directory");
    return FgString(buff);
}

// We use OS-specific APIs for this one since boost::filesystem::current_directory
// doesn't return a success flag:
bool
fgSetCurrentDir(
    const FgString &    dir,
    bool                throwOnFail)
{
    wstring wdir = dir.as_wstring();
    bool ret = (SetCurrentDirectory(wdir.c_str()) != 0);
    if ((!ret) && throwOnFail)
        fgThrowWindows("Unable to set current directory to",dir);
    return ret;
}

void
fgRemoveFile(const FgString & fname)
{
    wstring wfname = fname.as_wstring();
    if (DeleteFile(wfname.c_str()) == 0)
        fgThrowWindows("Unable to delete file",fname);
}

// Deletes regardless of read-only or hidden flags, but will not delete system files.
void
fgDeleteFile(const FgString & fname)
{
    wstring wfname = fname.as_wstring();
    DWORD   attributes = GetFileAttributes(wfname.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES)
        fgThrowWindows("Unable to read attributes of file",fname);
    attributes = attributes & (!FILE_ATTRIBUTE_READONLY) & (!FILE_ATTRIBUTE_HIDDEN);
    if (SetFileAttributes(wfname.c_str(),attributes) == 0)
        fgThrowWindows("Unable to modify attributes of file to allow delete",fname);
    fgRemoveFile(fname);
}

bool
fgRemoveDirectory(
    const FgString &    dirname,
    bool                throwOnFail)
{
    wstring wdirname = dirname.as_wstring();
    int     ret = RemoveDirectory(wdirname.c_str());
    if (ret != 0)
        return true;
    if (throwOnFail)
        fgThrowWindows("Unable to remove directory",dirname);
    return false;
}

// Does a recursive deletion of a directory tree. We do not change the current directory as we
// recurse because this would not be thread safe.
void
fgDeleteDirectory(const FgString & dirname)
{
    FgDirectoryContents     dc = fgDirectoryContents(dirname);
    for (size_t ii=0; ii<dc.dirnames.size(); ii++)
        fgDeleteDirectory(dirname+"/"+dc.dirnames[ii]);
    for (size_t ii=0; ii<dc.filenames.size(); ii++)
        fgDeleteFile(dirname+"/"+dc.filenames[ii]);
    fgRemoveDirectory(dirname,true);
}

bool
fgCreateDirectory(const FgString & dirname)
{
    wstring     curr = fgGetCurrentDir().as_wstring(),
                dirn = dirname.as_wstring();
    if (CreateDirectoryExW(&curr[0],&dirn[0],NULL) == 0)
    {
        if (GetLastError() == ERROR_ALREADY_EXISTS) return false;
        else
            fgThrowWindows("Unable to create directory",dirname);
    }
    return true;
}

FgString
fgExecutablePath()
{
    wchar_t     module_name[MAX_PATH] = {0};
    GetModuleFileNameW(0,module_name,sizeof(module_name));
    return FgString(module_name);
}

FgString
fgDirSystemAppDataRoot()
{
    wchar_t     path[MAX_PATH];
    HRESULT     retval =
        SHGetFolderPathW(
            NULL,                                   // Handle not necessary
            CSIDL_COMMON_APPDATA,
            NULL,                                   // Current user
            SHGFP_TYPE_CURRENT,
            path);
    if (retval != S_OK)
        fgThrowWindows("Unable to retrieve an all-users application data directory");
    return FgString(path) + "\\";
}

FgString
fgDirUserAppDataRoamingRoot()
{
    wchar_t     path[MAX_PATH];
    HRESULT     retval =
        SHGetFolderPathW(
            NULL,                                   // Handle not necessary
            CSIDL_APPDATA,
            NULL,                                   // Current user
            SHGFP_TYPE_CURRENT,
            path);
    if (retval != S_OK)
        fgThrowWindows("Unable to retrieve user's roaming application data directory");
    return FgString(path) + "\\";
}

FgString
fgDirUserAppDataLocalRoot()
{
    wchar_t     path[MAX_PATH];
    HRESULT     retval =
        SHGetFolderPathW(
            NULL,                                   // Handle not necessary
            CSIDL_LOCAL_APPDATA,
            NULL,                                   // Current user
            SHGFP_TYPE_CURRENT,
            path);
    if (retval != S_OK)
        fgThrowWindows("Unable to retrieve user's local application data directory");
    return FgString(path) + "\\";
}

FgString
fgUserDocumentsDirectory()
{
    wchar_t     path[MAX_PATH];
    HRESULT     retval =
        SHGetFolderPathW(
            NULL,                                   // Handle not necessary
            CSIDL_MYDOCUMENTS,
            NULL,                                   // Current user
            SHGFP_TYPE_CURRENT,
            path);
    if (retval != S_OK)
        fgThrowWindows("Unable to retrieve user documents directory");
    return FgString(path) + "\\";
}

FgString
fgPublicDocumentsDirectory()
{
    wchar_t     path[MAX_PATH];
    HRESULT     retval =
        SHGetFolderPathW(
            NULL,                                   // Handle not necessary
            CSIDL_COMMON_DOCUMENTS,
            NULL,                                   // Current user
            SHGFP_TYPE_CURRENT,
            path);
    if (retval != S_OK)
        fgThrowWindows("Unable to retrieve public documents directory");
    return FgString(path) + "\\";
}

bool
fgFileCreationTime(const FgString & filename,uint64 & time)
{
    HANDLE hndl =
        CreateFile(
            filename.as_wstring().c_str(),
            GENERIC_READ,
            NULL,                           // exclusive access.
            NULL,                           // no security options
            OPEN_EXISTING,                  // do not create if not present.
            NULL,                           // no attribute options
            NULL);                          // don't get template info
    if (hndl == INVALID_HANDLE_VALUE)
        return false;
    FILETIME    creation,
                lastAccess,
                lastWrite;
    BOOL    ret = GetFileTime(hndl,&creation,&lastAccess,&lastWrite);
    CloseHandle(hndl);
    FGASSERT(ret != 0);
    time = uint64(creation.dwLowDateTime) +
           (uint64(creation.dwHighDateTime) << 32);
    return true;
}
