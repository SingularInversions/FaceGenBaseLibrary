//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
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

namespace Fg {

#ifdef _WIN32
    #define FG_FS_DIR_SEP "\\"
#else
    #define FG_FS_DIR_SEP "/"
#endif

// Native directory separator converter:
inline String
toNativeSeparator(String const & path)
{
#ifdef _WIN32
    return replaceAll(path,'/','\\');
#else
    return replaceAll(path,'\\','/');
#endif
}

inline Ustring
fgNfs(Ustring const & path)
{
#ifdef _WIN32
    return path.replace('/','\\');
#else
    return path.replace('\\','/');
#endif
}

String
fgDirSep();                         // Directory separator ('/' on Unix, '\' on Windows)

// '.' and '..' are only handled relative to given path string, not 'current' path:
struct  Path
{
    // UNC root including initial delimiters (eg //server), in which case 'root' is always true,
    // OR drive letter on Windows (eg C:), in which case 'root' can be either.
    Ustring            drive;
    bool               root;   // Path starts at root ? (otherwise relative)
    Ustrings           dirs;   // No delimiters in in dir names. Can begin with '..' entries.
    Ustring            base;   // Base filename
    Ustring            ext;    // Filename extension (no '.')

    Path() : root(false) {}

    // Leaving this implicit allows Ustring to be used as an arg for functions taking Path.
    // Anything in 'path' not suffixed by a directory delimiter is assumed to be a file.
    Path(Ustring const & path);

    Path(
        Ustring const & d,bool r,Ustrings const & ds,
        Ustring const & b,Ustring const & e)
        : drive(d), root(r), dirs(ds), base(b), ext(e)
    {}

    // All delimiters in native form. All directory names end with delimiter:
    Ustring
    str() const;

    // Only the first N directories with no filename, terminated with a delimiter:
    Ustring
    dir(size_t n) const;

    // Directory terminated with a delimiter:
    Ustring
    dir() const
    {return dir(dirs.size()); }

    Ustring
    baseExt() const;

    Ustring
    dirBase() const
    {return dir() + base; }

    Path
    operator+(const Path &  rhs) const;

    // Move up 'num' directories. Throws an error if not possible:
    void
    popDirs(uint num);

    bool
    isDirectory() const
    {return (base.empty() && ext.empty()); }
};

// Ensure last name in path is interpreted as a directory even if it doesn't end with deliminter:
Path
pathFromDir(Ustring const & directory);

Ustring
pathToBase(Ustring const & path);

Ustring
pathToDirBase(Ustring const & path);

Ustring
pathToExt(Ustring const & path);

String
pathToExt(String const & path);

// Returns true if 'path' specifies a name with extension 'ext':
bool
checkExt(Ustring const & path,String const & ext);

Ustring
pathToName(Ustring const & path);

// Ensure the path ends with a delimiter if it ends with a (directory) name:
Ustring
asDirectory(Ustring const & path);

String
asDirectory(String const & path);

}

#endif
