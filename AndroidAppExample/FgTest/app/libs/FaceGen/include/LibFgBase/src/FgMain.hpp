//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
//

#ifndef FGMAIN_HPP
#define FGMAIN_HPP

// Use minimal includes for compile speed since this file is included in executable projects:
#include <string>
#include <vector>
#include <functional>

// Can contain UTF-8 strings (due to legacy):
typedef std::vector<std::string>    FgArgs;

typedef std::function<void(const FgArgs &)> FgCmdFunc;

#ifdef _WIN32
typedef wchar_t FgUtfChar;
#else
typedef char FgUtfChar;
#endif

int
fgMainConsole(FgCmdFunc func,int argc,const FgUtfChar * argv[]);

// Return the original command line. Useful for spawning current function on other computers.
// NB. Since main() only receives the command line as parsed by the shell, double quotes are added
// around any arguments containing spaces, and double quotes are escaped. This may not work properly
// on *nix:
std::string
fgMainArgs();

#ifdef _WIN32
#define FGMAIN(F) int wmain(int argc,const wchar_t *argv[]) {return fgMainConsole(F,argc,argv); }
#else
#define FGMAIN(F) int main(int argc,const char *argv[]) {return fgMainConsole(F,argc,argv); }
#endif

#endif
