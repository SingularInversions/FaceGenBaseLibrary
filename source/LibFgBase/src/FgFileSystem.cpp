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
using namespace boost::filesystem;

void
fgCopyFile(const FgString & src,const FgString & dst,bool overwrite)
{
    if (fgExists(dst)) {
        if (overwrite) {
            // 'copy_option::overwrite_if_exists' leaves file of size max(size(src),size(dst)) but
            // explicit removal is not always reflected quickly in filesystem leading to an error if we
            // don't use it, and possibly an error even if we do (attempt to overwrite non-existing file):
            copy_file(src.ns(),dst.ns(),copy_option::overwrite_if_exists);
        }
        else
            fgThrow("Attempt to copy over existing file",dst);
    }
    else
        // Global name conflicts prevent aliasing of some names to unversioned
        // 'filesystem' namespace:
        copy_file(src.ns(),dst.ns());
}

void
fgMoveFile(const FgString & src,const FgString & dst,bool overwrite)
{
    // We use copy and delete since boost::filesystem::rename will fail if the target
    // is on a different volume:
    fgCopyFile(src,dst,overwrite);
    fgDeleteFile(src);
}

FgDirectoryContents
fgDirectoryContents(const FgString & dirName)
{
    FgString        dn = dirName;
    if (dn.empty())     // Interpret this as current directory, which boost filesystem does not
        dn = FgString(".");
    if (!is_directory(dn.ns()))
        fgThrow("Not a directory",dirName);
    FgDirectoryContents     ret;
    directory_iterator      it_end;
    for (directory_iterator it(dn.ns()); it != it_end; ++it) {
        if (is_directory(it->status()))
            ret.dirnames.push_back(it->path().filename().string());
        else if (is_regular_file(it->status()))
            ret.filenames.push_back(it->path().filename().string());
    }
    return ret;
}

bool
fgSetCurrentDirUp()
{
    FgPath      cd = fgAsDirectory(fgGetCurrentDir());
    if (cd.dirs.empty())
        return false;
    fgSetCurrentDir(cd.dir(cd.dirs.size()-1));
    return true;
}

void
fgCreatePath(const FgString & path)
{
    FgPath          p(fgAsDirectory(path));
    for (size_t ii=0; ii<p.dirs.size(); ++ii) {
        FgString    dir = p.dir(ii+1);
        if (fgExists(dir)) {
            if (!fgIsDirectory(dir))
                fgThrow("Not a directory (unable to create)",dir);
        }
        else
            fgCreateDirectory(dir);
    }
}

FgString
fgExecutableDirectory()
{
    //return fgGetCurrentDir();         // For debugging installed versions
    FgPath      p(fgExecutablePath());
    return p.dir();
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
    FgIfstream          ifs(filename);
    ostringstream       ss;
    ss << ifs.rdbuf();
    return ss.str();
}

void
fgDump(
    const string &      data,
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

static bool s_dataDirFromPath = false;

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
    FgPath      path;
    if (s_dataDirFromPath)
        path = fgGetCurrentDir();
    else
        path = fgExecutableDirectory();
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

void
fgDataDirFromCurrent()
{s_dataDirFromPath = true; }

bool
fgNewer(const FgStrings & sources,const FgStrings & sinks)
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
fgGlobStartsWith(const FgPath & path)
{
    FgDirectoryContents ret;
    FgDirectoryContents dc = fgDirectoryContents(path.dir());
    for (size_t ii=0; ii<dc.filenames.size(); ++ii)
        if (dc.filenames[ii].beginsWith(path.base))
            ret.filenames.push_back(dc.filenames[ii]);
    for (size_t ii=0; ii<dc.dirnames.size(); ++ii)
        if (dc.dirnames[ii].beginsWith(path.base))
            ret.dirnames.push_back(dc.dirnames[ii]);
    return ret;
}

FgDirectoryContents
fgGlobHasExtension(FgPath path)
{
    FgDirectoryContents ret;
    FgDirectoryContents dc = fgDirectoryContents(path.dir());
    for (size_t ii=0; ii<dc.filenames.size(); ++ii)
        if (FgPath(dc.filenames[ii]).ext == path.ext)
            ret.filenames.push_back(dc.filenames[ii]);
    for (size_t ii=0; ii<dc.dirnames.size(); ++ii)
        if (FgPath(dc.dirnames[ii]).ext == path.ext)
            ret.dirnames.push_back(dc.dirnames[ii]);
    return ret;
}

FgStrings
fgGlobFiles(const FgPath & path)
{
    FgStrings        ret;
    FgDirectoryContents     dc = fgDirectoryContents(path.dir());
    for (size_t ii=0; ii<dc.filenames.size(); ++ii) {
        FgPath              fn = dc.filenames[ii];
        if (fgGlobMatch(path.base,fn.base) && (fgGlobMatch(path.ext,fn.ext)))
            ret.push_back(dc.filenames[ii]);
    }
    return ret;
}

bool
fgCopyAllFiles(const FgString & fromDir_,const FgString & toDir_,bool overwrite)
{
    bool                    ret = false;
    FgString                fromDir = fgAsDirectory(fromDir_),  // Ensure ends with delim
                            toDir = fgAsDirectory(toDir_);
    FgDirectoryContents     dc = fgDirectoryContents(fromDir);
    for (size_t ii=0; ii<dc.filenames.size(); ++ii) {
        FgString            fn = dc.filenames[ii];
        if (fgExists(toDir+fn)) {
            ret = true;
            if (!overwrite)
                continue;
        }
        fgCopyFile(fromDir+fn,toDir+fn,overwrite);
    }
    return ret;
}

void
fgCopyToCurrentDir(const FgPath & file)
{
    if (fgExists(file.baseExt()))
        fgThrow("Attempt to copy to current directory which already contains",file.baseExt());
    fgCopyFile(file.str(),file.baseExt());
}

void
fgCopyRecursive(const FgString & fromDir,const FgString & toDir)
{
    if (!fgIsDirectory(fromDir))
        fgThrow("Not a directory (unable to copy)",fromDir);
    fgCreateDirectory(toDir);
    FgPath                  fromP(fgAsDirectory(fromDir)),
                            toP(fgAsDirectory(toDir));
    FgString                from = fromP.str(),     // Ensures delimiter at end when appropriate
                            to = toP.str();
    FgDirectoryContents     dc = fgDirectoryContents(fromDir);
    for (size_t ii=0; ii<dc.filenames.size(); ++ii) {
        FgString &          fn = dc.filenames[ii];
        fgCopyFile(from+fn,to+fn);
    }
    for (size_t ii=0; ii<dc.dirnames.size(); ++ii) {
        const FgString &    dn = dc.dirnames[ii];
        fgCopyRecursive(from+dn,to+dn);
    }
}

void
fgMirrorFile(const FgPath & src,const FgPath & dst)
{
    FGASSERT(!src.base.empty());
    FGASSERT(!dst.base.empty());
    for (size_t ii=0; ii<dst.dirs.size(); ++ii) {
        FgString    dir = dst.dir(ii+1);
        if (fgExists(dir)) {
            if (!fgIsDirectory(dir))
                fgThrow("Not a directory (unable to mirror)",dir);
        }
        else
            fgCreateDirectory(dir);
    }
    if (fgNewer(src.str(),dst.str()))
        fgCopyFile(src.str(),dst.str(),true);
}
