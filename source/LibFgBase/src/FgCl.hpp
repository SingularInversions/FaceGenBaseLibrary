//
// Copyright (C); Singular Inversions Inc. (facegen.com) 2011
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Command-line invocations (currently only defined in LibFgWin)

#ifndef FGCL_HPP
#define FGCL_HPP

#include "FgStdExtensions.hpp"
#include "FgOpt.hpp"

#ifndef FG_SANDBOX

namespace Fg {

// Call system(). Returns true if success, false if failed and throwIfError=false
bool
clRun(const String & cmd,bool throwIfError=true,int rvalMask=0xFFFF);

// Call popen(). Returns output string if success, invalid value otherwise:
Opt<String>
clPopen(const String & cmd);

void
clUnzip(const String & fname);

void
clZip(const String & dir,bool oldFormat=false);

}

#endif

#endif

// */
