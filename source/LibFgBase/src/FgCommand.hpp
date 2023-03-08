//
// Copyright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Structures for building nested commands within a single CLI executable

#ifndef FG_COMMAND_HPP
#define FG_COMMAND_HPP

#include "FgFileSystem.hpp"
#include "FgMain.hpp"

namespace Fg {

struct      Cmd
{
    CmdFunc         func;
    String          name;
    String          description;

    Cmd() {}

    Cmd(CmdFunc f,char const * n) : func(f), name(n) {}
    Cmd(CmdFunc f,char const * n,char const * d) : func(f), name(n), description(d) {}

    bool            operator<(Cmd const & rhs) const {return (name < rhs.name); }
};
typedef Svec<Cmd> Cmds;

void                doMenu(
    CLArgs              args,
    Cmds const &        cmds,
    bool                optionAll=false,    // Give option to run all sub-commands in sequence
    bool                optionQuiet=false,  // Give option to silence console output
    String const &      notes=String{});    // Printed out below the list of commands under the title NOTES:

// Creates a temporary directory with a name giving the CL args and date/time on construction,
// and removes the directory on destruction (unless the 'keep temp files' option has been selected):
struct      TestDir
{
    PushDir         pd;
    Path            path;

    TestDir() {}
    TestDir(String const & name);
    ~TestDir();
};

// Creates a test directory with a name formed from the breadcrumb of commands, changes the
// current directory to that, then reverts to the initial directory when it goes out of scope:
#define FGTESTDIR FGASSERT(!args.empty()); TestDir fgTestDir(toLower(args[0]));

// Set the root test directory. Useful for sandboxed platforms:
void            setRootTestDir(String8 const & dir);
// Make a copy of a data file in current directory. Throws if file already exists.
void            copyFileToCurrentDir(String const & nameRelativeToDataDir);
// fgout the desired command, parse 'argStr' into an CLArgs, and run with indent:
void            runCmd(CmdFunc const & func,String const & argStr);

}

#endif
