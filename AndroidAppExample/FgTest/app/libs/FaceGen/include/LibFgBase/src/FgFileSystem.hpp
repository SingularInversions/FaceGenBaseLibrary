//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     May 5, 2005
//
// Makes use of boost::filesystem which requires native unicode representation arguments for unicode support
// ie. UTF-16 wstring on Windows, UTF-8 string on *nix.

#ifndef FGFILESYSTEM_HPP
#define FGFILESYSTEM_HPP

#include "FgStdLibs.hpp"
#include "FgBoostLibs.hpp"
#include "FgStdString.hpp"
#include "FgString.hpp"
#include "FgException.hpp"
#include "FgPath.hpp"

// **************************************************************************************
//                          SYSTEM DIRECTORIES
// **************************************************************************************

// Get root all-users application data directory (delimited):
// WARNING: See warning below.
FgString
fgDirSystemAppDataRoot();

// Get (and create if necessary) the all-users application data directory for the specified application.
// Note that for some users, access is not granted. I am unable to replicate this even if these
// directories are created by an admin user and the file within them is modified by a non-admin user....
FgString
fgDirSystemAppData(FgString const & groupName,FgString const & appName);

FgString
fgDirUserAppDataRoamingRoot();

// Place to store local app data for this user:
FgString
fgDirUserAppDataLocalRoot();

// As above but verifies/creates given subPath
FgString
fgDirUserAppDataLocal(const vector<string> & subDirs);

// As above but verifies/creates subdirectory for "FaceGen" then for specified:
inline
FgString
fgDirUserAppDataLocalFaceGen(const string & subd0,const string & subd1)
{return fgDirUserAppDataLocal(fgSvec<string>("FaceGen",subd0,subd1)); }

// Can and does sometimes fail on Windows, possibly when using roaming identities.
// If it fails but 'throwOnFail' is false, it returns the empty string:
FgString
fgUserDocumentsDirectory(bool throwOnFail=true);

// This has not been known to fail on Windows:
FgString
fgPublicDocumentsDirectory();

// Find FaceGen data directory from path of current executable, searching up one directory
// at a time for a directory named 'data' containing the file '_facegen_data_dir.flag'.
// If 'throwIfNotFound' is false, check the return value for the empty string (failure):
const FgString & fgDataDir(bool throwIfNotFound=true);

// Manually set data directory. Useful for sandboxed platforms and debugging apps on native
// platforms:
void
fgSetDataDir(const FgString & dirEndingWithSlash);

// **************************************************************************************
//                          OPERATIONS ON THE FILESYSTEM
// **************************************************************************************

// Both 'src' and 'dst' must be file paths; 'dst' should not be a directory:
void
fgCopyFile(const FgString & srcFilePath,const FgString & dstFilePath,bool overwrite = false);

// Copy then delete original (safer than rename which doesn't work across volumes):
void
fgMoveFile(const FgString & srcFilePath,const FgString & dstFilePath,bool overwrite = false);

// Will not throw, returns false for any kind of failure on 'fname':
bool
fgExists(const FgString & fname);

inline bool
fgRemove(const FgString & fname)
{return boost::filesystem::remove(fname.ns()); }

inline void
fgRename(const FgString & from,const FgString & to)
{return boost::filesystem::rename(from.ns(),to.ns()); }

// We can't use boost::filesystem::is_directory inline here since it doesn't work on Windows 10 as of
// 18.04 update.
bool
fgIsDirectory(const FgString & path);   // Doesn't throw - returns false for invalid path

struct      FgDirectoryContents
{
    FgStrings    filenames;
    FgStrings    dirnames;   // Not including separators
};

// If dirName is a relative path, the current directory is used as the base point.
// The names strings are returned without the path.
// Directory names "." and ".." are NOT returned.
FgDirectoryContents
fgDirectoryContents(const FgString & dirName);

// Directory names end with a delimiter:
FgString
fgGetCurrentDir();

bool                                // true if successful
fgSetCurrentDir(
    const FgString &    dir,        // Accepts full path or relative path
    bool throwOnFail=true);

bool                                // true if successful
fgSetCurrentDirUp();

// Doesn't remove read-only files:
inline void
fgRemoveFile(const FgString & fname)
{boost::filesystem::remove(fname.ns()); }

// Ignores read-only or hidden attribs on Windows (identical to fgRemoveFile on nix):
void
fgDeleteFile(const FgString &);

// Only works on empty dirs, return true if successful:
bool
fgRemoveDirectory(
    const FgString &    dirName,
    bool                throwOnFail=false);

// Throws on failure:
void
fgRemoveDirectoryRecursive(const FgString &);      // Full recursive delete

// Accepts full or relative path, but only creates last delimited directory:
bool            // Returns false if the directory already exists, true otherwise
fgCreateDirectory(const FgString &);

// Create all non-existing directories in given path.
// An undelimited name will be created as a directory:
void
fgCreatePath(const FgString &);

FgString                        // Return the full path of the executable
fgExecutablePath();

FgString                        // Return the full directory of the current application binary
fgExecutableDirectory();

// Returns true if the supplied filename is a file which can be read
// by the calling process:
bool
fgFileReadable(const FgString & filename);

std::string
fgSlurp(FgString const & filename);

// Setting 'onlyIfChanged' to false will result in the file always being written,
// regardless of whether the new data may be identical.
// Leaving 'true' is useful to avoid triggering unwanted change detections.
bool    // Returns true if the file was written
fgDump(const std::string & data,const FgString & filename,bool onlyIfChanged=true);

// Returns true if identical:
bool
fgBinaryFileCompare(
    const FgString & file1,
    const FgString & file2);

// Returns false if the given file or directory cannot be read.
// The returned time is NOT compatible with std raw time and will in fact crash fgDateTimeString():
bool
fgCreationTime(const FgString & path,uint64 & time);

// Works for both files and directories:
inline
std::time_t
fgLastWriteTime(const FgString & path)
{return boost::filesystem::last_write_time(path.ns()); }

// Return true if any of the sources have a 'last write time' newer than any of the sinks,
// of if any of the sinks don't exist (an error results if any of the sources don't exist):
bool
fgNewer(const FgStrings & sources,const FgStrings & sinks);

inline
bool
fgNewer(const FgString & src,const FgString & dst)
{return fgNewer(fgSvec(src),fgSvec(dst)); }

// Usually only need to include the one last output of a code chunk as 'dst':
inline
bool
fgNewer(const FgStrings & sources,const FgString & dst)
{return fgNewer(sources,fgSvec(dst)); }

struct  FgPushDir
{
    FgStrings    orig;

    // Often need to create in different scope from 'push':
    FgPushDir() {}

    explicit
    FgPushDir(const FgString & dir)
    {push(dir); }

    ~FgPushDir()
    {
        if (!orig.empty())
            fgSetCurrentDir(orig[0]);
    }

    void push(const FgString & dir)
    {
        orig.push_back(fgGetCurrentDir());
        fgSetCurrentDir(dir);
    }

    void pop()
    {
        fgSetCurrentDir(orig.back());
        orig.resize(orig.size()-1);
    }

    void change(const FgString & dir)
    {fgSetCurrentDir(dir); }
};

// Returns name of each matching file & dir:
FgDirectoryContents
fgGlobStartsWith(const FgPath & path);

// If 'path' has a base name it's ignored.
// Extension matching is case sensitive and includes null string.
// Returns filenames only, not paths.
FgDirectoryContents
fgGlobHasExtension(FgPath path);

// Very simple glob - only matches '*' at beginning or end of file base name (but not both
// unless whole name is '*') and/or extension.
// A single '*' does not glob with base and extension.
// Does not glob on input directory name.
// RETURNS: Matching filenames without directory:
FgStrings
fgGlobFiles(const FgPath & path);

// As above but the full path is given by 'basePath + keepPath' and the return paths include 'keepPath':
FgStrings
fgGlobFiles(const FgString & basePath,const FgString & relPath,const FgString & filePattern);

// 'toDir' must exist.
// Returns true if there were any files in 'toDir' with the same name as a 'fromDir' file:
bool
fgCopyAllFiles(const FgString & fromDir,const FgString & toDir,bool overwrite=false);

// Throws an exception if the filename already exists in the current directory:
void
fgCopyToCurrentDir(const FgPath & file);

// WARNING: Does not check if dirs are sym/hard links so be careful.
// The tip of 'toDir' will be created.
// Will throw on overwrite of any file or directory:
void
fgCopyRecursive(const FgString & fromDir,const FgString & toDir);

// Copy 'src' to 'dst' if 'src' is newer or 'dst' (or its path) doesn't exist.
// Doesn't work reliably across network shares due to time differences.
void
fgMirrorFile(const FgPath & src,const FgPath & dst);

// Modify permissions to allow all users to write to given file (not directory).
// Of course client must have right to do this:
void
fgMakeWritableByAll(const FgString & name);

#endif
