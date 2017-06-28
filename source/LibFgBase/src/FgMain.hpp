//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
//

#ifndef FGMAIN_HPP
#define FGMAIN_HPP

#include "FgStdLibs.hpp"
#include "FgBoostLibs.hpp"
#include "FgStdVector.hpp"
#include "FgStdString.hpp"

// Can contain UTF-8 strings (due to legacy):
typedef vector<string> FgArgs;

typedef boost::function<void(const FgArgs &)> FgCmdFunc;

// Catches exceptions, outputs error details, returns appropriate value:
int
fgMainConsole(FgCmdFunc func,int argc,const char * argv[]);

// UTF-16 version for Windows:
int
fgMainConsole(FgCmdFunc func,int argc,const wchar_t * argv[]);

// Return the original command line. Useful for spawning current function on other computers.
// NB. Since main() only receives the command line as parsed by the shell, double quotes are added
// around any arguments containing spaces, and double quotes are escaped. This may not work properly
// on *nix:
string
fgMainArgs();

#endif
