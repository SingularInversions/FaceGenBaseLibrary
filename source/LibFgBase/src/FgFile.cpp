//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgFile.hpp"
#include "FgParse.hpp"
#include "FgCommand.hpp"
#include "FgTestUtils.hpp"

using namespace std;

namespace Fg {

char constexpr      otherDirSep8 =
#ifdef _WIN32
    '/';
#else
    '\\';
#endif
char32_t constexpr  otherDirSep32 = 
#ifdef _WIN32
    U'/';
#else
    U'\\';
#endif

String              toNativeDirSep(String const & path)
{
    String          ret = path;
    for (char & ch : ret)
        if (ch == otherDirSep8)
            ch = nativeDirSep8;
    return ret;
}

String8             toNativeDirSep(String8 const & path)
{
    Str32            ret = path.as_utf32();
    for (char32_t & ch : ret)
        if (ch == otherDirSep32)
            ch = nativeDirSep32;
    return String8{ret};
}

bool                isAllowedInNodeName(char ascii)
{
    uint            asciiu = scast<uint>(ascii);
    if (asciiu < 32)            // control codes
        return false;
    char            bad[] = {'<','>',':','"','/','\\','|','?','*'};
    size_t          sz = sizeof(bad);
    for (size_t cc=0; cc<sz; ++cc)
        if (ascii == bad[cc])
            return false;
    return asciiu < 127;        // DEL
}

bool                isAllowedInNodeName(char32_t utf32)
{
    if (scast<uint32>(utf32) < 127)
        return isAllowedInNodeName(scast<char>(utf32));
    else
        return true;
}

bool                isValidNodeName(Str32 const & fn32)
{
    if (fn32.empty())
        return false;
    for (char32_t ch : fn32)
        if (!isAllowedInNodeName(ch))
            return false;
    if (fn32.back() == '.')
        return false;
    return true;
}

Str32               removeNonFilenameChars(Str32 const & name)
{
    Str32            ret;
    for (char32_t ch : name)
        if (isAllowedInNodeName(ch))
            ret.push_back(ch);
    return ret;
}

// TODO: We don't handle double-delim paths such as //server/share/...
Path::Path(String8 const & pathUtf8) : root(false)
{
    if (pathUtf8.empty())
        return;
    Str32            path = pathUtf8.as_utf32();
    path = replaceAll(path,char32_t('\\'),char32_t('/'));  // VS2013 doesn't support char32_t literal U
    if (path.size() > 1) {
        if ((path[0] == '/') && (path[1] == '/')) {
            root = true;
            auto                it = find(path.begin()+2,path.end(),uint('/'));
            if (it == path.end()) {
                drive = String8(path);
                return;
            }
            size_t              slashPos = it-path.begin();
            drive = String8(cHead(path,slashPos));
            path = cRest(path,slashPos);
        }
        // Strictly else since we don't combine UNC and LFS:
        else if (path[1] == ':') {
            drive = String8(cHead(path,2));
            path = cRest(path,2);
        }
    }
    if (path.empty())
        return;
    if (path[0] == '/') {
        root = true;
        path = cRest(path,1);
    }
    if (path.empty())
        return;
    if (contains(path,char32_t('/'))) {
        Str32s           s = splitAtChar(path,char32_t('/'));
        for (size_t ii=0; ii<s.size()-1; ++ii) {
            String8             str(s[ii]);
            if ((str == "..") && (!dirs.empty())) {
                if (String8(dirs.back()) == str)
                    dirs.push_back(str);            // '..' does not back up over '..' !
                else
                    dirs = cHead(dirs,dirs.size()-1);
            }
            else if (str != ".")
                dirs.push_back(String8(s[ii]));
        }
        if (!s.back().empty())
            path = s.back();
        else
            path.clear();
    }
    if (path.empty())
        return;
    size_t              dotIdx = path.find_last_of('.');
    if (dotIdx == Str32::npos)
        base = String8(path);
    else {
        base = String8(path.substr(0,dotIdx));
        ext = String8(path.substr(dotIdx+1));      // Don't include the dot
    }
}

String8             Path::str() const
{
    String8    ret = dir();
    ret += base;
    if (!ext.empty())
        ret += String8(".") + ext;
    return ret;
}

String8             Path::dir(size_t numDirs) const
{
    FGASSERT(numDirs <= dirs.size());
    String8         ret = drive;
    if (root)
        ret += nativeDirSep8;
    for (size_t dd=0; dd<numDirs; ++dd) {
        String8 const &     dir = dirs[dd];
        ret += dir;
        ret += nativeDirSep8;
    }
    return ret;
}

String8             Path::baseExt() const
{
    String8        ret(base);
    if (!ext.empty())
        ret += String8(".") + ext;
    return ret;
}

Path                Path::operator+(Path const &  rhs) const
{
    FGASSERT((base.empty()) && (ext.empty()));      // lhs is directory only
    FGASSERT((rhs.drive.empty()) && (!rhs.root));   // rhs is relative path
    return
        Path(
            drive,root,
            cat(dirs,rhs.dirs),
            rhs.base,rhs.ext);
}

void                Path::popDirs(uint n)
{
    FGASSERT(n <= dirs.size());
    dirs.resize(dirs.size()-n);
}

Path                pathFromDir(String8 const & directory)
{
    Path      ret(directory);
    if (!ret.base.empty()) {
        ret.dirs.push_back(ret.baseExt());
        ret.base.clear();
        ret.ext.clear();
    }
    return ret;
}

String8             pathToBase(String8 const & f) {return Path(f).base; }
String8             pathToDirBase(String8 const & p) {return Path(p).dirBase(); }
String8             pathToExt(String8 const & p) {return Path(p).ext; }
String              pathToExt(String const & p) {return pathToExt(String8(p)).m_str; }

bool                checkExt(String8 const & path,String const & ext)
{
    Path      p(path);
    return (p.ext.toLower() == toLower(ext));
}

String8             setExt(String8 const & path,String const & ext)
{
    Path            p {path};
    p.ext = ext;
    return p.str();
}

String8             pathToName(String8 const & f) {return Path(f).baseExt(); }

String8             asDirectory(String8 const & path)
{
    String8        ret = path;
    if (path.empty())
        return ret;
    Str32       str = path.as_utf32();
    if (str.back() == '/')
        return ret;
    if (str.back() == '\\')
        return ret;
    if (str.back() == ':')
        return  ret;
    ret.m_str += '/';
    return ret;
}

String              asDirectory(String const & path) {return asDirectory(String8(path)).m_str; }

bool                fileReadable(String8 const & filename)
{
    Ifstream ifs(filename,false);
    if (!ifs.is_open()) return false;
    return true;
}

String              loadRawString(String8 const & filename)
{
    Ifstream            ifs {filename};
    ostringstream       ss;
    ss << ifs.rdbuf();
    return ss.str();
}

Bytes               loadRaw(String8 const & filename)
{
    // * Files can be appended while being read so ifstream offers no way to get file size (tellg() is NOT guaranteed to work).
    // * Tried using std::filesystem::file_size() but on Windows this doesn't get updated quickly so recently downloaded files were
    //    corrupted.
    // * Seems the only simple and reliable solution is to read until you can read no more:
    size_t constexpr    S = 4096;               // read block size. Arbitrary, hasn't been optimized
    Bytes               ret;
    Ifstream            ifs {filename};         // throws on fail
    do {
        size_t              P = ret.size();
        ret.resize(P + S);
        ifs.read(reinterpret_cast<char*>(&ret[P]),scast<streamsize>(S));
    }
    while (ifs.gcount() == S);                      // returns the actual number of bytes read
    ret.resize((ret.size() + ifs.gcount()) - S);    // size to the remainder
    return ret;
}

bool                saveRaw(String const & data,String8 const & filename,bool onlyIfChanged)
{
    if (onlyIfChanged && pathExists(filename)) {
        string      fileData = loadRawString(filename);
        if (data == fileData)
            return false;
    }
    Ofstream            ofs(filename);
    ofs << data;
    return true;
}

bool                saveRaw(Bytes const & data,String8 const & filename,bool onlyIfChanged)
{
    if (onlyIfChanged && pathExists(filename)) {
        Bytes           fileData = loadRaw(filename);
        if (fileData == data)
            return false;
    }
    Ofstream            ofs {filename};
    if (!data.empty())
        ofs.write(reinterpret_cast<char const *>(&data[0]),data.size());
    if (ofs.fail())
        fgThrow("saveRaw error writing file",filename);
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
    string contents1(loadRawString(file1));
    string contents2(loadRawString(file2));
    return contents1 == contents2;
}

bool                equateFilesText(String8 const & fname0,String8 const & fname1)
{
    return (splitLinesUtf8(loadRawString(fname0)) == splitLinesUtf8(loadRawString(fname1)));
}

void                testPath(CLArgs const &)
{
    Path  p;

    p = Path("file");
    FGASSERT(p.drive.empty());
    FGASSERT(!p.root);
    FGASSERT(p.dirs.empty());
    FGASSERT(p.base == "file");
    FGASSERT(p.ext.empty());

    p = Path("file.ext");
    FGASSERT(p.drive.empty());
    FGASSERT(!p.root);
    FGASSERT(p.dirs.empty());
    FGASSERT(p.base == "file");
    FGASSERT(p.ext == "ext");

    p = Path("reldir/");
    FGASSERT(p.drive.empty());
    FGASSERT(!p.root);
    FGASSERT(p.dirs.size() == 1);
    FGASSERT(p.dirs[0] == "reldir");
    FGASSERT(p.base.empty());
    FGASSERT(p.ext.empty());

    p = Path("/absdir/");
    FGASSERT(p.drive.empty());
    FGASSERT(p.root);
    FGASSERT(p.dirs.size() == 1);
    FGASSERT(p.dirs[0] == "absdir");
    FGASSERT(p.base.empty());
    FGASSERT(p.ext.empty());

    p = Path("C:file");
    FGASSERT(p.drive == "C:");
    FGASSERT(!p.root);
    FGASSERT(p.dirs.empty());
    FGASSERT(p.base == "file");
    FGASSERT(p.ext.empty());

    p = Path("/d.0/d.1/d.2/file");
    FGASSERT(p.drive.empty());
    FGASSERT(p.root);
    FGASSERT(p.dirs.size() == 3);
    FGASSERT(p.dirs[2] == "d.2");
    FGASSERT(p.base == "file");
    FGASSERT(p.ext.empty());

    p = Path("dir/../");
    FGASSERT(p.drive.empty());
    FGASSERT(!p.root);
    FGASSERT(p.dirs.empty());
    FGASSERT(p.base.empty());
    FGASSERT(p.ext.empty());

    p = Path("C:../");
    FGASSERT(p.drive == "C:");
    FGASSERT(!p.root);
    FGASSERT(p.dirs.size() == 1);
    FGASSERT(p.dirs[0] == "..");
    FGASSERT(p.base.empty());
    FGASSERT(p.ext.empty());

    p = Path("//computer");
    FGASSERT(p.drive == "//computer");

    p = Path("//computer/some/path/");
    FGASSERT(p.drive == "//computer");
    FGASSERT(p.dirs.size() == 2);
}

#ifdef _WIN32

// File I/O defined in LibFgWin/FgFileWin.cpp

#else

bool                Ofstream::open(String8 const & fname,bool appendFile,bool throwOnFail)
{
    // Always use binary read/write. Text translation mode is complex, error-prone and unnecessary:
    ios::openmode       om = ios::binary;
    if (appendFile)
        om = om | ios::app;
    ofstream::open(fname.m_str.c_str(),om);                     // this will not throw
    if (is_open())
        return true;
    if (throwOnFail)
        fgThrow("Unable to open file for writing",fname);
    return false;
}

bool                Ifstream::open(String8 const & fname,bool throwOnFail)
{
    ifstream::open(fname.m_str.c_str(),std::ios::binary);       // this will not throw
    if (is_open())
        return true;
    if (throwOnFail)
        fgThrow("Unable to open file for reading",fname);
    return false;
}

FILE *              openFile(String8 const & fname,bool write)
{
    FILE *          fPtr;
    char const *    mode = write ? "wb" : "rb";
    fPtr = fopen(fname.m_str.c_str(),mode);
    if (fPtr == nullptr) {
        string          msg = "Unable to open file for " + string(write ? "writing" : "reading");
        fgThrow(msg,fname.m_str);
    }
    return fPtr;
}

#endif

Bytes               Ifstream::readBytes(size_t num)
{
    Bytes               ret;
    if (num > 0) {
        ret.resize(num);
        read(reinterpret_cast<char *>(ret.data()),num);
        if (fail())
            fgThrow("Unable to read more bytes from file");
    }
    return ret;
}
String              Ifstream::readChars(size_t num)
{
    String              ret;
    if (num > 0) {
        ret.resize(num);
        read(&ret[0],num);
        if (fail())
            fgThrow("Unable to read more chars from file");
    }
    return ret;
}

void                testOpenFile(CLArgs const & args)
{
    FGTESTDIR;
    char32_t        ch = 0x00004EE5;            // A Chinese character
    String8        chinese(ch);
    string          data = "test data";
    FILE *          fPtr = openFile(chinese,true);
    fwrite(data.data(),1,data.size(),fPtr);
    fclose(fPtr);
    string          test;
    test.resize(data.size());
    fPtr = openFile(chinese,false);
    size_t          sz = fread(&test[0],1,test.size(),fPtr);
    fclose(fPtr);
    FGASSERT(sz == data.size());
    FGASSERT(test == data);
}

void                testMetaFormat(CLArgs const &)
{
    TestDir             td("MetaFormat");
    int                 a = 42;
    saveMessage(a,"test.fgbin");
    int                 b;
    loadMessage_("test.fgbin",b);
    FGASSERT(a == b);
}

}

// */
