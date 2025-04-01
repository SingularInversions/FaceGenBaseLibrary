//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Cross-platform unicode filename fstreams and other conveniences.
//
// NOTES:
//
// * More convenient to select exception throwing on a per-call basis.
// * Always use binary mode due to complexity of text mode some of which is:
// * In C++ the '\n' character has value 0x0A (LF) on both windows and *nix.
// * When using text mode std::ostream on Windows this is converted to the 2 byte code CR LF
//   (0x0D 0x0A) or ('\r' '\n'). On *nix it is left unchanged.
// * When using text mode std::istream on either platform, any of the various EOL formats are converted to '\n'.
//

#ifndef FGSTDSTREAM_HPP
#define FGSTDSTREAM_HPP

#include "FgSerial.hpp"

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
bool                isAllowedInNodeName(char ascii);
bool                isAllowedInNodeName(char32_t utf32);    // only checks for bad ASCII vals; rest of UTF32 accepted.
bool                isValidNodeName(Str32 const & fname);   // uses above to check all characters. Will NOT accept delimiters.
inline bool         isValidNodeName(String8 const & name) {return isValidNodeName(toUtf32(name.m_str));}
Str32               removeNonFilenameChars(Str32 const & name);      // can return empty

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

struct  Ofstream : public std::ofstream
{
    Ofstream() {}
    explicit Ofstream(String8 const & fname,bool appendFile=false,bool throwOnFail=true)
        {open(fname,appendFile,throwOnFail); }

    bool            open(String8 const & fname,bool appendFile=false,bool throwOnFail=true);
    // raw binary output of simple data types:
    template<class T>
    void            writeBinRaw(T const & val) {write(reinterpret_cast<char const*>(&val),sizeof(val)); }
};

// 32/64 portable file format interface (boost::serialization tends to be incompatible with past versions).
// Just casts size_t to 32 bit and assumes little-endian native and IEEE floats:

template<class T>
void                writeBinRaw_(std::ostream & os,T const & val)    // Only use for builtins !
{
    os.write(reinterpret_cast<char const*>(&val),sizeof(val));
}

struct  Ifstream : public std::ifstream
{
    Ifstream() {}
    explicit Ifstream(String8 const & fname,bool throwOnFail=true) {open(fname,throwOnFail); }

    bool                        open(String8 const & fname,bool throwOnFail=true);
    template<typename T> void   readb_(T & val) {read(reinterpret_cast<char*>(&val),sizeof(val)); }
    template<typename T> T      readBinRaw_()
    {
        T       ret;
        read(reinterpret_cast<char*>(&ret),sizeof(ret));
        return ret;
    }
    Bytes                       readBytes(size_t num);
    String                      readChars(size_t num);
};

template<class T>
void                readBinRaw_(std::istream & is,T & val)
{
    is.read(reinterpret_cast<char*>(&val),sizeof(T));
}

template<class T>
T                   readBinRaw(std::istream & is)
{
    T                   ret;
    readBinRaw_(is,ret);
    return ret;
}

// Always opens in 'binary' mode.
// Throws a descriptive error if the file cannot be opened - never returns nullptr.
FILE *              openFile(String8 const & filename,bool write);  // false = read

// Returns true if the supplied filename is a file which can be read
// by the calling process:
bool                fileReadable(String8 const & filename);
String              loadRawString(String8 const & filename);
Bytes               loadRaw(String8 const & filename);
// Setting 'onlyIfChanged' to false will result in the file always being written,
// regardless of whether the new data may be identical.
// Leaving 'true' is useful to avoid triggering unwanted change detections.
// Returns true if the file was written:
bool                saveRaw(String const & data,String8 const & filename,bool onlyIfChanged=true);
bool                saveRaw(Bytes const & data,String8 const & filename,bool onlyIfChanged=true);
// Creates the file if it doesn't already exist:
void                appendRaw(String8 const & filename,String const & data);
// Returns true if identical:
bool                equateFilesBinary(String8 const & file1,String8 const & file2);
// Returns true if identical except for line endings (LF, CRLF or CR):
// Need to use this for text file regression since some users may have their VCS configured (eg. git default)
// to auto convert all text files to CRLF on Windows. UTF-8 aware.
bool                equateFilesText(String8 const & fname0,String8 const & fname1);
// Returns false if the given file or directory cannot be read.
// On windows, sets time to 100 nanosecond intervals since 1601.01.01
// On Unix, sets time in seconds since 1970.01.01
// The returned time is NOT compatible with std raw time and will in fact crash getDateTimeString().
// Some *nix systems don't support creation time.
// WINE API bug returns last modification time.
// Note that sub-second precision is basically random due to OS filesystem workings.

// Combine serialization with file load/save (load/save is used for whole files, read/write is used for streaming/serialization):
template<class T>
void                saveMessage(T const & val,String8 const & filename)
{
    saveRaw(toMessage(val),filename);
}
template<class T>
void                loadMessage_(String8 const & filename,T & val)
{
    try {
        fromMessage_(loadRaw(filename),val);
    }
    catch (FgException & e) {
        e.contexts.emplace_back("while loading file",filename.m_str);
        throw;
    }
    catch (std::exception const & e) {
        throw FgException {{
            {e.what(),""},
            {"while loading file",filename.m_str},
        }};
    }
    catch (...) {
        throw FgException {{
            {"UNKNOWN",""},
            {"while loading file",filename.m_str},
        }};
    }
}
template<class T>
T                   loadMessage(String8 const & filename)
{
    T                   ret;
    loadMessage_(filename,ret);
    return ret;
}
template<class T>
void                saveMessageExplicit(T const & val,String8 const & filename)
{
    saveRaw(toMessageExplicit(val),filename);
}
template<class T>
T                   loadMessageExplicit(String8 const & filename)
{
    try {
        return fromMessageExplicit<T>(loadRaw(filename));
    }
    catch (FgException & e) {
        e.contexts.emplace_back("while loading file",filename.m_str);
        throw;
    }
    catch (std::exception const & e) {
        throw FgException {{
            {e.what(),""},
            {"while loading file",filename.m_str},
        }};
    }
    catch (...) {
        throw FgException {{
            {"UNKNOWN",""},
            {"while loading file",filename.m_str},
        }};
    }
}

}

#endif

// */
