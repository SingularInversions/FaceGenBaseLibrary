//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
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
#include "FgCommand.hpp"
#include "FgMetaFormat.hpp"

using namespace std;
using namespace boost::filesystem;

namespace Fg {

void                fileCopy(String8 const & src,String8 const & dst,bool overwrite)
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

void                fileMove(String8 const & src,String8 const & dst,bool overwrite)
{
    // We use copy and delete since boost::filesystem::rename will fail if the target
    // is on a different volume:
    fileCopy(src,dst,overwrite);
    deleteFile(src);
}

bool                pathExists(String8 const & fname)
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
void                fileTouch(String8 const & fname)
{
    // Just opening the file for write on Windows doesn't actually change the modify date ... WTF
    String      tmp = loadRaw(fname);
    saveRaw(tmp,fname,false);
}

bool                setCurrentDirUp()
{
    Path      cd = asDirectory(getCurrentDir());
    if (cd.dirs.empty())
        return false;
    setCurrentDir(cd.dir(cd.dirs.size()-1));
    return true;
}

String8             toAbsolutePath(String8 const & anyPath)
{
    Path            path {anyPath};
    if (!path.root)
        path = Path {getCurrentDir() + anyPath};
    return path.str();
}

void                deleteDirectoryFiles(String8 const & dirname)
{
    DirContents       dc = getDirContents(dirname);
    for (size_t ii=0; ii<dc.filenames.size(); ii++)
        deleteFile(dirname+"/"+dc.filenames[ii]);
}

void                deleteDirectoryRecursive(String8 const & dirname)
{
#ifdef _WIN32
    // Manually recurse deletion of the directory tree to get around Windows filesystem bug:
    DirContents     dc = getDirContents(dirname);
    for (size_t ii=0; ii<dc.dirnames.size(); ii++)
        deleteDirectoryRecursive(dirname+"/"+dc.dirnames[ii]);
    for (size_t ii=0; ii<dc.filenames.size(); ii++)
        deleteFile(dirname+"/"+dc.filenames[ii]);
    // Get around windows filesystem recursive folder delete bug by trying a second time
    // after a pause for the filesystem to catch up:
    if (removeDirectory(dirname))
        return;
    sleepSeconds(1);
    removeDirectory(dirname,true);
#else
    boost::filesystem::remove_all(dirname.m_str);
#endif
}

void                createPath(String8 const & path)
{
    Path          p(asDirectory(path));
    for (size_t ii=0; ii<p.dirs.size(); ++ii) {
        String8    dir = p.dir(ii+1);
        if (pathExists(dir)) {
            if (!isDirectory(dir))
                fgThrow("Not a directory (unable to create)",dir);
        }
        else
            createDirectory(dir);
    }
}

String8             getExecutableDirectory()
{
    //return getCurrentDir();         // For debugging installed versions
    Path      p(getExecutablePath());
    return p.dir();
}

bool                fileReadable(String8 const & filename)
{
    Ifstream ifs(filename,false);
    if (!ifs.is_open()) return false;
    return true;
}

String8             getDirUserAppDataLocal(Strings const & subPath)
{
    String8         ret = getDirUserAppDataLocal();
    for (String const & dir : subPath) {
        ret += dir;
        ret += nativeDirSep8;
        createDirectory(ret);
    }
    return ret;
}

String8             getDirUserAppDataLocalFaceGen(Strings const & subDirs)
{
    return getDirUserAppDataLocal(prepend(String{"FaceGen"},subDirs));
}

string              loadRaw(String8 const & filename)
{
    Ifstream          ifs(filename);
    ostringstream       ss;
    ss << ifs.rdbuf();
    return ss.str();
}

bool                saveRaw(String const & data,String8 const & filename,bool onlyIfChanged)
{
    if (onlyIfChanged && pathExists(filename)) {
        string      fileData = loadRaw(filename);
        if (data == fileData)
            return false;
    }
    Ofstream            ofs(filename);
    ofs << data;
    return true;
}

void                appendRaw(String8 const & filename,String const & data)
{
    Ofstream            ofs {filename,true};
    ofs << data;
}

bool                equateFilesBinary(
    String8 const & file1,
    String8 const & file2)
{
    string contents1(loadRaw(file1));
    string contents2(loadRaw(file2));
    return contents1 == contents2;
}

bool                equateFilesText(String8 const & fname0,String8 const & fname1)
{
    return (splitLinesUtf8(loadRaw(fname0)) == splitLinesUtf8(loadRaw(fname1)));
}

String8             getDataDir(bool throwIfNotFound)
{
    // Typical locations relative to executable:
    //  ./data                  (applications & remote execution)
    //  ../../data              (pre-built sdk binaries in bin/os/)
    //  ../../../../../data     (dev & sdk binaries)
    String8         pathStr = getExecutablePath();
    Path            path {pathStr};
    string          data = "data/",
                    flag = "_facegen_data_dir.flag";
    // check exe dir and every dir above in that order (incl. top level in case of thumb drive etc.):
    for(;;) {
        String8        dd = path.dir() + data;
        if (pathExists(dd+flag))
            return dd;
        if (path.dirs.empty())
            break;
        else
            path.dirs.pop_back();
    }
    if (throwIfNotFound)
        fgThrow("Unable to find FaceGen data directory.\n"
            "If you moved this executable from its original path (see below),\n"
            "You must also copy the 'data' directory (including contents)\n"
            "to the same location or one of its parent directories",
            pathStr);
    return String8{};
}

static String8      s_fgDataDir;

String8 const &     dataDir(bool throwIfNotFound)
{
    if (s_fgDataDir.empty())
        s_fgDataDir = getDataDir(throwIfNotFound);
    return s_fgDataDir;
}

void                setDataDir(String8 const & dir)
{
    if (!pathExists(dir+"_facegen_data_dir.flag"))
        fgThrow("setDataDir FaceGen data flag not found",dir);
    s_fgDataDir = dir;
}

bool                fileNewer(String8s const & sources,String8s const & sinks)
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

String8s            globFiles(Path const & path)
{
    String8s               ret;
    DirContents     dc = getDirContents(path.dir());
    for (size_t ii=0; ii<dc.filenames.size(); ++ii) {
        Path              fn = dc.filenames[ii];
        if (isGlobMatch(path.base,fn.base) && (isGlobMatch(path.ext,fn.ext)))
            ret.push_back(dc.filenames[ii]);
    }
    return ret;
}

String8s            globFiles(String8 const & basePath,String8 const & relPath,String8 const & filePattern)
{
    String8s       ret = globFiles(basePath+relPath+filePattern);
    for (String8 & r : ret)
        r = relPath + r;
    return ret;
}

DirContents         globNodeStartsWith(Path const & path)
{
    DirContents ret;
    DirContents dc = getDirContents(path.dir());
    for (String8 const & fname : dc.filenames)
        if (fname.beginsWith(path.baseExt()))
            ret.filenames.push_back(fname);
    for (String8 const & dname : dc.dirnames)
        if (dname.beginsWith(path.baseExt()))
            ret.dirnames.push_back(dname);
    return ret;
}

String8s            globBaseVariants(const String8 & pathBaseExt)
{
    Path            path {pathBaseExt};
    String8s        fnames = globNodeStartsWith(path.dirBase()).filenames;
    String8s        ret;
    for (String8 const & fname : fnames) {
        Path            fp {fname};
        if ((fp.base != path.base) && (fp.ext == path.ext))
            ret.push_back(cRest(fp.base,path.base.size()));
    }
    return ret;
}

bool                fgCopyAllFiles(String8 const & fromDir_,String8 const & toDir_,bool overwrite)
{
    bool                    ret = false;
    String8                fromDir = asDirectory(fromDir_),  // Ensure ends with delim
                            toDir = asDirectory(toDir_);
    DirContents     dc = getDirContents(fromDir);
    for (size_t ii=0; ii<dc.filenames.size(); ++ii) {
        String8            fn = dc.filenames[ii];
        if (pathExists(toDir+fn)) {
            ret = true;
            if (!overwrite)
                continue;
        }
        fileCopy(fromDir+fn,toDir+fn,overwrite);
    }
    return ret;
}

void                fgCopyToCurrentDir(Path const & file)
{
    if (pathExists(file.baseExt()))
        fgThrow("Attempt to copy to current directory which already contains",file.baseExt());
    fileCopy(file.str(),file.baseExt());
}

void                copyRecursive(String8 const & fromDir,String8 const & toDir)
{
    if (!isDirectory(fromDir))
        fgThrow("Not a directory (unable to copy)",fromDir);
    createDirectory(toDir);
    Path                  fromP(asDirectory(fromDir)),
                            toP(asDirectory(toDir));
    String8                from = fromP.str(),     // Ensures delimiter at end when appropriate
                            to = toP.str();
    DirContents     dc = getDirContents(fromDir);
    for (size_t ii=0; ii<dc.filenames.size(); ++ii) {
        String8 &          fn = dc.filenames[ii];
        fileCopy(from+fn,to+fn);
    }
    for (size_t ii=0; ii<dc.dirnames.size(); ++ii) {
        String8 const &    dn = dc.dirnames[ii];
        copyRecursive(from+dn,to+dn);
    }
}

void                mirrorFile(Path const & src,Path const & dst)
{
    FGASSERT(!src.base.empty());
    FGASSERT(!dst.base.empty());
    for (size_t ii=0; ii<dst.dirs.size(); ++ii) {
        String8    dir = dst.dir(ii+1);
        if (pathExists(dir)) {
            if (!isDirectory(dir))
                fgThrow("Not a directory (unable to mirror)",dir);
        }
        else
            createDirectory(dir);
    }
    if (fileNewer(src.str(),dst.str()))
        fileCopy(src.str(),dst.str(),true);
}

Strings             findExts(String8 const & dirBase,Strings const & exts)
{
    Strings             ret;
    for (String const & ext : exts)
        if (fileExists(dirBase+"."+ext))
            ret.push_back(ext);
    return ret;
}

namespace {

void                testCurrentDirectory(CLArgs const & args)
{
    FGTESTDIR
    try
    {
        char32_t        ch = 0x00004EE5;            // A Chinese character
        String8         chinese(ch);
        String8         oldDir = getCurrentDir();
        String8         dirName = chinese + nativeDirSep8;
        createDirectory(dirName);
        setCurrentDir(dirName);
        String8         newDir = getCurrentDir();
        String8         expected = oldDir + dirName;
        setCurrentDir(oldDir);
        String8         restored = getCurrentDir();
        FGASSERT(removeDirectory(dirName));
        fgout << fgnl << "Original directory:    " << oldDir.as_utf8_string();
        fgout << fgnl << "New current directory: " << newDir.as_utf8_string();
        fgout << fgnl << "Expected directory:    " << expected.as_utf8_string();
        fgout << fgnl << "Restored directory:    " << restored.as_utf8_string();
        FGASSERT(expected == newDir);
    }
    catch (FgExceptionNotImplemented const & e) 
    {
        fgout << e.tr_message();
    }
}

void                testOfstreamUnicode(CLArgs const & args)
{
    FGTESTDIR
    char32_t        cent = 0x000000A2;              // The cent sign
    String8        test = String8(cent);
    Ofstream      ofs(test);
    FGASSERT(ofs);
    ofs.close();
    pathRemove(test);
}

void                testReadableFile(CLArgs const & args)
{
    FGTESTDIR
    std::ofstream ofs("testReadableFile.txt");
    FGASSERT(ofs);
    ofs << "Hi";
    ofs.close();
    FGASSERT(fileReadable("testReadableFile.txt"));
    FGASSERT(!fileReadable("This file does not exist"));
}

void                testDeleteDirectory(CLArgs const & args)
{
    FGTESTDIR
    char32_t        ch = 0x000000A2;              // The cent sign
    String8        cent = String8(ch)+"/";
    String8        name = "testDeleteDirectory/";
    createDirectory(name);
    FGASSERT(pathExists(name));
    createDirectory(name+cent);
    saveBsaXml(name+cent+"a",42);
    saveBsaXml(name+"b",21);
    deleteDirectoryRecursive(name);
    FGASSERT(!pathExists(name));
}

void                testRecursiveCopy(CLArgs const & args)
{
    FGTESTDIR
    string          path = "silly-v3.4.7/subdir/";
    createPath("tst1/"+path);
    Ofstream      ofs("tst1/"+path+"file");
    ofs << "hello";
    ofs.close();
    copyRecursive("tst1","tst2");
    Ifstream      ifs("tst2/"+path+"file");
    string          hello;
    ifs >> hello;
    FGASSERT(hello == "hello");
}

void                testExists(CLArgs const &)
{
    FGASSERT(!pathExists("//doesNotExists"));
}

}

void                testFilesystem(CLArgs const & args)
{
    Cmds            cmds {
        {testCurrentDirectory,"curDir"},
        {testOfstreamUnicode,"ofsUni"},
        {testReadableFile,"readable"},
        {testDeleteDirectory,"delDir"},
        {testRecursiveCopy,"recurseCopy"},
        {testExists,"exists"},
    };
    doMenu(args,cmds,true,true,true);
}

}
