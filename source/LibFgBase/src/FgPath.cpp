//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgPath.hpp"
#include "FgDiagnostics.hpp"
#include "FgStdVector.hpp"
#include "FgMain.hpp"
#include "FgParse.hpp"

using namespace std;

namespace Fg {

string
fgDirSep()
{
#ifdef _WIN32
    return string("\\");
#else
    return string("/");
#endif
}

// TODO: We don't handle double-delim paths such as //server/share/...
Path::Path(String8 const & pathUtf8)
: root(false)
{
    if (pathUtf8.empty())
        return;
    String32       path = pathUtf8.as_utf32();
    path = replaceAll(path,char32_t('\\'),char32_t('/'));  // VS2013 doesn't support char32_t literal U
    if (path.size() > 1) {
        if ((path[0] == '/') && (path[1] == '/')) {
            root = true;
            auto        it = find(path.begin()+2,path.end(),uint('/'));
            if (it == path.end()) {
                drive = String8(path);
                return;
            }
            size_t      slashPos = it-path.begin();
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
        String32s        s = splitAtChar(path,char32_t('/'));
        for (size_t ii=0; ii<s.size()-1; ++ii) {
            String8    str(s[ii]);
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
    size_t          dotIdx = path.find_last_of('.');
    if (dotIdx == String32::npos)
        base = String8(path);
    else {
        base = String8(path.substr(0,dotIdx));
        ext = String8(path.substr(dotIdx+1));      // Don't include the dot
    }
}

String8
Path::str() const
{
    String8    ret = dir();
    ret += base;
    if (!ext.empty())
        ret += String8(".") + ext;
    return ret;
}

String8
Path::dir(size_t n) const
{
    FGASSERT(n <= dirs.size());
    String8    ret(drive);
    if (root)
        ret += fgDirSep();
    for (size_t ii=0; ii<n; ++ii)
        ret += dirs[ii] + fgDirSep();
    return ret;
}

String8
Path::baseExt() const
{
    String8        ret(base);
    if (!ext.empty())
        ret += String8(".") + ext;
    return ret;
}

Path
Path::operator+(Path const &  rhs) const
{
    FGASSERT((base.empty()) && (ext.empty()));      // lhs is directory only
    FGASSERT((rhs.drive.empty()) && (!rhs.root));   // rhs is relative path
    return
        Path(
            drive,root,
            cat(dirs,rhs.dirs),
            rhs.base,rhs.ext);
}

void
Path::popDirs(uint n)
{
    FGASSERT(n <= dirs.size());
    dirs.resize(dirs.size()-n);
}

Path
pathFromDir(String8 const & directory)
{
    Path      ret(directory);
    if (!ret.base.empty()) {
        ret.dirs.push_back(ret.baseExt());
        ret.base.clear();
        ret.ext.clear();
    }
    return ret;
}

String8
pathToBase(String8 const & f)
{return Path(f).base; }

String8
pathToDirBase(String8 const & p)
{return Path(p).dirBase(); }

String8
pathToExt(String8 const & p)
{return Path(p).ext; }

std::string
pathToExt(const std::string & p)
{return pathToExt(String8(p)).m_str; }

bool
checkExt(String8 const & path,string const & ext)
{
    Path      p(path);
    return (p.ext.toLower() == toLower(ext));
}

String8
setExt(String8 const & path,String const & ext)
{
    Path            p {path};
    p.ext = ext;
    return p.str();
}

String8
pathToName(String8 const & f)
{return Path(f).baseExt(); }

String8
asDirectory(String8 const & path)
{
    String8        ret = path;
    if (path.empty())
        return ret;
    String32       str = path.as_utf32();
    if (str.back() == '/')
        return ret;
    if (str.back() == '\\')
        return ret;
    if (str.back() == ':')
        return  ret;
    ret.m_str += '/';
    return ret;
}

string
asDirectory(string const & path)
{return asDirectory(String8(path)).m_str; }

void
testPath(CLArgs const &)
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

}
