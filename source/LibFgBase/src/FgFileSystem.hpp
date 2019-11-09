//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

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

namespace Fg {

// Get root all-users application data directory (delimited):
// WARNING: See warning below.
Ustring
fgDirSystemAppDataRoot();

// Get (and create if necessary) the all-users application data directory for the specified application.
// Note that for some users, access is not granted. I am unable to replicate this even if these
// directories are created by an admin user and the file within them is modified by a non-admin user....
Ustring
fgDirSystemAppData(Ustring const & groupName,Ustring const & appName);

// Avoid using Windows 'roaming' directories as they only roam the WDS LAN, not personal cloud
// (see user documents directory below):
Ustring
fgDirUserAppDataRoamingRoot();

// Place to store local app data for this user:
Ustring
fgDirUserAppDataLocalRoot();

// As above but verifies/creates given subPath
Ustring
fgDirUserAppDataLocal(const Svec<String> & subDirs);

// As above but verifies/creates subdirectory for "FaceGen" then for specified:
inline
Ustring
fgDirUserAppDataLocalFaceGen(const String & subd0,const String & subd1)
{return fgDirUserAppDataLocal(fgSvec<String>("FaceGen",subd0,subd1)); }

// Can and does sometimes fail on Windows, possibly when using roaming identities.
// If it fails but 'throwOnFail' is false, it returns the empty string.
// WINDOWS: If user has OneDrive installed, new directories created here will be created within
//     the OneDrive/Documents/ directory instead of the local drive one.
Ustring
fgUserDocumentsDirectory(bool throwOnFail=true);

// This has not been known to fail on Windows:
Ustring
fgPublicDocumentsDirectory();

// Find FaceGen data directory from path of current executable, searching up one directory
// at a time for a directory named 'data' containing the file '_facegen_data_dir.flag'.
// If 'throwIfNotFound' is false, check the return value for the empty string (failure):
const Ustring & dataDir(bool throwIfNotFound=true);

// Manually set data directory. Useful for sandboxed platforms and debugging apps on native
// platforms:
void
fgSetDataDir(const Ustring & dirEndingWithSlash);

// **************************************************************************************
//                          OPERATIONS ON THE FILESYSTEM
// **************************************************************************************

// We can't use boost::filesystem::is_directory inline here since it doesn't work on Windows 10 as of
// 18.04 update.
bool
isDirectory(const Ustring & path);   // Doesn't throw - returns false for invalid path

// Both 'src' and 'dst' must be file paths; 'dst' should not be a directory.
// Note that both the creation time and last modification time are preserved on the copy !
void
fileCopy(const Ustring & srcFilePath,const Ustring & dstFilePath,bool overwrite = false);

// Copy then delete original (safer than rename which doesn't work across volumes):
void
fileMove(const Ustring & srcFilePath,const Ustring & dstFilePath,bool overwrite = false);

// Will not throw, returns false for any kind of failure on 'fname':
bool
pathExists(const Ustring & path);

inline bool
fileExists(Ustring const & fname)
{return (!isDirectory(fname) && pathExists(fname)); }

inline void
fgRename(const Ustring & from,const Ustring & to)
{return boost::filesystem::rename(from.ns(),to.ns()); }

// Update last written time on existing file (will not create). Avoid large files as it current re-writes:
void
fileTouch(Ustring const & fname);

struct      DirectoryContents
{
    Ustrings    filenames;
    Ustrings    dirnames;   // Not including separators
};

// If dirName is a relative path, the current directory is used as the base point.
// The names strings are returned without the path.
// Directory names "." and ".." are NOT returned.
DirectoryContents
directoryContents(const Ustring & dirName);

// Directory names end with a delimiter:
Ustring
fgGetCurrentDir();

bool                                // true if successful
fgSetCurrentDir(
    const Ustring &    dir,        // Accepts full path or relative path
    bool throwOnFail=true);

bool                                // true if successful
fgSetCurrentDirUp();

// Doesn't remove read-only files / dirs:
inline void
pathRemove(const Ustring & fname)
{boost::filesystem::remove(fname.ns()); }

// Ignores read-only or hidden attribs on Windows (identical to pathRemove on nix):
void
fgDeleteFile(const Ustring &);

// Only works on empty dirs, return true if successful:
bool
fgRemoveDirectory(
    const Ustring &    dirName,
    bool                throwOnFail=false);

// Throws on failure:
void
fgRemoveDirectoryRecursive(const Ustring &);      // Full recursive delete

// Accepts full or relative path, but only creates last delimited directory:
bool            // Returns false if the directory already exists, true otherwise
fgCreateDirectory(const Ustring &);

// Create all non-existing directories in given path.
// An undelimited name will be created as a directory:
void
fgCreatePath(const Ustring &);

Ustring                        // Return the full path of the executable
fgExecutablePath();

Ustring                        // Return the full directory of the current application binary
fgExecutableDirectory();

// Returns true if the supplied filename is a file which can be read
// by the calling process:
bool
fileReadable(const Ustring & filename);

String
fgSlurp(Ustring const & filename);

// Setting 'onlyIfChanged' to false will result in the file always being written,
// regardless of whether the new data may be identical.
// Leaving 'true' is useful to avoid triggering unwanted change detections.
bool    // Returns true if the file was written
fgDump(const String & data,const Ustring & filename,bool onlyIfChanged=true);

// Returns true if identical:
bool
equateFilesBinary(Ustring const & file1,Ustring const & file2);

// Returns true if identical except for line endings (LF, CRLF or CR):
// Need to use this for text file regression since some users may have their VCS configured (eg. git default)
// to auto convert all text files to CRLF on Windows. UTF-8 aware.
bool
equateFilesText(Ustring const & fname0,Ustring const & fname1);

// Returns false if the given file or directory cannot be read.
// The returned time is NOT compatible with std raw time and will in fact crash fgDateTimeString().
bool
getCreationTime(const Ustring & path,uint64 & time);

// Works for both files and directories:
// Don't use boost::filesystem::last_write_time(); it doesn't work; returns create time on Win.
std::time_t
getLastWriteTime(const Ustring & node);

// Return true if any of the sources have a 'last write time' newer than any of the sinks,
// of if any of the sinks don't exist (an error results if any of the sources don't exist):
bool
fileNewer(const Ustrings & sources,const Ustrings & sinks);

inline
bool
fileNewer(const Ustring & src,const Ustring & dst)
{return fileNewer(fgSvec(src),fgSvec(dst)); }

// Usually only need to include the one last output of a code chunk as 'dst':
inline
bool
fileNewer(const Ustrings & sources,const Ustring & dst)
{return fileNewer(sources,fgSvec(dst)); }

struct  PushDir
{
    Ustrings    orig;

    // Often need to create in different scope from 'push':
    PushDir() {}

    explicit
    PushDir(const Ustring & dir)
    {push(dir); }

    ~PushDir()
    {
        if (!orig.empty())
            fgSetCurrentDir(orig[0]);
    }

    void push(const Ustring & dir)
    {
        orig.push_back(fgGetCurrentDir());
        fgSetCurrentDir(dir);
    }

    void pop()
    {
        fgSetCurrentDir(orig.back());
        orig.resize(orig.size()-1);
    }

    void change(const Ustring & dir)
    {fgSetCurrentDir(dir); }
};

// Returns name of each matching file & dir:
DirectoryContents
globDirStartsWith(const Path & path);

// Very simple glob - only matches '*' at beginning or end of file base name (but not both
// unless whole name is '*') and/or extension.
// A single '*' does not glob with base and extension.
// Does not glob on input directory name.
// RETURNS: Matching filenames without directory:
Ustrings
globFiles(const Path & path);

// As above but the full path is given by 'basePath + keepPath' and the return paths include 'keepPath':
Ustrings
globFiles(const Ustring & basePath,const Ustring & relPath,const Ustring & filePattern);

// 'toDir' must exist.
// Returns true if there were any files in 'toDir' with the same name as a 'fromDir' file:
bool
fgCopyAllFiles(const Ustring & fromDir,const Ustring & toDir,bool overwrite=false);

// Throws an exception if the filename already exists in the current directory:
void
fgCopyToCurrentDir(const Path & file);

// WARNING: Does not check if dirs are sym/hard links so be careful.
// The tip of 'toDir' will be created.
// Will throw on overwrite of any file or directory:
void
fgCopyRecursive(const Ustring & fromDir,const Ustring & toDir);

// Copy 'src' to 'dst' if 'src' is newer or 'dst' (or its path) doesn't exist.
// Doesn't work reliably across network shares due to time differences.
void
fgMirrorFile(const Path & src,const Path & dst);

// Modify permissions to allow all users to write to given file (not directory).
// Of course client must have right to do this:
void
fgMakeWritableByAll(const Ustring & name);

}

#endif
