//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#if defined(__APPLE__)
#include <CoreFoundation/CFBundle.h>
#include <CoreFoundation/CFURL.h>
#endif

#include "FgFileSystem.hpp"
#include "FgSerial.hpp"

using namespace std;
using std::filesystem::is_directory;
using std::filesystem::directory_iterator;

namespace Fg {

bool                isDirectory(String8 const & name)
{
    return is_directory(name.ns());
}

DirContents         getDirContents(String8 const & dirName,bool includeDot)
{
    String8             dn = dirName;
    if (dn.empty())     // Interpret this as current directory, which filesystem does not
        dn = String8(".");
    if (!is_directory(dn.ns()))
        fgThrow("Not a directory",dirName);
    DirContents   ret;
    directory_iterator  it_end;
    for (directory_iterator it(dn.ns()); it != it_end; ++it) {
        if (is_directory(it->status())) {
            String              pathName = it->path().filename().string();
            if ((pathName[0] != '.') || includeDot)
                ret.dirnames.push_back(pathName);
        }
        else if (is_regular_file(it->status())) {
            String              pathName = it->path().filename().string();
            if ((pathName[0] != '.') || includeDot)
                ret.filenames.push_back(pathName);
        }
    }
    return ret;
}

String8             getCurrentDir()
{
    char    buff[512] = {0};
    if (!getcwd(buff,511))
        fgThrow("Unable to get current working directory");
    String8    ps(buff);
    if (!ps.empty() && !ps.endsWith("/"))
        ps += "/";
    return ps;
}

bool                setCurrentDir(
    String8 const &     dir,
    bool                throwOnFail)
{
    string      sdir = dir.as_utf8_string();
    bool ret = (chdir(sdir.c_str()) == 0);
    if ((!ret) && throwOnFail)
        fgThrow("Unable to set current directory to",dir);
    return ret;
}

bool                createDirectory(String8 const & dir)
{
    string      sdir = dir.as_utf8_string();
    return (mkdir(sdir.c_str(),0777) == 0);
}

void                deleteFile(String8 const & fname)
{
    pathRemove(fname);
}

bool                removeDirectory(
    String8 const &     dir,
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

String8             getDirSystemAppData()
{
    throw FgExceptionNotImplemented();
    return String8();
}

String8             getDirSystemAppData(String8 const &,String8 const &)
{
    throw FgExceptionNotImplemented();
    return String8();
}

String8             getDirUserAppDataLocal()
{
    throw FgExceptionNotImplemented();
    return String8();
}

String8             getDirUserAppDataRoaming()
{
    throw FgExceptionNotImplemented();
    return String8();
}

String8             getUserDocsDir(bool)
{
    throw FgExceptionNotImplemented();
    return String8();
}

String8             getPublicDocsDir()
{
    throw FgExceptionNotImplemented();
    return String8();
}

bool                getCreationTimePrecise(String8 const &,uint64 &)
{
    throw FgExceptionNotImplemented();
    return false;
}

uint64              getCreationTime(String8 const & path)
{
    uint64          tn;
    FGASSERT(getCreationTimePrecise(path,tn));
    return tn;
}

uint64              getLastWriteTime(String8 const &)
{
    throw FgExceptionNotImplemented();
    return 0;
}

#if defined(__APPLE__)

String8             getExecutablePath()
{
    CFURLRef bundleUrlRef(CFBundleCopyExecutableURL(CFBundleGetMainBundle()));
    FGASSERT(bundleUrlRef);
    std::shared_ptr<const void> bundleUrl(bundleUrlRef,CFRelease);

    CFStringRef pathRef(CFURLCopyFileSystemPath(bundleUrlRef,kCFURLPOSIXPathStyle));
    FGASSERT(pathRef);
    std::shared_ptr<const void> path(pathRef,CFRelease);

    size_t size = 2*(CFStringGetLength(pathRef)+1); // worst case for UTF8
    vector<char>        buffer(size);

    FGASSERT(CFStringGetCString(pathRef,buffer.data(),size,kCFStringEncodingUTF8));

    return String8(buffer.data()); // Conversion from utf8
}

#else

String8             getExecutablePath()
{
    std::ostringstream os;
    os.imbue(std::locale::classic());
    os << getpid();

    std::string proclink("/proc/" + os.str() + "/exe");
    FGASSERT(fileReadable(String8(proclink)));
    char resolved_name[PATH_MAX] = {0};
    FGASSERT(realpath(proclink.c_str(),resolved_name));
    return String8(resolved_name);
}

#endif

}

// */
