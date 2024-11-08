//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Makes use of std::filesystem which requires native unicode representation arguments for unicode support
// ie. UTF-16 wstring on Windows, UTF-8 string on *nix.

#ifndef FGFILESYSTEM_HPP
#define FGFILESYSTEM_HPP

#include "FgFile.hpp"

// **************************************************************************************
//                          SYSTEM DIRECTORIES
// **************************************************************************************

namespace Fg {

// Get root all-users application data directory (delimited):
// WARNING: See warning below.
String8             getDirSystemAppData();
// Get (and create if necessary) the all-users application data directory for the specified application.
// Note that for some users, access is not granted. I am unable to replicate this even if these
// directories are created by an admin user and the file within them is modified by a non-admin user....
String8             getDirSystemAppData(String8 const & groupName,String8 const & appName);
// Avoid using Windows 'roaming' directories as they only roam the WDS LAN, not personal cloud
// (see user documents directory below):
String8             getDirUserAppDataRoaming();
// Place to store local app data for this user:
String8             getDirUserAppDataLocal();
// As above but verifies/creates given subPath
String8             getDirUserAppDataLocal(Strings const & subDirs);
// As above but 'FaceGen' is pre-pended to 'subDirs':
String8             getDirUserAppDataLocalFaceGen(Strings const & subDirs);     // subDirs can be empty
// Can and does sometimes fail on Windows, possibly when using roaming identities.
// If it fails but 'throwOnFail' is false, it returns the empty string.
// WINDOWS: If user has OneDrive installed, new directories created here will be created within
//     the OneDrive/Documents/ directory instead of the local drive one.
String8             getUserDocsDir(bool throwOnFail=true);
// This has not been known to fail on Windows:
String8             getPublicDocsDir();

// **************************************************************************************
//                          FACEGEN DIRECTORIES
// **************************************************************************************
// Find FaceGen data directory from path of current executable, searching up one directory
// at a time for a directory named 'data' containing the file '_facegen_data_dir.flag'.
// If 'throwIfNotFound' is false, check the return value for the empty string (failure):
String8 const &     dataDir(bool throwIfNotFound=true);
// Manually set data directory. Useful for sandboxed platforms and debugging apps on native
// platforms. Does not check for the FaceGen data dir flag, since that it only there to help find
// the data dir, not validate it:
void                setDataDir(String8 const & dirEndingWithSlash);
String8 const &     getTestDir();                   // defaults to dataDir()/../test-output/
void                setTestDir(String8 const &);    // for customizing where tests store temp data

// **************************************************************************************
//                          OPERATIONS ON THE FILESYSTEM
// **************************************************************************************

// We can't use std::filesystem::is_directory inline here since it doesn't work on Windows 10 as of
// 18.04 update.
bool                isDirectory(String8 const & path);   // Doesn't throw - returns false for invalid path
// Both 'src' and 'dst' must be file paths; 'dst' should not be a directory.
// Note that both the creation time and last modification time are preserved on the copy !
void                copyFile(String8 const & srcFilePath,String8 const & dstFilePath,bool overwrite = false);
// copy multiple files from one directory to another. 'dstDir' must exist. Overwrite will cause an error:
void                copyFiles(String8 const & srcDir,String8 const & dstDir,String8s const & fnames);
// copy file to current directory:
inline void         copyFileToCurr(String8 const & filePath) {copyFile(filePath,pathToName(filePath)); }
// copy of a data file (ie. given by path relative to ~sdk/data) to current directory:
inline void         copyDataFileToCurr(String const & relPath) {copyFileToCurr(dataDir()+relPath); }

// Copy then delete original (safer than rename which doesn't work across volumes):
void                fileMove(String8 const & srcFilePath,String8 const & dstFilePath,bool overwrite = false);
// Will not throw, returns false for any kind of failure on 'fname':
bool                pathExists(String8 const & path);
inline bool         fileExists(String8 const & fname) {return (!isDirectory(fname) && pathExists(fname)); }
inline void         renameNode(String8 const & from,String8 const & to)
{
    return std::filesystem::rename(from.ns(),to.ns());
}
// Update last written time on existing file (will not create). Avoid large files as it current re-writes:
void                fileTouch(String8 const & fname);

struct      DirContents
{
    String8s    filenames;
    String8s    dirnames;   // Not including separators
};

// If dirName is a relative path, the current directory is used as the base point.
// The name strings are returned without the path.
DirContents         getDirContents(
    String8 const & dirName,
    bool            includeDot=false);  // Include files/dirs starting with '.' character, eg. ".", "..", ".hg"
// Directory names end with a delimiter:
String8             getCurrentDir();
bool                setCurrentDir(      // returns true if successful
    String8 const &    dir,        // Accepts full path or relative path
    bool throwOnFail=true);
bool                setCurrentDirUp();  // returns true if successful
// If 'anyPath' is relative, use the current directory to make it absolute:
String8             toAbsolutePath(String8 const & anyPath);
// Doesn't remove read-only files / dirs:
inline void         pathRemove(String8 const & fname) {std::filesystem::remove(fname.ns()); }
// Ignores read-only or hidden attribs on Windows (identical to pathRemove on nix):
void                deleteFile(String8 const &);
// Only works on empty dirs, return true if successful:
bool                removeDirectory(String8 const & dirName,bool throwOnFail=false);
// Delete all files in a directory (does not delete subdirectories):
void                deleteDirectoryFiles(String8 const &);
// Delete all files and subdirectories in a directory (does not delete the directory):
void                deleteDirectoryContentsRecursive(String8 const &);
// Delete a directory:
void                deleteDirectoryRecursive(String8 const &);      // Full recursive delete
// Accepts full or relative path, but only creates last delimited directory,
// Returns false if the directory already exists, true otherwise:
bool                createDirectory(String8 const &);
// Create all non-existing directories in given path. If the last name does not end with a delimiter it will be ignored:
void                createPath(String8 const &);
String8             getExecutablePath();    // Return the full path of the executable
// Return the full directory of the current application binary
String8             getExecutableDirectory();
bool                getCreationTimePrecise(String8 const & path,uint64 & time);
// Works for both files and directories.
// Value in seconds since filesystem resolution only gives about that anyway.
// On windows, returns time in seconds since 1601.01.01
// On unix, returns time in seconds since 1970.01.01
uint64              getCreationTime(String8 const & path);
// Works for both files and directories.
// Value in seconds since filesystem resolution only gives about that anyway.
// On windows, returns time in seconds since 1601.01.01
// On unix, returns time in seconds since 1970.01.01
// Don't use std::filesystem::last_write_time(); it doesn't work; returns create time on Win.
uint64              getLastWriteTime(String8 const & node);
// Return true if any of the sources have a 'last write time' newer than any of the sinks,
// of if any of the sinks don't exist (an error results if any of the sources don't exist):
bool                filesNewer(String8s const & sources,String8s const & sinks);
// Usually only need to include the one last output of a code chunk as 'dst':
inline bool         fileNewer(String8s const & sources,String8 const & dst) {return filesNewer(sources,{dst}); }

struct      PushDir         // set process current directory and keep previous ones in a stack
{
    String8s            orig;

    PushDir() {}
    explicit PushDir(String8 const & dir) {push(dir); }

    ~PushDir() {if (!orig.empty()) setCurrentDir(orig[0]); }

    void                push(String8 const & dir)
    {
        orig.push_back(getCurrentDir());
        setCurrentDir(dir);
    }
    void                pop()
    {
        if (orig.empty())
            return;
        setCurrentDir(orig.back());
        orig.resize(orig.size()-1);
    }
    void                change(String8 const & dir) {setCurrentDir(dir); }
};

// All output to 'fgout' will also be logged to a file
// This can't be in 'FgSerial.hpp' due to String8 dependency
struct      PushLogFile
{
    explicit PushLogFile(String8 const & fname,bool append=false)
    {
        fgout.logFile(fname.m_str,append,false);
    }
    ~PushLogFile() {fgout.logFileClose(); }
};

// Operations on multiple files and searching for files:
// 
// Very simple glob - only matches '*' at beginning or end of file base name (but not both unless whole name is '*') and/or extension.
// A single '*' does not glob with base and extension.
// Does not glob on input directory name.
// RETURNS: Matching filenames without directory:
String8s            globFiles(Path const & path);
// As above but the full path is given by 'basePath + keepPath' and the return paths include 'keepPath':
String8s            globFiles(String8 const & basePath,String8 const & relPath,String8 const & filePattern);
// Returns name of each matching file & dir:
DirContents         globNodeStartsWith(Path const & path);
// Returns the additional (delta) String8 of every file that starts with the same base name and has
// additional characters and the same extension:
String8s            globBaseVariants(const String8 & pathBaseExt);
// Returns true if there were any files in 'toDir' with the same name as a 'fromDir' file. 'toDir' must exist.
bool                fgCopyAllFiles(String8 const & fromDir,String8 const & toDir,bool overwrite=false);
// WARNING: Does not check if dirs are sym/hard links so be careful.
// The tip of 'toDir' will be created.
// Will throw on overwrite of any file or directory:
void                copyRecursive(String8 const & fromDir,String8 const & toDir);
// Copy 'src' to 'dst' if 'src' is newer or 'dst' (or its path) doesn't exist.
// Doesn't work reliably across network shares due to time differences.
void                mirrorFile(Path const & src,Path const & dst);
// returns all 'ext' in 'exts' (in order) such that the file 'dirBase.ext' exists:
Strings             findExts(String8 const & dirBase,Strings const & exts);

}

#endif
