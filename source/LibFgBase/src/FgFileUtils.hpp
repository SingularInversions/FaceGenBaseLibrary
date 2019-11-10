//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGFILEUTILS_HPP
#define FGFILEUTILS_HPP

#include "FgFileSystem.hpp"
#include "FgTime.hpp"

namespace Fg {

// First argument is a list of input files, second argument is a list of output files:
typedef std::function<void(const Ustrings &,const Ustrings &)>  FgUpdateFilesFunc;

// Handle workflow dependencies between files:
// Avoid double-typing inline values of 'ins' and 'outs'.
// Log output to log file starting with given log number.
// Returns true if updates were required:
bool
fgUpdateFiles(uint num,const Ustrings & ins,const Ustrings & outs,FgUpdateFilesFunc func);

}

#endif
