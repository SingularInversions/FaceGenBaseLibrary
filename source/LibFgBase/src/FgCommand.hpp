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

    FgCmd(FgCmdFunc func_,const char * name_)
        : func(func_), name(name_) {}

    FgCmd(FgCmdFunc func_,const char * name_,const char * description_)
        : func(func_), name(name_), description(description_) {}

    bool
    operator<(const FgCmd & rhs) const
    {return (name < rhs.name); }
};

typedef std::vector<FgCmd> FgCmds;

void
fgMenu(
    FgArgs              args,
    const FgCmds &      cmds,
    bool                optionAll=false,    // Give the 'all' and 'automated' options (for tests)
    bool                optionQuiet=false,  // Give option to silence console output
    bool                optionKeep=false);  // Give option to keep test files

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

// fgout the desired command, parse 'argStr' into an FgArgs, and run with indent:
void
fgRunCmd(const FgCmdFunc & func,const string & argStr);

// Useful for writing command dispatch commands - as long as vector<FgCmd> is named 'cmds':
#define FGADDCMD1(fn,name) void fn(const FgArgs &); cmds.push_back(FgCmd(fn,name))

#endif
