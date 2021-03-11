//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//


#ifndef FGMAIN_HPP
#define FGMAIN_HPP

// Use minimal includes for compile speed since this file is included in executable projects:
#include <string>
#include <vector>
#include <functional>

namespace Fg {

// Returns true only if program was started via 'mainConsole' below
bool isConsoleProgram() noexcept;

// An alias for strings that can contain UTF-8:
typedef std::vector<std::string>    CLArgs;

typedef std::function<void(CLArgs const &)> CmdFunc;

#ifdef _WIN32
typedef wchar_t NativeUtfChar;
#else
typedef char NativeUtfChar;
#endif

int
mainConsole(CmdFunc func,int argc,const NativeUtfChar * argv[]);

// Return the original command line. Useful for spawning current function on other computers.
// NB. Since main() only receives the command line as parsed by the shell, double quotes are added
// around any arguments containing spaces, and double quotes are escaped. This may not work properly
// on *nix:
std::string
mainArgs();

#ifdef _WIN32
#define FGMAIN(F) int wmain(int argc,const wchar_t *argv[]) {return mainConsole(F,argc,argv); }
#else
#define FGMAIN(F) int main(int argc,char const *argv[]) {return mainConsole(F,argc,argv); }
#endif

}

#endif
