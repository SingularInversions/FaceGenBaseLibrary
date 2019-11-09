//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

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
Path::Path(const Ustring & path)
: root(false)
{
    if (path.empty())
        return;
    u32string       p = path.as_utf32();
    p = fgReplace(p,char32_t('\\'),char32_t('/'));  // VS2013 doesn't support char32_t literal U
    if (p.size() > 1) {
        if ((p[0] == '/') && (p[1] == '/')) {
            root = true;
            auto        it = find(p.begin()+2,p.end(),uint('/'));
            if (it == p.end()) {
                drive = Ustring(p);
                return;
            }
            size_t      slashPos = it-p.begin();
            drive = Ustring(fgHead(p,slashPos));
            p = fgRest(p,slashPos);
        }
        // Strictly else since we don't combine UNC and LFS:
        else if (p[1] == ':') {
            drive = Ustring(fgHead(p,2));
            p = fgRest(p,2);
        }
    }
    if (p.empty())
        return;
    if (p[0] == '/') {
        root = true;
        p = fgRest(p,1);
    }
    if (p.empty())
        return;
    if (fgContains(p,char32_t('/'))) {
        FgStr32s        s = fgSplit(p,char32_t('/'));
        for (size_t ii=0; ii<s.size()-1; ++ii) {
            Ustring    str(s[ii]);
            if ((str == "..") && (!dirs.empty())) {
                if (Ustring(dirs.back()) == str)
                    dirs.push_back(str);            // '..' does not back up over '..' !
                else
                    dirs = fgHead(dirs,dirs.size()-1);
            }
            else if (str != ".")
                dirs.push_back(Ustring(s[ii]));
        }
        if (!s.back().empty())
            p = s.back();
        else
            p.clear();
    }
    if (p.empty())
        return;
    if (fgContains(p,char32_t('.'))) {
        size_t      idx = fgFindLastIdx(p,char32_t('.'));
        base = Ustring(fgHead(p,idx));
        ext = Ustring(fgRest(p,idx+1));    // Don't include the dot
    }
    else
        base = Ustring(p);
}

Ustring
Path::str() const
{
    Ustring    ret = dir();
    ret += base;
    if (!ext.empty())
        ret += Ustring(".") + ext;
    return ret;
}

Ustring
Path::dir(size_t n) const
{
    FGASSERT(n <= dirs.size());
    Ustring    ret(drive);
    if (root)
        ret += fgDirSep();
    for (size_t ii=0; ii<n; ++ii)
        ret += dirs[ii] + fgDirSep();
    return ret;
}

Ustring
Path::baseExt() const
{
    Ustring        ret(base);
    if (!ext.empty())
        ret += Ustring(".") + ext;
    return ret;
}

Path
Path::operator+(const Path &  rhs) const
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
fgPathFromDir(const Ustring & directory)
{
    Path      ret(directory);
    if (!ret.base.empty()) {
        ret.dirs.push_back(ret.baseExt());
        ret.base.clear();
        ret.ext.clear();
    }
    return ret;
}

Ustring
fgPathToBase(const Ustring & f)
{return Path(f).base; }

Ustring
fgPathToDirBase(const Ustring & p)
{return Path(p).dirBase(); }

Ustring
fgPathToExt(const Ustring & p)
{return Path(p).ext; }

std::string
fgPathToExt(const std::string & p)
{return fgPathToExt(Ustring(p)).m_str; }

bool
fgCheckExt(const Ustring & path,const string & ext)
{
    Path      p(path);
    return (p.ext.toLower() == fgToLower(ext));
}

Ustring
fgPathToName(const Ustring & f)
{return Path(f).baseExt(); }

Ustring
fgAsDirectory(const Ustring & path)
{
    Ustring        ret = path;
    if (path.empty())
        return ret;
    u32string       str = path.as_utf32();
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
fgAsDirectory(const string & path)
{return fgAsDirectory(Ustring(path)).m_str; }

void
fgPathTest(const CLArgs &)
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
