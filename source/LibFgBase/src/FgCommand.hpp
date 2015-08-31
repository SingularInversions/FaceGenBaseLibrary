//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
//

#ifndef FG_COMMAND_HPP
#define FG_COMMAND_HPP

#include "FgStdLibs.hpp"
#include "FgException.hpp"
#include "FgOut.hpp"
#include "FgString.hpp"
#include "FgMain.hpp"
#include "FgFileSystem.hpp"

struct FgCmd
{
    FgCmdFunc       func;
    std::string     name;
    std::string     description;

    FgCmd() : func(0) {}
    FgCmd(FgCmdFunc f,const char * n)
        : func(f), name(n) {}
    FgCmd(FgCmdFunc f,const char * n,const char * d)
        : func(f), name(n), description(d) {}

    bool
    operator<(const FgCmd & rhs) const
    {return (name < rhs.name); }
};

void
fgMenu(
    FgArgs                      args,
    const std::vector<FgCmd> &  cmds,
    bool                        optionAll=false,    // Give the 'all' and 'automated' options (for tests)
    bool                        optionQuiet=false,  // Give option to silence console output
    bool                        optionKeep=false);  // Give option to keep test files

struct FgTestDir
{
    FgPushDir       pd;
    FgPath          path;

    FgTestDir() {}

    FgTestDir(const std::string & name);

    ~FgTestDir();
};

// Creates a test directory with a name formed from the breadcrumb of commands, changes the
// current directory to that, then reverts to the initial directory when it goes out of scope:
#define FGTESTDIR FgTestDir fgTestDir(fgToLower(args[0]));

// Make a copy of a data file in current directory:
void
fgTestCopy(const std::string & nameRelativeToDataDir);

extern bool     fgCommandAutomated;     // No user interactivity

#endif
