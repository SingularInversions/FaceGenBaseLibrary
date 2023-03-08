//
// Copyright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//


#include "stdafx.h"

#include "FgSerial.hpp"
#include "FgCommand.hpp"
#include "FgFileSystem.hpp"
#include "FgTime.hpp"
#include "FgSyntax.hpp"
#include "FgBuild.hpp"
#include "FgTestUtils.hpp"
#include "FgParse.hpp"
#include "FgConio.hpp"

using namespace std;

namespace Fg {

static Strings      s_breadcrumbs;

static string       cmdStr(Cmd const & cmd)
{
    string          si = "\n        " + cmd.name;
    if (!cmd.description.empty()) {
        for (size_t jj=si.size(); jj<24; ++jj)
            si += " ";
        si += "- " + cmd.description + "\n";
    }
    return si;
}

void                doMenu(
    CLArgs              args,
    Cmds const &        cmdsUnsorted,
    bool                optionAll,
    bool                optionQuiet,
    String const &      notes)
{
    s_breadcrumbs.push_back(toLower(pathToBase(args[0]).m_str));
    string              cl,desc;
    if (optionQuiet) {
        cl +=   "[-s] ";
        desc += "    -s              - Suppress standard output except for errors.\n";
    }
    if (optionAll) {
        cl +=   "(<command> | all)\n";
        desc += "    -a              - Automated, no interactive feedback or regression updates\n";
    }
    else
        cl += "<command>\n";
    desc += "    <command>:";
    Cmds                cmds = cmdsUnsorted;
    sort(cmds.begin(),cmds.end());
    for (size_t ii=0; ii<cmds.size(); ++ii)
        desc += cmdStr(cmds[ii]);
    if (!notes.empty())
        desc += "\nNOTES:\n    " + notes;
    Syntax              syn {args,cl+desc};
    while (syn.peekNext()[0] == '-') {
        string              opt = syn.next();
        if ((opt == "-s") && optionQuiet)
            fgout.setDefOut(false);
        else
            syn.error("Invalid option");
    }
    string              arg = syn.next(),
                        argl = toLower(arg);
    if (optionAll && (argl == "all")) {
        if (syn.more())
            syn.error("no further arguments permitted after 'all'");
        for (Cmd const & cmd : cmds) {
            PushIndent      pind {cmd.name};
            cmd.func({cmd.name,argl});          // Pass on the 'all'
        }
    }
    else {
        for (Cmd const & cmd : cmds) {
            if (argl == toLower(cmd.name)) {
                cmd.func(syn.rest());
                return;
            }
        }
        syn.error("command not found",arg);
    }
    s_breadcrumbs.pop_back();
}

static String8 s_rootTestDir;

TestDir::TestDir(String const & name)
{
    FGASSERT(!name.empty());
    if (s_rootTestDir.empty()) {
        path = Path(dataDir());
        path.dirs.back() = "test-output";      // replace 'data'
    }
    else
        path = s_rootTestDir;
    path.dirs.push_back(cat(s_breadcrumbs,"-")+"-"+name);
    createPath(path.dir());
    pd.push(path.str());
    // remove any previous output for clean test:
    deleteDirectoryContentsRecursive(path.str());
}

TestDir::~TestDir()
{
    if (!pd.orig.empty()) {
        pd.pop();
    }
}

void                setRootTestDir(String8 const & dir) { s_rootTestDir = dir; }

void                copyFileToCurrentDir(String const & relPath)
{
    String8        name = pathToName(relPath);
    if (pathExists(name))
        fgThrow("Test copy filename collision",name);
    String8        source = dataDir() + relPath;
    fileCopy(source,name);
}

void                runCmd(CmdFunc const & func,String const & argStr)
{
    fgout << fgnl << argStr << " " << fgpush;
    func(splitChar(argStr));
    fgout << fgpop;
}

}
