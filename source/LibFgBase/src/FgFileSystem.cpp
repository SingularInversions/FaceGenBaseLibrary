//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgDiagnostics.hpp"
#include "FgFileSystem.hpp"
#include "FgStdStream.hpp"
#include "FgStdString.hpp"
#include "FgStdVector.hpp"
#include "FgTime.hpp"
#include "FgParse.hpp"

using namespace std;
using namespace boost::filesystem;

namespace Fg {

void
fileCopy(Ustring const & src,Ustring const & dst,bool overwrite)
{
    if (pathExists(dst)) {
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
fileMove(Ustring const & src,Ustring const & dst,bool overwrite)
{
    // We use copy and delete since boost::filesystem::rename will fail if the target
    // is on a different volume:
    fileCopy(src,dst,overwrite);
    fgDeleteFile(src);
}

bool
pathExists(Ustring const & fname)
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

// TODO: re-write more efficient OS-specific code (utime on Linus, god knows what on Win):
void
fileTouch(Ustring const & fname)
{
    // Just opening the file for write on Windows doesn't actually change the modify date ... WTF
    String      tmp = fgSlurp(fname);
    fgDump(tmp,fname,false);
}

bool
fgSetCurrentDirUp()
{
    Path      cd = fgAsDirectory(fgGetCurrentDir());
    if (cd.dirs.empty())
        return false;
    fgSetCurrentDir(cd.dir(cd.dirs.size()-1));
    return true;
}

void
fgRemoveDirectoryRecursive(Ustring const & dirname)
{
#ifdef _WIN32
    // Manually recurse deletion of the directory tree to get around Windows filesystem bug:
    DirectoryContents     dc = directoryContents(dirname);
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
fgCreatePath(Ustring const & path)
{
    Path          p(fgAsDirectory(path));
    for (size_t ii=0; ii<p.dirs.size(); ++ii) {
        Ustring    dir = p.dir(ii+1);
        if (pathExists(dir)) {
            if (!isDirectory(dir))
                fgThrow("Not a directory (unable to create)",dir);
        }
        else
            fgCreateDirectory(dir);
    }
}

Ustring
fgExecutableDirectory()
{
    //return fgGetCurrentDir();         // For debugging installed versions
    Path      p(fgExecutablePath());
    return p.dir();
}

bool
fileReadable(Ustring const & filename)
{
    Ifstream ifs(filename,false);
    if (!ifs.is_open()) return false;
    return true;
}

Ustring
fgDirUserAppDataLocal(const vector<string> & subPath)
{
    Ustring    ret = fgDirUserAppDataLocalRoot();
    for (size_t ii=0; ii<subPath.size(); ++ii) {
        ret += subPath[ii] + fgDirSep();
        fgCreateDirectory(ret);
    }
    return ret;
}

string
fgSlurp(Ustring const & filename)
{
    Ifstream          ifs(filename);
    ostringstream       ss;
    ss << ifs.rdbuf();
    return ss.str();
}

bool
fgDump(string const & data,Ustring const & filename,bool onlyIfChanged)
{
    if (onlyIfChanged && pathExists(filename)) {
        string      fileData = fgSlurp(filename);
        if (data == fileData)
            return false;
    }
    Ofstream  ofs(filename);
    ofs << data;
    return true;
}

bool
equateFilesBinary(
    Ustring const & file1,
    Ustring const & file2)
{
    string contents1(fgSlurp(file1));
    string contents2(fgSlurp(file2));
    return contents1 == contents2;
}

bool
equateFilesText(Ustring const & fname0,Ustring const & fname1)
{
    return (fgSplitLinesUtf8(fgSlurp(fname0)) == fgSplitLinesUtf8(fgSlurp(fname1)));
}

static Ustring s_fgDataDir;

Ustring const &
dataDir(bool throwIfNotFound)
{
    // Typical locations relative to executable:
    //  ./data                  (applications & remote execution)
    //  ../../data              (pre-built sdk binaries in bin/os/)
    //  ../../../../../data     (dev & sdk binaries)
    if (!s_fgDataDir.empty())
        return s_fgDataDir;
    Ustring        pathStr = fgExecutableDirectory();
    Path          path(pathStr);
    string          data("data/"),
                    flag("_facegen_data_dir.flag");
    bool            keepSearching = true;
    while (keepSearching) {
        Ustring        dd = path.str()+data;
        if (isDirectory(dd)) {
            if (pathExists(dd+flag)) {
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
        fgThrow("dataDir unable to find FaceGen data directory",pathStr);
    return s_fgDataDir;
}

void
fgSetDataDir(Ustring const & dir)
{
    if (!pathExists(dir+"_facegen_data_dir.flag"))
        fgThrow("fgSetDataDir FaceGen data flag not found",dir);
    s_fgDataDir = dir;
}

bool
fileNewer(const Ustrings & sources,const Ustrings & sinks)
{
    FGASSERT(!sources.empty() && !sinks.empty());
    time_t      srcTime = getLastWriteTime(sources[0]);
    for (size_t ii=1; ii<sources.size(); ++ii) {
        time_t  st = getLastWriteTime(sources[ii]);
        if (st > srcTime)
            srcTime = st;
    }
    for (size_t ii=0; ii<sinks.size(); ++ii) {
        if (!pathExists(sinks[ii]))
            return true;
        time_t  st = getLastWriteTime(sinks[ii]);
        if (st < srcTime)
            return true;
    }
    return false;
}

DirectoryContents
globDirStartsWith(const Path & path)
{
    DirectoryContents ret;
    DirectoryContents dc = directoryContents(path.dir());
    for (size_t ii=0; ii<dc.filenames.size(); ++ii)
        if (dc.filenames[ii].beginsWith(path.base))
            ret.filenames.push_back(dc.filenames[ii]);
    for (size_t ii=0; ii<dc.dirnames.size(); ++ii)
        if (dc.dirnames[ii].beginsWith(path.base))
            ret.dirnames.push_back(dc.dirnames[ii]);
    return ret;
}

Ustrings
globFiles(const Path & path)
{
    Ustrings               ret;
    DirectoryContents     dc = directoryContents(path.dir());
    for (size_t ii=0; ii<dc.filenames.size(); ++ii) {
        Path              fn = dc.filenames[ii];
        if (fgGlobMatch(path.base,fn.base) && (fgGlobMatch(path.ext,fn.ext)))
            ret.push_back(dc.filenames[ii]);
    }
    return ret;
}

Ustrings
globFiles(Ustring const & basePath,Ustring const & relPath,Ustring const & filePattern)
{
    Ustrings       ret = globFiles(basePath+relPath+filePattern);
    for (Ustring & r : ret)
        r = relPath + r;
    return ret;
}

bool
fgCopyAllFiles(Ustring const & fromDir_,Ustring const & toDir_,bool overwrite)
{
    bool                    ret = false;
    Ustring                fromDir = fgAsDirectory(fromDir_),  // Ensure ends with delim
                            toDir = fgAsDirectory(toDir_);
    DirectoryContents     dc = directoryContents(fromDir);
    for (size_t ii=0; ii<dc.filenames.size(); ++ii) {
        Ustring            fn = dc.filenames[ii];
        if (pathExists(toDir+fn)) {
            ret = true;
            if (!overwrite)
                continue;
        }
        fileCopy(fromDir+fn,toDir+fn,overwrite);
    }
    return ret;
}

void
fgCopyToCurrentDir(const Path & file)
{
    if (pathExists(file.baseExt()))
        fgThrow("Attempt to copy to current directory which already contains",file.baseExt());
    fileCopy(file.str(),file.baseExt());
}

void
fgCopyRecursive(Ustring const & fromDir,Ustring const & toDir)
{
    if (!isDirectory(fromDir))
        fgThrow("Not a directory (unable to copy)",fromDir);
    fgCreateDirectory(toDir);
    Path                  fromP(fgAsDirectory(fromDir)),
                            toP(fgAsDirectory(toDir));
    Ustring                from = fromP.str(),     // Ensures delimiter at end when appropriate
                            to = toP.str();
    DirectoryContents     dc = directoryContents(fromDir);
    for (size_t ii=0; ii<dc.filenames.size(); ++ii) {
        Ustring &          fn = dc.filenames[ii];
        fileCopy(from+fn,to+fn);
    }
    for (size_t ii=0; ii<dc.dirnames.size(); ++ii) {
        Ustring const &    dn = dc.dirnames[ii];
        fgCopyRecursive(from+dn,to+dn);
    }
}

void
fgMirrorFile(const Path & src,const Path & dst)
{
    FGASSERT(!src.base.empty());
    FGASSERT(!dst.base.empty());
    for (size_t ii=0; ii<dst.dirs.size(); ++ii) {
        Ustring    dir = dst.dir(ii+1);
        if (pathExists(dir)) {
            if (!isDirectory(dir))
                fgThrow("Not a directory (unable to mirror)",dir);
        }
        else
            fgCreateDirectory(dir);
    }
    if (fileNewer(src.str(),dst.str()))
        fileCopy(src.str(),dst.str(),true);
}

}
