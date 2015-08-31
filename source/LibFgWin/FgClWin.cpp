//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: April 21, 2011
//

#include "stdafx.h"

#include "FgCl.hpp"
#include "FgException.hpp"

using namespace std;

namespace fgCl
{

void
unzip(const string & fname)
{
    run("\"C:\\Program Files\\7-Zip\\7z.exe\" x "+fname+" >> log.txt");
}

void
zip(const string & dir,bool oldFormat)
{
    string      ext = (oldFormat ? ".zip " : ".7z ");
    run("\"C:\\Program Files\\7-Zip\\7z.exe\" a "+dir+ext+dir+" >> log.txt");
}

void
ren(const string & from,const string & to)
{
    run("ren "+from+" "+to);
}

void
copy(const string & from,const string & to)
{
    run("copy "+from+" "+to);
}

void
copyDeep(string from,const string & to)
{
    if (from[from.size()-1] == '\\')    // xcopy doesn't accept source dir ending with delimiter.
        from.erase(from.end()-1);
    run("echo d | xcopy /e /q "+from+" "+to);
}

void
del(const string & file)
{
    run("del "+file);
}

void
mkdir(const string & dir)
{
    run("if not exist " + dir + " md " + dir);
}

void
move(const string & from,const string & to)
{
    run("move "+from+" "+to);
}

void
rm_r(const std::string & dir)
{
    run("if exist " + dir + " rd /s /q " + dir);
}

}   // namespace
