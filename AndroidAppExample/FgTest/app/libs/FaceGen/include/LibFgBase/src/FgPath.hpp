//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Jan 27, 2012
//
// Terminology:
//
// Path - any string identifying a directory or file, with relative or absolute path
// Base - the filename without directory or extension
// Dir - the directory (relative or absolute) without the filename. Must end with delimiter.
// Ext - the filename extension (or null string of none) without the period.
// Name - the Base and, if Ext is non null, then a dot then Ext.
//
// Notes:
//
// boost::filesystem::path uses a single string as its internal format, which is not
// very useful.
//

#ifndef FGPATH_HPP
#define FGPATH_HPP

#include "FgStdString.hpp"
#include "FgString.hpp"

#ifdef _WIN32
    #define FG_FS_DIR_SEP "\\"
#else
    #define FG_FS_DIR_SEP "/"
#endif

// Native directory separator converter:
inline string
fgNs(const string & path)
{
#ifdef _WIN32
    return fgReplace(path,'/','\\');
#else
    return fgReplace(path,'\\','/');
#endif
}

inline FgString
fgNfs(const FgString & path)
{
#ifdef _WIN32
    return path.replace('/','\\');
#else
    return path.replace('\\','/');
#endif
}

string
fgDirSep();                         // Directory separator ('/' on Unix, '\' on Windows)

// '.' and '..' are only handled relative to given path string, not 'current' path:
struct  FgPath
{
    // UNC root including initial delimiters (eg //server), in which case 'root' is always true,
    // OR drive letter on Windows (eg C:), in which case 'root' can be either.
    FgString            drive;
    bool                root;   // Path starts at root ? (otherwise relative)
    FgStrings           dirs;   // No delimiters in in dir names. Can begin with '..' entries.
    FgString            base;   // Base filename
    FgString            ext;    // Filename extension (no '.')

    FgPath() : root(false) {}

    // Leaving this implicit allows FgString to be used as an arg for functions taking FgPath.
    // Anything in 'path' not suffixed by a directory delimiter is assumed to be a file.
    FgPath(const FgString & path);

    FgPath(
        const FgString & d,bool r,const FgStrings & ds,
        const FgString & b,const FgString & e)
        : drive(d), root(r), dirs(ds), base(b), ext(e)
    {}

    // All delimiters in native form. All directory names end with delimiter:
    FgString
    str() const;

    // Only the first N directories with no filename, terminated with a delimiter:
    FgString
    dir(size_t n) const;

    // Directory terminated with a delimiter:
    FgString
    dir() const
    {return dir(dirs.size()); }

    FgString
    baseExt() const;

    FgString
    dirBase() const
    {return dir() + base; }

    FgPath
    operator+(const FgPath &  rhs) const;

    // Move up 'num' directories. Throws an error if not possible:
    void
    popDirs(uint num);

    bool
    isDirectory() const
    {return (base.empty() && ext.empty()); }
};

// Ensure last name in path is interpreted as a directory even if it doesn't end with deliminter:
FgPath
fgPathFromDir(const FgString & directory);

FgString
fgPathToBase(const FgString & path);

FgString
fgPathToDirBase(const FgString & path);

FgString
fgPathToExt(const FgString & path);

string
fgPathToExt(const string & path);

// Returns true if 'path' specifies a name with extension 'ext':
bool
fgCheckExt(const FgString & path,const string & ext);

FgString
fgPathToName(const FgString & path);

// Ensure the path ends with a delimiter if it ends with a (directory) name:
FgString
fgAsDirectory(const FgString & path);

string
fgAsDirectory(const string & path);

#endif
