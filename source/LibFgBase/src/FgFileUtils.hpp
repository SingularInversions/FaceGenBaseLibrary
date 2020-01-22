//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGFILEUTILS_HPP
#define FGFILEUTILS_HPP

#include "FgFileSystem.hpp"
#include "FgTime.hpp"

namespace Fg {

// First argument is a list of input files, second argument is a list of output files:
typedef std::function<void(Ustrings const &,Ustrings const &)>  FgUpdateFilesFunc;

// Handle workflow dependencies between files:
// Avoid double-typing inline values of 'ins' and 'outs'.
// Log output to log file starting with given log number.
// Returns true if updates were required:
bool
fgUpdateFiles(uint num,Ustrings const & ins,Ustrings const & outs,FgUpdateFilesFunc func);

}

#endif
