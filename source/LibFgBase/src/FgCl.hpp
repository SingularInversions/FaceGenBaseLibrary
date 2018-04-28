//
// Copyright (C); Singular Inversions Inc. (facegen.com) 2011
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     April 21, 2011
//
// Command-line invocations (currently only defined in LibFgWin)

#ifndef FGCL_HPP
#define FGCL_HPP

#include "FgStdLibs.hpp"

namespace fgCl
{

extern bool preview;

// Returns true if success, false if failed and throwIfError=false
bool
run(const std::string & cmd,bool throwIfError=true,int rvalMask=0xFFFF);

void
unzip(const std::string & fname);

void
zip(const std::string & dir,bool oldFormat=false);

void
ren(const std::string & from,const std::string & to);

void
copy(const std::string & from,const std::string & to);

void
copyDeep(std::string from,const std::string & to);

void
del(const std::string & file);

// Creates the full directoy path specified and returns success if the path
// already exists:
void
mkdir(const std::string & dir);

void
move(const std::string & from,const std::string & to);

// Recursive delete of a directory. More reliable on Windows than using fgRemoveAll
// (ie. boost::filesystem::remove_all) for which the filesystem doesn't seem to be
// ready for re-creation of the dir immediately after:
void
rm_r(const std::string & dir);

}   // namespace

#endif

// */
