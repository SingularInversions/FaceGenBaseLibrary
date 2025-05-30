//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Command-line invocations (currently only defined in LibFgWin)

#ifndef FGCL_HPP
#define FGCL_HPP

#include "FgSerial.hpp"

#ifndef FG_SANDBOX

namespace Fg {

// Call system(). Returns true if success, false if failed and throwIfError=false. 'cmd' must use native separators:
bool            clRun(const String & cmd,bool throwIfError=true,int rvalMask=0xFFFF);
// Call popen() to run a command and pipe the output into the returned string. Throws if 'popen()' fails:
String          clPopen(const String & cmd);
void            clUnzip(const String & fname);
// Zip a directory into a file, which must end in '.7z':
void            clZip(String const & dirName,String const & zipFileName);

}

#endif

#endif

// */
