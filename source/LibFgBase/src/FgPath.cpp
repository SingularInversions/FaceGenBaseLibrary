//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Jan 27, 2012
//

#include "stdafx.h"

#include "FgPath.hpp"
#include "FgDiagnostics.hpp"
#include "FgStdVector.hpp"
#include "FgMain.hpp"

using namespace std;

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
FgPath::FgPath(const FgString & path)
: root(false)
{
    if (path.empty())
        return;
    vector<uint>    p = path.as_utf32();
    p = fgReplace(p,uint('\\'),uint('/'));
    if ((p.size() > 1) && (p[1] == ':')) {
        drive = FgString(fgHead(p,2));
        p = fgRest(p,2);
    }
    if (!p.empty() && (p[0] == '/')) {
        root = true;
        p = fgRest(p,1);
    }
    if (fgContains(p,uint('/'))) {
        vector<vector<uint> >   s = fgSplit(p,uint('/'));
        for (size_t ii=0; ii<s.size()-1; ++ii) {
            FgString    str(s[ii]);
            if ((str == "..") && (!dirs.empty())) {
                if (FgString(dirs.back()) == str)
                    dirs.push_back(str);            // '..' does not back up over '..' !
                else
                    dirs = fgHead(dirs,dirs.size()-1);
            }
            else if (str != ".")
                dirs.push_back(FgString(s[ii]));
        }
        if (!s.back().empty())
            p = s.back();
        else
            p.clear();
    }
    if (!p.empty()) {
        if (fgContains(p,uint('.'))) {
            size_t  idx = fgFindLastIdx(p,uint('.'));
            base = FgString(fgHead(p,idx));
            ext = FgString(fgRest(p,idx+1));    // Don't include the dot
        }
        else
            base = FgString(p);
    }
}

FgString
FgPath::str() const
{
    FgString    ret = dir();
    ret += base;
    if (!ext.empty())
        ret += FgString(".") + ext;
    return ret;
}

FgString
FgPath::dir(size_t n) const
{
    FGASSERT(n <= dirs.size());
#ifndef _WIN32
    FGASSERT(drive.empty());
#endif
    FgString    ret(drive);
    if (root)
        ret += fgDirSep();
    for (size_t ii=0; ii<n; ++ii)
        ret += dirs[ii] + fgDirSep();
    return ret;
}

FgString
FgPath::baseExt() const
{
    FgString        ret(base);
    if (!ext.empty())
        ret += FgString(".") + ext;
    return ret;
}

FgPath
FgPath::operator+(const FgPath &  rhs) const
{
    FGASSERT((base.empty()) && (ext.empty()));      // lhs is directory only
    FGASSERT((rhs.drive.empty()) && (!rhs.root));   // rhs is relative path
    return
        FgPath(
            drive,root,
            fgConcat(dirs,rhs.dirs),
            rhs.base,rhs.ext);
}

void
FgPath::popDirs(uint n)
{
    FGASSERT(n <= dirs.size());
    dirs.resize(dirs.size()-n);
}

FgString
fgPathToBase(const FgString & f)
{return FgPath(f).base; }

FgString
fgPathToDirBase(const FgString & p)
{return FgPath(p).dirBase(); }

FgString
fgPathToExt(const FgString & p)
{return FgPath(p).ext; }

std::string
fgPathToExt(const std::string & p)
{return fgPathToExt(FgString(p)).ascii(); }

FgString
fgPathToName(const FgString & f)
{return FgPath(f).baseExt(); }

bool
fgCheckSetExtension(
    std::string &       filename,
    const std::string & extension)
{
    FgPath      p(filename);
    if (p.ext.empty())
        filename = filename + "." + extension;
    else if (fgToLower(p.ext.ascii()) != fgToLower(extension))
        return false;
    return true;
}

FgString
fgAsDirectory(const FgString & path)
{
    FgString        ret = path;
    if (path.empty())
        return ret;
    vector<uint>    str = path.as_utf32();
    if (str.back() == uint('/'))
        return ret;
    if (str.back() == uint('\\'))
        return ret;
    if (str.back() == uint(':'))
        return  ret;
    ret.m_str += '/';
    return ret;
}

void
fgPathTest(const FgArgs &)
{
    FgPath  p;

    p = FgPath("file");
    FGASSERT(p.drive.empty());
    FGASSERT(!p.root);
    FGASSERT(p.dirs.empty());
    FGASSERT(p.base == "file");
    FGASSERT(p.ext.empty());

    p = FgPath("file.ext");
    FGASSERT(p.drive.empty());
    FGASSERT(!p.root);
    FGASSERT(p.dirs.empty());
    FGASSERT(p.base == "file");
    FGASSERT(p.ext == "ext");

    p = FgPath("reldir/");
    FGASSERT(p.drive.empty());
    FGASSERT(!p.root);
    FGASSERT(p.dirs.size() == 1);
    FGASSERT(p.dirs[0] == "reldir");
    FGASSERT(p.base.empty());
    FGASSERT(p.ext.empty());

    p = FgPath("/absdir/");
    FGASSERT(p.drive.empty());
    FGASSERT(p.root);
    FGASSERT(p.dirs.size() == 1);
    FGASSERT(p.dirs[0] == "absdir");
    FGASSERT(p.base.empty());
    FGASSERT(p.ext.empty());

    p = FgPath("C:file");
    FGASSERT(p.drive == "C:");
    FGASSERT(!p.root);
    FGASSERT(p.dirs.empty());
    FGASSERT(p.base == "file");
    FGASSERT(p.ext.empty());

    p = FgPath("/d.0/d.1/d.2/file");
    FGASSERT(p.drive.empty());
    FGASSERT(p.root);
    FGASSERT(p.dirs.size() == 3);
    FGASSERT(p.dirs[2] == "d.2");
    FGASSERT(p.base == "file");
    FGASSERT(p.ext.empty());

    p = FgPath("dir/../");
    FGASSERT(p.drive.empty());
    FGASSERT(!p.root);
    FGASSERT(p.dirs.empty());
    FGASSERT(p.base.empty());
    FGASSERT(p.ext.empty());

    p = FgPath("C:../");
    FGASSERT(p.drive == "C:");
    FGASSERT(!p.root);
    FGASSERT(p.dirs.size() == 1);
    FGASSERT(p.dirs[0] == "..");
    FGASSERT(p.base.empty());
    FGASSERT(p.ext.empty());
}
