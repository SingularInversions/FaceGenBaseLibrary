//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
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
using boost::filesystem::is_directory;
using boost::filesystem::directory_iterator;

namespace Fg {

bool
isDirectory(Ustring const & name)
{return is_directory(name.ns()); }

DirectoryContents
directoryContents(Ustring const & dirName)
{
    Ustring        dn = dirName;
    if (dn.empty())     // Interpret this as current directory, which boost filesystem does not
        dn = Ustring(".");
    if (!is_directory(dn.ns()))
        fgThrow("Not a directory",dirName);
    DirectoryContents     ret;
    directory_iterator      it_end;
    for (directory_iterator it(dn.ns()); it != it_end; ++it) {
        if (is_directory(it->status()))
            ret.dirnames.push_back(it->path().filename().string());
        else if (is_regular_file(it->status()))
            ret.filenames.push_back(it->path().filename().string());
    }
    return ret;
}

Ustring
fgGetCurrentDir()
{
    char    buff[512] = {0};
    if (!getcwd(buff,511))
        fgThrow("Unable to get current working directory");
    Ustring    ps(buff);
    if (!ps.empty() && !ps.endsWith("/"))
        ps += "/";
    return ps;
}

bool
fgSetCurrentDir(
    Ustring const &    dir,
    bool                throwOnFail)
{
    string      sdir = dir.as_utf8_string();
    bool ret = (chdir(sdir.c_str()) == 0);
    if ((!ret) && throwOnFail)
        fgThrow("Unable to set current directory to",dir);
    return ret;
}

bool
fgCreateDirectory(Ustring const & dir)
{
    string      sdir = dir.as_utf8_string();
    return (mkdir(sdir.c_str(),0777) == 0);
}

void
deleteFile(Ustring const & fname)
{pathRemove(fname); }

bool
fgRemoveDirectory(
    Ustring const &    dir,
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

Ustring
fgDirSystemAppDataRoot()
{
    throw FgExceptionNotImplemented();
    return Ustring();
}

Ustring
fgDirSystemAppData(Ustring const &,Ustring const &)
{
    throw FgExceptionNotImplemented();
    return Ustring();
}

Ustring
fgDirUserAppDataLocalRoot()
{
    throw FgExceptionNotImplemented();
    return Ustring();
}

Ustring
fgDirUserAppDataRoamingRoot()
{
    throw FgExceptionNotImplemented();
    return Ustring();
}

Ustring
fgUserDocumentsDirectory(bool)
{
    throw FgExceptionNotImplemented();
    return Ustring();
}

Ustring
fgPublicDocumentsDirectory()
{
    throw FgExceptionNotImplemented();
    return Ustring();
}

struct  Dir
{
    Dir(Ustring const & dirName)
    {handle = opendir(dirName.as_utf8_string().c_str()); }

    ~Dir()
    {if (handle != 0) closedir(handle); }

    DIR *   handle;
};

bool
getCreationTime(Ustring const &,uint64 &)
{
    throw FgExceptionNotImplemented();
    return false;
}

std::time_t
getLastWriteTime(Ustring const & path)
{return boost::filesystem::last_write_time(path.ns()); }

void
fgMakeWritableByAll(Ustring const &)
{throw FgExceptionNotImplemented(); }

#if defined(__APPLE__)

#include <CoreFoundation/CFBundle.h>
#include <CoreFoundation/CFURL.h>

Ustring
fgExecutablePath()
{
    CFURLRef bundleUrlRef(CFBundleCopyExecutableURL(CFBundleGetMainBundle()));
    FGASSERT(bundleUrlRef);
    std::shared_ptr<const void> bundleUrl(bundleUrlRef,CFRelease);

    CFStringRef pathRef(CFURLCopyFileSystemPath(bundleUrlRef,kCFURLPOSIXPathStyle));
    FGASSERT(pathRef);
    std::shared_ptr<const void> path(pathRef,CFRelease);

    size_t size = 2*(CFStringGetLength(pathRef)+1); // worst case for UTF8
    boost::scoped_array<char> buffer(new char[size]);

    FGASSERT(CFStringGetCString(pathRef,buffer.get(),size,kCFStringEncodingUTF8));

    return Ustring(buffer.get()); // Conversion from utf8
}

#else

Ustring
fgExecutablePath()
{
    std::ostringstream os;
    os.imbue(std::locale::classic());
    os << getpid();

    std::string proclink("/proc/" + os.str() + "/exe");
    FGASSERT(fileReadable(Ustring(proclink)));
    char resolved_name[PATH_MAX] = {0};
    FGASSERT(realpath(proclink.c_str(),resolved_name));
    return Ustring(resolved_name);
}

#endif

}

// */
