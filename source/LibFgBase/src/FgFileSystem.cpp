//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     May 5, 2005
//

#include "stdafx.h"

#include "FgDiagnostics.hpp"
#include "FgFileSystem.hpp"
#include "FgStdStream.hpp"
#include "FgStdString.hpp"
#include "FgStdVector.hpp"

using namespace std;

void
fgCopyFile(const FgString & src,const FgString & dst,bool overwrite)
{
    // copy_option::overwrite_if_exists  should not be used as the resulting size of the
    // file is max(size(src),size(dst)):
    if (fgExists(dst)) {
        if (overwrite)
            fgRemove(dst);
        else
            fgThrow("Attempt to copy over existing file",dst);
    }
    // Global name conflicts prevent aliasing of some names to unversioned
    // 'filesystem' namespace:
    boost::filesystem::copy_file(src.ns(),dst.ns());
}

void
fgMoveFile(const FgString & src,const FgString & dst,bool overwrite)
{
    // We use copy and delete since boost::filesystem::rename will fail if the target
    // is on a different volume:
    fgCopyFile(src,dst,overwrite);
    fgDeleteFile(src);
}

void
fgCreatePath(const FgPath & p)
{
    for (size_t ii=0; ii<p.dirs.size(); ++ii) {
        FgString    dir = p.str(ii+1);
        if (fgExists(dir))
            FGASSERT(fgIsDirectory(dir));
        else
            fgCreateDirectory(dir);
    }
}

FgString
fgExecutableDirectory()
{
    FgPath      p(fgExecutablePath());
    return p.dirOnly();
}

bool
fgFileReadable(const FgString & filename)
{
    FgIfstream ifs(filename,false);
    if (!ifs.is_open()) return false;
    return true;
}

FgString
fgDirSystemAppData(
    FgString const & groupName,
    FgString const & appName)
{
    FgString    appDir = fgDirSystemAppDataRoot() + groupName;
    fgCreateDirectory(appDir);
    appDir = appDir + fgDirSep() + appName;
    fgCreateDirectory(appDir);
    return appDir + fgDirSep();
}

FgString
fgDirUserAppDataLocal(const vector<string> & subPath)
{
    FgString    ret = fgDirUserAppDataLocalRoot();
    for (size_t ii=0; ii<subPath.size(); ++ii) {
        ret += subPath[ii] + fgDirSep();
        fgCreateDirectory(ret); }
    return ret;
}

string
fgSlurp(const FgString & filename)
{
    FgIfstream ifs(filename);
    std::ostringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

void
fgDump(
    const std::string & data,
    const FgString &    filename)
{
    FgOfstream  ofs(filename);
    ofs << data;
}

bool
fgBinaryFileCompare(
    const FgString & file1,
    const FgString & file2)
{
    string contents1(fgSlurp(file1));
    string contents2(fgSlurp(file2));
    return contents1 == contents2;
}

const FgString &
fgDataDir()
{
    // First find the data directory relative to current binary. It can be in one of:
    //  ./data                  (applications & remote execution)
    //  ../../data              (pre-built sdk binaries in bin/os/)
    // ...
    //  ../../../../../data     (dev & sdk binaries)
    static FgString     ret;
    if (!ret.empty())
        return ret;
    FgPath      path(fgExecutableDirectory());
    string      data("data"+ fgDirSep());
    for (size_t ii=0; ii<6; ++ii) {
        FgString    dd = path.str()+data;
        if (fgIsDirectory(dd)) {
            ret = dd;
            return ret;
        }
        path.dirs.pop_back();
    }
    fgThrow("Unable to find FaceGen data directory from executable location",fgExecutableDirectory());
    return ret;
}

bool
fgNewer(const vector<FgString> & sources,const vector<FgString> & sinks)
{
    FGASSERT(!sources.empty() && !sinks.empty());
    time_t      srcTime = fgLastWriteTime(sources[0]);
    for (size_t ii=1; ii<sources.size(); ++ii) {
        time_t  st = fgLastWriteTime(sources[ii]);
        if (st > srcTime)
            srcTime = st;
    }
    for (size_t ii=0; ii<sinks.size(); ++ii) {
        if (!fgExists(sinks[ii]))
            return true;
        time_t  st = fgLastWriteTime(sinks[ii]);
        if (st < srcTime)
            return true;
    }
    return false;
}

FgDirectoryContents
fgGlobStartsWith(const FgString & sw)
{
    FgDirectoryContents ret;
    FgPath              path(sw);
    FgDirectoryContents dc = fgDirectoryContents(path.dirOnly());
    for (size_t ii=0; ii<dc.filenames.size(); ++ii)
        if (dc.filenames[ii].beginsWith(path.base))
            ret.filenames.push_back(path.dirOnly()+dc.filenames[ii]);
    for (size_t ii=0; ii<dc.dirnames.size(); ++ii)
        if (dc.dirnames[ii].beginsWith(path.base))
            ret.dirnames.push_back(path.dirOnly()+dc.dirnames[ii]);
    return ret;
}

FgDirectoryContents
fgGlobHasExtension(FgPath path)
{
    FgDirectoryContents ret;
    FgDirectoryContents dc = fgDirectoryContents(path.dirOnly());
    for (size_t ii=0; ii<dc.filenames.size(); ++ii)
        if (FgPath(dc.filenames[ii]).ext == path.ext)
            ret.filenames.push_back(dc.filenames[ii]);
    for (size_t ii=0; ii<dc.dirnames.size(); ++ii)
        if (FgPath(dc.dirnames[ii]).ext == path.ext)
            ret.dirnames.push_back(dc.dirnames[ii]);
    return ret;
}

vector<FgString>
fgGlobFiles(const FgPath & path)
{
    vector<FgString>        ret;
    FgDirectoryContents     dc = fgDirectoryContents(path.dirOnly());
    for (size_t ii=0; ii<dc.filenames.size(); ++ii) {
        FgPath              fn = dc.filenames[ii];
        if (fgGlobMatch(path.base,fn.base) && (fgGlobMatch(path.ext,fn.ext)))
            ret.push_back(dc.filenames[ii]);
    }
    return ret;
}

void
fgCopyToCurrentDir(const FgPath & file)
{
    if (fgExists(file.nameOnly()))
        fgThrow("Attempt to copy to current directory which already contains",file.nameOnly());
    fgCopyFile(file.str(),file.nameOnly());
}
