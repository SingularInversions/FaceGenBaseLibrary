//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     May 5, 2005
//

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

// Get (and create if necessary) the all-users application data directory for the
// specified application:
// WARNING: On windows, modifying a file here that was not created by the same user
// requires administrator privelege, which means that it won't work even for other admins
// due to UAC. Creating new files within a directory created by other users is OK.
FgString
fgDirSystemAppData(
    FgString const &    groupName,
    FgString const &    appName);

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

FgString
fgUserDocumentsDirectory();

FgString
fgPublicDocumentsDirectory();

// Find SDK data directory from path of current executable:
const FgString &
fgDataDir();

// **************************************************************************************
//                          OPERATIONS ON THE FILESYSTEM
// **************************************************************************************

void
fgCopyFile(const FgString & src,const FgString & dst,bool overwrite = false);

// Copy then delete original (safer than rename which doesn't work across volumes):
void
fgMoveFile(const FgString & src,const FgString & dst,bool overwrite = false);

inline bool
fgExists(const FgString & fname)
{return boost::filesystem::exists(fname.ns()); }

inline bool
fgRemove(const FgString & fname)
{return boost::filesystem::remove(fname.ns()); }

// Recursive descent remove:
inline size_t
fgRemoveAll(const FgString & fname)
{return size_t(boost::filesystem::remove_all(fname.ns())); }

inline void
fgRename(const FgString & from,const FgString & to)
{return boost::filesystem::rename(from.ns(),to.ns()); }

// is_directory doesn't throw:
inline bool
fgIsDirectory(const FgString & name)
{return boost::filesystem::is_directory(name.ns()); }

struct      FgDirectoryContents
{
    vector<FgString>    filenames;
    vector<FgString>    dirnames;   // Not including separators
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

// Doesn't remove read-only files:
void
fgRemoveFile(const FgString &);

// Ignores read-only or hidden attribs (Windows only):
void
fgDeleteFile(const FgString &);

// Only works on empty dirs, return true if successful:
bool
fgRemoveDirectory(
    const FgString &    dirName,
    bool                throwOnFail=false);

void
fgDeleteDirectory(const FgString &);      // Full recursive delete

bool            // Returns false if the directory already exists, true otherwise
fgCreateDirectory(const FgString &);  // Accepts full or relative path,
                                        // but only creates last delimited directory
inline void
fgFsCreateSetDirectory(const FgString & d) 
    {fgCreateDirectory(d); fgSetCurrentDir(d); }

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

void
fgDump(
    const std::string & data,
    const FgString &    filename);

// Returns true if identical:
bool
fgBinaryFileCompare(
    const FgString & file1,
    const FgString & file2);

// Returns false if the given file or directory cannot be read.
// The returned time is NOT compatible with std raw time and will in fact crash fgDateTime():
bool
fgCreationTime(const FgString & path,uint64 & time);

// Works for both files and directories:
inline
std::time_t
fgLastWriteTime(const FgString & path)
{return boost::filesystem::last_write_time(path.as_utf8_string()); }

// Return true if source has a 'last write time' newer than sink, or the sink doesn't exist:
inline
bool
fgNewer(const FgString & source,const FgString & sink)
{return (!fgExists(sink) || (fgLastWriteTime(source) > fgLastWriteTime(sink))); }

// Return true if any of the sources have a 'last write time' newer than any of the sinks:
bool
fgNewer(const vector<FgString> & sources,const vector<FgString> & sinks);

struct  FgPushDir
{
    std::vector<FgString>    orig;

    // Often need to create in different scope from 'push':
    FgPushDir() {}

    explicit
    FgPushDir(const FgString & dir)
    {push(dir); }

    ~FgPushDir() {
        if (!orig.empty())
            fgSetCurrentDir(orig[0]); }

    void push(const FgString & dir) {
        orig.push_back(fgGetCurrentDir());
        fgSetCurrentDir(dir); }

    void pop() {
        fgSetCurrentDir(orig.back());
        orig.resize(orig.size()-1); }
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
vector<FgString>
fgGlobFiles(const FgPath & path);

// Throws an exception if the filename already exists in the current directory:
void
fgCopyToCurrentDir(const FgPath & file);

// WARNING: Does not check if dirs are sym/hard links so be careful.
// The tip of 'toDir' will be created.
// Will throw on overwrite of any file or directory:
void
fgCopyRecursive(const FgString & fromDir,const FgString & toDir);

#endif
