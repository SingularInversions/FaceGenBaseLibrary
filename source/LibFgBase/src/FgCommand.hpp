//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

// Structures for building nested commands within a single CLI executable

#ifndef FG_COMMAND_HPP
#define FG_COMMAND_HPP

#include "FgStdLibs.hpp"
#include "FgException.hpp"
#include "FgOut.hpp"
#include "FgString.hpp"
#include "FgMain.hpp"
#include "FgFileSystem.hpp"

namespace Fg {

struct Cmd
{
    CmdFunc         func;
    String          name;
    String          description;

    Cmd() {}

    Cmd(CmdFunc func_,const char * name_)
        : func(func_), name(name_) {}

    Cmd(CmdFunc func_,const char * name_,const char * description_)
        : func(func_), name(name_), description(description_) {}

    bool
    operator<(const Cmd & rhs) const
    {return (name < rhs.name); }
};

typedef Svec<Cmd> Cmds;

void
doMenu(
    CLArgs              args,
    const Cmds &        cmds,
    bool                optionAll=false,    // Give option to run all sub-commands in sequence
    bool                optionQuiet=false,  // Give option to silence console output
    bool                optionKeep=false);  // Give option to keep test files

// Creates a temporary directory with a name giving the CL args and date/time on construction,
// and removes the directory on destruction (unless the 'keep temp files' option has been selected):
struct TestDir
{
    PushDir         pd;
    Path            path;

    TestDir() {}

    TestDir(String const & name);

    ~TestDir();
};

// Creates a test directory with a name formed from the breadcrumb of commands, changes the
// current directory to that, then reverts to the initial directory when it goes out of scope:
#define FGTESTDIR FGASSERT(!args.empty()); TestDir fgTestDir(fgToLower(args[0]));

// Set the root test directory. Useful for sandboxed platforms:
void
fgSetRootTestDir(Ustring const & dir);

// Make a copy of a data file in current directory:
void
fgTestCopy(String const & nameRelativeToDataDir);

// fgout the desired command, parse 'argStr' into an CLArgs, and run with indent:
void
runCmd(const CmdFunc & func,String const & argStr);

// Useful for writing command dispatch commands - as long as Svec<Cmd> is named 'cmds':
#define FGADDCMD1(fn,name) void fn(CLArgs const &); cmds.push_back(Cmd(fn,name))
#define FGADDCMD(fn,name,desc) void fn(CLArgs const &); cmds.push_back(Cmd(fn,name,desc))

// Are we currently configured to keep temporary files ?
bool fgKeepTempFiles();

// Returns true if the user is doing a 'test all', so we can choose to skip non-automatable tests:
bool
fgAutomatedTest(CLArgs const &);

}

#endif
