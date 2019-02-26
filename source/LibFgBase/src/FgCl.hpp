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
#include "FgPlatform.hpp"

#ifndef FG_SANDBOX

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

}   // namespace

#endif

#endif

// */
