//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
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

#include "FgString.hpp"

namespace Fg {

// directory separator for current platform ('\' on Windows, '/' otherwise):
char constexpr      nativeDirSep8 =
#ifdef _WIN32
    '\\';
#else
    '/';
#endif
char32_t constexpr  nativeDirSep32 = 
#ifdef _WIN32
    U'\\';
#else
    U'/';
#endif

// ensure all directory separators are native separators:
String              toNativeDirSep(String const & pathAscii);
String8             toNativeDirSep(String8 const & pathUtf8);
bool                isAllowedInFilename(char ascii);
bool                isAllowedInFilename(char32_t utf32);    // only checks for bad ASCII vals; rest of UTF32 accepted.
String32            removeNonFilenameChars(String32 const & name);      // can return empty

// '.' and '..' are only handled relative to given path string, not 'current' path:
struct      Path
{
    // UNC root including initial delimiters (eg //server), in which case 'root' is always true,
    // OR drive letter on Windows (eg C:), in which case 'root' can be either
    String8            drive;
    bool               root;   // Path starts at root ? (eg. /path/, otherwise relative)
    String8s           dirs;   // No delimiters in in dir names. Can begin with '..' entries.
    String8            base;   // Base filename
    String8            ext;    // Filename extension (no '.')

    Path() : root(false) {}
    // Leaving this implicit allows String8 to be used as an arg for functions taking Path.
    // Anything in 'path' not suffixed by a directory delimiter is assumed to be a file.
    Path(String8 const & path);
    Path(String8 const & d,bool r,String8s const & ds,String8 const & b,String8 const & e)
        : drive(d), root(r), dirs(ds), base(b), ext(e)
    {}

    // All delimiters in native form. All directory names end with delimiter:
    String8             str() const;
    // Drive and root delimiter if specified, first 'n' directories, terminated with delimiter (no filename):
    String8             dir(size_t n) const;
    // As above with full directory path:
    String8             dir() const {return dir(dirs.size()); }
    String8             baseExt() const;
    String8             dirBase() const {return dir() + base; }
    Path                operator+(Path const &  rhs) const;
    // Move up 'num' directories. Throws an error if not possible:
    void                popDirs(uint num);
    bool                isDirectory() const {return (base.empty() && ext.empty()); }
};

// Ensure last name in path is interpreted as a directory even if it doesn't end with deliminter:
Path                pathFromDir(String8 const & directory);
String8             pathToBase(String8 const & path);
String8             pathToDirBase(String8 const & path);
String8             pathToExt(String8 const & path);
String              pathToExt(String const & path);
// Returns true if 'path' specifies a name with extension 'ext':
bool                checkExt(String8 const & path,String const & ext);
// Ensure the filename extension is set to 'ext':
String8             setExt(String8 const & path,String const & ext);
String8             pathToName(String8 const & path);
// Ensure the path ends with a delimiter if it ends with a (directory) name:
String8             asDirectory(String8 const & path);
String              asDirectory(String const & path);

}

#endif
