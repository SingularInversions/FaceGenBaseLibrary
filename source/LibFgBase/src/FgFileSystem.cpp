//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     May 5, 2005
//

#include "stdafx.h"

#include "FgAlgs.hpp"
#include "FgDiagnostics.hpp"
#include "FgFileSystem.hpp"
#include "FgStdStream.hpp"
#include "FgStdString.hpp"
#include "FgStdVector.hpp"
#include "FgTime.hpp"

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

bool
fgExists(const FgString & fname)
{
    // boost::filesytem::exists can throw when it is unable to obtain the status of a path.
    // We don't want that - consider it just not there:
    bool    ret;
    try {
        ret = boost::filesystem::exists(fname.ns());
    }
    catch (...) {
        ret = false;
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
fgRemoveDirectoryRecursive(const FgString & dirname)
{
#ifdef _WIN32
    // Manually recurse deletion of the directory tree to get around Windows filesystem bug:
    FgDirectoryContents     dc = fgDirectoryContents(dirname);
    for (size_t ii=0; ii<dc.dirnames.size(); ii++)
        fgRemoveDirectoryRecursive(dirname+"/"+dc.dirnames[ii]);
    for (size_t ii=0; ii<dc.filenames.size(); ii++)
        fgDeleteFile(dirname+"/"+dc.filenames[ii]);
    // Get around windows filesystem recursive folder delete bug by trying a second time
    // after a pause for the filesystem to catch up:
    if (fgRemoveDirectory(dirname))
        return;
    fgSleep(1);
    fgRemoveDirectory(dirname,true);
#else
    boost::filesystem::remove_all(dirname.m_str);
#endif
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
fgDirUserAppDataLocal(const vector<string> & subPath)
{
    FgString    ret = fgDirUserAppDataLocalRoot();
    for (size_t ii=0; ii<subPath.size(); ++ii) {
        ret += subPath[ii] + fgDirSep();
        fgCreateDirectory(ret);
    }
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

bool
fgDump(const string & data,const FgString & filename,bool onlyIfChanged)
{
    if (onlyIfChanged && fgExists(filename)) {
        string      fileData = fgSlurp(filename);
        if (data == fileData)
            return false;
    }
    FgOfstream  ofs(filename);
    ofs << data;
    return true;
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

static FgString s_fgDataDir;

const FgString &
fgDataDir(bool throwIfNotFound)
{
    // Typical locations relative to executable:
    //  ./data                  (applications & remote execution)
    //  ../../data              (pre-built sdk binaries in bin/os/)
    //  ../../../../../data     (dev & sdk binaries)
    if (!s_fgDataDir.empty())
        return s_fgDataDir;
    FgString        pathStr = fgExecutableDirectory();
    FgPath          path(pathStr);
    string          data("data/"),
                    flag("_facegen_data_dir.flag");
    bool            keepSearching = true;
    while (keepSearching) {
        FgString        dd = path.str()+data;
        if (fgIsDirectory(dd)) {
            if (fgExists(dd+flag)) {
                s_fgDataDir = dd;
                return s_fgDataDir;
            }
        }
        if (path.dirs.empty())
            keepSearching = false;
        else
            path.dirs.pop_back();
    }
    if (throwIfNotFound)
        fgThrow("fgDataDir unable to find FaceGen data directory",pathStr);
    return s_fgDataDir;
}

void
fgSetDataDir(const FgString & dir)
{
    if (!fgExists(dir+"_facegen_data_dir.flag"))
        fgThrow("fgSetDataDir FaceGen data flag not found",dir);
    s_fgDataDir = dir;
}

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
    FgStrings               ret;
    FgDirectoryContents     dc = fgDirectoryContents(path.dir());
    for (size_t ii=0; ii<dc.filenames.size(); ++ii) {
        FgPath              fn = dc.filenames[ii];
        if (fgGlobMatch(path.base,fn.base) && (fgGlobMatch(path.ext,fn.ext)))
            ret.push_back(dc.filenames[ii]);
    }
    return ret;
}

FgStrings
fgGlobFiles(const FgString & basePath,const FgString & relPath,const FgString & filePattern)
{
    FgStrings       ret = fgGlobFiles(basePath+relPath+filePattern);
    for (FgString & r : ret)
        r = relPath + r;
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
