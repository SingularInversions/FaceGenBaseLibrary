//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Feb 27, 2009
//

#include "stdafx.h"

#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "FgFileSystem.hpp"
#include "FgException.hpp"
#include "FgDiagnostics.hpp"
#include <boost/scoped_array.hpp>

using namespace std;

FgString
fgGetCurrentDir()
{
    char    buff[512] = {0};
    if (!getcwd(buff,511))
        fgThrow("Unable to get current working directory");
    FgString    ps(buff);
    if (!ps.empty() && !ps.endsWith("/"))
        ps += "/";
    return ps;
}

bool
fgSetCurrentDir(
    const FgString &    dir,
    bool                throwOnFail)
{
    string      sdir = dir.as_utf8_string();
    bool ret = (chdir(sdir.c_str()) == 0);
    if ((!ret) && throwOnFail)
        fgThrow("Unable to set current directory to",dir);
    return ret;
}

bool
fgCreateDirectory(const FgString & dir)
{
    string      sdir = dir.as_utf8_string();
    return (mkdir(sdir.c_str(),0777) == 0);
}

void
fgRemoveFile(const FgString & fname)
{
    string      utf8 = fname.as_utf8_string();
    if (remove(utf8.c_str()) != 0)
        fgThrow("Unable to remove file",fname);
}

void
fgDeleteFile(const FgString & fname)
{fgRemoveFile(fname); }

bool
fgRemoveDirectory(
    const FgString &    dir,
    bool                throwOnFail)
{
    string      sdir = dir.as_utf8_string();
    int         ret =  rmdir(sdir.c_str());
    if (ret == 0)
        return true;
    if (throwOnFail)
        fgThrow("Unable to remove directory",dir);
    return false;
}

FgString
fgDirSystemAppDataRoot()
{
    fgThrowNotImplemented();
    return FgString();
}

FgString
fgDirUserAppDataLocalRoot()
{
    fgThrowNotImplemented();
    return FgString();
}

FgString
fgDirUserAppDataRoamingRoot()
{
    fgThrowNotImplemented();
    return FgString();
}

FgString
fgUserDocumentsDirectory(bool)
{
    fgThrowNotImplemented();
    return FgString();
}

FgString
fgPublicDocumentsDirectory()
{
    fgThrowNotImplemented();
    return FgString();
}

struct  Dir
{
    Dir(const FgString & dirName)
    {handle = opendir(dirName.as_utf8_string().c_str()); }

    ~Dir()
    {if (handle != 0) closedir(handle); }

    DIR *   handle;
};

bool
fgCreationTime(const FgString &,uint64 &)
{
    fgThrowNotImplemented();
    return false;
}

void
fgMakeWritableByAll(const FgString &)
{fgThrowNotImplemented(); }

#if defined(__APPLE__)

#include <CoreFoundation/CFBundle.h>
#include <CoreFoundation/CFURL.h>

FgString
fgExecutablePath()
{
    CFURLRef bundleUrlRef(CFBundleCopyExecutableURL(CFBundleGetMainBundle()));
    FGASSERT(bundleUrlRef);
    boost::shared_ptr<const void> bundleUrl(bundleUrlRef,CFRelease);

    CFStringRef pathRef(CFURLCopyFileSystemPath(bundleUrlRef,kCFURLPOSIXPathStyle));
    FGASSERT(pathRef);
    boost::shared_ptr<const void> path(pathRef,CFRelease);

    size_t size = 2*(CFStringGetLength(pathRef)+1); // worst case for UTF8
    boost::scoped_array<char> buffer(new char[size]);

    FGASSERT(CFStringGetCString(pathRef,buffer.get(),size,kCFStringEncodingUTF8));

    return FgString(buffer.get()); // Conversion from utf8
}

#else

FgString
fgExecutablePath()
{
    std::ostringstream os;
    os.imbue(std::locale::classic());
    os << getpid();

    std::string proclink("/proc/" + os.str() + "/exe");
    FGASSERT(fgFileReadable(FgString(proclink)));
    char resolved_name[PATH_MAX] = {0};
    FGASSERT(realpath(proclink.c_str(),resolved_name));
    return FgString(resolved_name);
}

#endif

// */
