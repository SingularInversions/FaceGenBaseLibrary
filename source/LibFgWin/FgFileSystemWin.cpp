//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
//

#include "stdafx.h"

#include "FgTypes.hpp"
#include "FgFileSystem.hpp"
#include "FgDiagnostics.hpp"
#include "FgStdString.hpp"
#include "FgThrowWindows.hpp"
#include "FgTime.hpp"

using namespace std;

namespace Fg {

bool
isDirectory(Ustring const & name)
{
    wstring         nameW = name.as_wstring();
    DWORD           attr = GetFileAttributesW(nameW.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES)    // Function failed - path probably doesn't exist.
        return false;
    return (attr & FILE_ATTRIBUTE_DIRECTORY);
}

// Can't use boost::filesystem as is_directory doesn't wok on Win 10 as of 18.04 update:
DirectoryContents
directoryContents(Ustring const & dirName)
{
    Path              dir(asDirectory(dirName));
    DirectoryContents ret;
    WIN32_FIND_DATAW	finddata;
    Ustring            spec = dir.str() + "*";
    HANDLE hFind = FindFirstFileW(spec.as_wstring().c_str(),&finddata);
    if (hFind == INVALID_HANDLE_VALUE)
        return ret;
    do {
        if (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            wstring     name(finddata.cFileName);
            if ((name != L".") && (name != L".."))
                ret.dirnames.push_back(Ustring(finddata.cFileName));
        }
        else
            ret.filenames.push_back(Ustring(finddata.cFileName));
    } while	(FindNextFileW(hFind,&finddata) != 0);
    FindClose(hFind);
    return ret;
}

Ustring
getCurrentDir()
{
    wchar_t     buff[MAX_PATH]= {0};
    if (!GetCurrentDirectory(MAX_PATH,buff))
        throwWindows("Unable to get current directory");
    Ustring    ps(buff);
    if (!ps.empty() && !ps.endsWith("\\"))
        ps += "\\";
    return ps;
}

// We use OS-specific APIs for this one since boost::filesystem::current_directory
// doesn't return a success flag:
bool
setCurrentDir(
    Ustring const &    dir,
    bool                throwOnFail)
{
    wstring wdir = dir.as_wstring();
    bool ret = (SetCurrentDirectory(wdir.c_str()) != 0);
    if ((!ret) && throwOnFail)
        throwWindows("Unable to set current directory to",dir);
    return ret;
}

// Deletes regardless of read-only or hidden flags, but will not delete system files.
void
deleteFile(Ustring const & fname)
{
    wstring wfname = fname.as_wstring();
    DWORD   attributes = GetFileAttributes(wfname.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES)
        throwWindows("Unable to read attributes of file",fname);
    attributes = attributes & (!FILE_ATTRIBUTE_READONLY) & (!FILE_ATTRIBUTE_HIDDEN);
    if (SetFileAttributes(wfname.c_str(),attributes) == 0)
        throwWindows("Unable to modify attributes of file to allow delete",fname);
    pathRemove(fname);
}

bool
removeDirectory(
    Ustring const &    dirname,
    bool                throwOnFail)
{
    wstring wdirname = dirname.as_wstring();
    int     ret = RemoveDirectory(wdirname.c_str());
    if (ret != 0)
        return true;
    if (throwOnFail)
        throwWindows("Unable to remove directory",dirname);
    return false;
}

bool
createDirectory(Ustring const & dirname)
{
    Ustring     templateDir;
    // Figure out the parent directory for the "template" (WTF):
    Path        path(asDirectory(dirname));
    if (path.dirs.size() > 1) {
        path.dirs.pop_back();
        templateDir = path.str();
    }
    else
        templateDir = getCurrentDir();  // Whatever
    wstring     td = templateDir.as_wstring(),
                dirn = dirname.as_wstring();
    if (CreateDirectoryExW(&td[0],&dirn[0],NULL) == 0) {
        if (GetLastError() == ERROR_ALREADY_EXISTS)
            return false;
        else
            throwWindows("Unable to create directory",dirname);
    }
    return true;
}

Ustring
getExecutablePath()
{
    wchar_t     module_name[MAX_PATH] = {0};
    GetModuleFileNameW(0,module_name,sizeof(module_name));
    return Ustring(module_name);
}

Ustring
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
        throwWindows("Unable to retrieve an all-users application data directory");
    return Ustring(path) + "\\";
}

// Return true if succeeded:
static bool
setAllUsersFullControl(Ustring const path)  // Directory or file
{
    FGASSERT(!path.empty());
    wstring                 pathw = path.as_wstring();
    LPCWSTR                 pszObjName = pathw.c_str();
    bool                    ret = true;
	DWORD                   dwRes = 0;
	PACL                    pOldDACL = NULL,
                            pNewDACL = NULL;
	PSECURITY_DESCRIPTOR    pSD = NULL;
	EXPLICIT_ACCESS         ea;
	PSID                    pSIDUsers = NULL;
	PACL                    pACL = NULL;
	SE_OBJECT_TYPE          ObjectType = SE_FILE_OBJECT;
	LPWSTR                  pszTrustee = NULL;
	TRUSTEE_FORM            TrusteeForm = TRUSTEE_IS_SID;
	DWORD                   dwAccessRights = FILE_ALL_ACCESS;
	ACCESS_MODE             AccessMode = SET_ACCESS;
	DWORD                   dwInheritance = OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE;
	SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;	
	WCHAR                   szInfo[300] = { 0 };
	try
	{
		// allocate Sid for BUILTIN/Users group
		if (!AllocateAndInitializeSid(
            &SIDAuthNT, 2,SECURITY_BUILTIN_DOMAIN_RID,DOMAIN_ALIAS_RID_USERS,0, 0, 0, 0, 0, 0,&pSIDUsers))
		{
			swprintf_s(szInfo,sizeof(szInfo)/sizeof(WCHAR), L"AllocateAndInitializeSid (Everyone) error %u", GetLastError());
			throw szInfo;
		}
		pszTrustee = (LPWSTR)pSIDUsers;
		// Get a pointer to the existing DACL.
		dwRes = GetNamedSecurityInfoW(
            pszObjName, ObjectType,DACL_SECURITY_INFORMATION,NULL, NULL, &pOldDACL, NULL, &pSD);
		if (ERROR_SUCCESS != dwRes) {
			swprintf_s(szInfo, sizeof(szInfo) / sizeof(WCHAR), L"GetNamedSecurityInfo Error %u\n", dwRes);
			throw szInfo;			
		}
		// Initialize an EXPLICIT_ACCESS structure for the new ACE. 
		ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
		ea.grfAccessPermissions = dwAccessRights;
		ea.grfAccessMode = AccessMode;
		ea.grfInheritance = dwInheritance;
		ea.Trustee.TrusteeForm = TrusteeForm;
		ea.Trustee.ptstrName = pszTrustee;
		// Create a new ACL that merges the new ACE
		// into the existing DACL.
		dwRes = SetEntriesInAclW(1, &ea, pOldDACL, &pNewDACL);
		if (ERROR_SUCCESS != dwRes) {
			swprintf_s(szInfo, sizeof(szInfo) / sizeof(WCHAR), L"SetEntriesInAcl Error %u\n", dwRes);
			throw szInfo;
		}
		// Attach the new ACL as the object's DACL.
		dwRes = SetNamedSecurityInfoW(
            (wchar_t*)pszObjName, ObjectType,DACL_SECURITY_INFORMATION,NULL, NULL, pNewDACL, NULL);
		if (ERROR_SUCCESS != dwRes) {
			swprintf_s(szInfo, sizeof(szInfo) / sizeof(WCHAR), L"SetNamedSecurityInfo Error %u\n", dwRes);
			throw szInfo;			
		}
	}
	catch (WCHAR*) {
		ret = false;
	}
	if (pSIDUsers)
		FreeSid(pSIDUsers);
	if (pACL)
		LocalFree(pACL);
	if (pSD != NULL)
		LocalFree((HLOCAL)pSD);
	if (pNewDACL != NULL)
		LocalFree((HLOCAL)pNewDACL);
	return ret;
}

Ustring
fgDirSystemAppData(Ustring const & groupName,Ustring const & appName)
{
    Ustring    appDir = fgDirSystemAppDataRoot() + groupName;
    if (!pathExists(appDir)) {
        createDirectory(appDir);
        // AllUsers by default has limited access to directories created within '\ProgramData', so
        // to ensure that license files can be deleted when necessary if the program is run by a 
        // different user we must ensure AllUsers has FullControl access. This is then inherited
        // by subdirectories and files within them (tested):
        setAllUsersFullControl(appDir.as_wstring().c_str());
    }
    appDir = appDir + fgDirSep() + appName;
    createDirectory(appDir);
    return appDir + fgDirSep();
}

Ustring
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
        throwWindows("Unable to retrieve user's roaming application data directory");
    return Ustring(path) + "\\";
}

Ustring
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
        throwWindows("Unable to retrieve user's local application data directory");
    return Ustring(path) + "\\";
}

Ustring
fgUserDocumentsDirectory(bool throwOnFail)
{
    wchar_t     path[MAX_PATH];
    HRESULT     retval =
        SHGetFolderPathW(
            NULL,                                   // Handle not necessary
            CSIDL_MYDOCUMENTS,
            NULL,                                   // Current user
            SHGFP_TYPE_CURRENT,
            path);
    if (retval != S_OK) {
        if (throwOnFail)
            throwWindows("Unable to retrieve user documents directory");
        else
            return Ustring();
    }
    return Ustring(path) + "\\";
}

Ustring
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
        throwWindows("Unable to retrieve public documents directory");
    return Ustring(path) + "\\";
}

bool
getCreationTime(Ustring const & path,uint64 & time)
{
    HANDLE hndl =
        CreateFile(
            path.as_wstring().c_str(),
            NULL,                           // Leaving zero means we're only getting meta-data about the file/dir
            NULL,                           // no need to specify share mode for meta-data
            NULL,                           // no security options
            OPEN_EXISTING,                  // Do not create
            FILE_FLAG_BACKUP_SEMANTICS,     // Must be so to work with directories
            NULL);                          // don't get template info
    if (hndl == INVALID_HANDLE_VALUE)
        return false;
    FILETIME    creation,
                lastAccess,
                lastWrite;
    BOOL    ret = GetFileTime(hndl,&creation,&lastAccess,&lastWrite);
    CloseHandle(hndl);
    FGASSERTWIN(ret);
    time = uint64(creation.dwLowDateTime) +
           (uint64(creation.dwHighDateTime) << 32);
    return true;
}

std::time_t
getLastWriteTime(Ustring const & fname)
{
    // Do NOT replace with boost::filesystem::last_write_time() which actually returns create time on Win.
    HANDLE hndl =
        CreateFile(
            fname.as_wstring().c_str(),
            NULL,                           // Leaving zero means we're only getting meta-data about the file/dir
            NULL,                           // no need to specify share mode for meta-data
            NULL,                           // no security options
            OPEN_EXISTING,                  // Do not create
            FILE_FLAG_BACKUP_SEMANTICS,     // Must be so to work with directories
            NULL);                          // don't get template info
    if (hndl == INVALID_HANDLE_VALUE)
        return false;
    FILETIME    creation,
                lastAccess,
                lastWrite;
    BOOL    ret = GetFileTime(hndl,&creation,&lastAccess,&lastWrite);
    CloseHandle(hndl);
    FGASSERTWIN(ret);
    return uint64(lastWrite.dwLowDateTime) +(uint64(lastWrite.dwHighDateTime) << 32);
}

}
