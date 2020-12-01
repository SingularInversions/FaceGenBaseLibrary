//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//


#include "stdafx.h"

#include "FgStdString.hpp"
#include "FgStdVector.hpp"
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

static string       s_breadcrumb;
static string       s_annotateTestDir;
static bool         s_keepTempFiles = false;

static
string
cmdStr(const Cmd & cmd)
{
    string          si = "\n        " + cmd.name;
    if (!cmd.description.empty()) {
        for (size_t jj=si.size(); jj<24; ++jj)
            si += " ";
        si += "- " + cmd.description + "\n";
    }
    return si;
}

void
doMenu(
    Strings          args,
    const Cmds &   cmdsUnsorted,
    bool                    optionAll,
    bool                    optionQuiet,
    bool                    optionKeep)
{
    s_breadcrumb += toLower(pathToBase(args[0]).m_str) + "_";
    string      cl,desc;
    if (optionQuiet) {
        cl +=   "[-s] ";
        desc += "    -s              - Suppress standard output except for errors.\n";
    }
    if (optionKeep) {
        cl +=   "[-k[<desc>]] ";
        desc += "    -k[<desc>]      - Keep test files in dated test directory [suffixed with <desc>].\n";
    }
    if (optionAll) {
        cl +=   "(<command> | all)\n";
        desc += "    -a              - Automated, no interactive feedback or regression updates\n";
    }
    else
        cl += "<command>\n";
    desc += "    <command>:";
    Cmds       cmds = cmdsUnsorted;
    std::sort(cmds.begin(),cmds.end());
    for (size_t ii=0; ii<cmds.size(); ++ii)
        desc += cmdStr(cmds[ii]);
    Syntax    syntax(args,cl+desc);
    while (syntax.peekNext()[0] == '-') {
        string  opt = syntax.next();
        if ((opt == "-s") && optionQuiet)
            fgout.setDefOut(false);
        else if ((opt.substr(0,2) == "-k") && optionKeep) {
            s_keepTempFiles = true;
            if (opt.size() > 2)
                s_annotateTestDir = opt.substr(2);
        }
        else
            syntax.error("Invalid option");
    }
    string      cmd = syntax.next(),
                cmdl = toLower(cmd);
    if (optionAll) {
        if (cmdl == "all") {
            fgout << fgnl << "Testing: " << fgpush;
            for (size_t ii=0; ii<cmds.size(); ++ii) {
                fgout << fgnl << cmds[ii].name << ": " << fgpush;
                cmds[ii].func(svec(cmds[ii].name,cmdl));      // Pass on the 'all'
                fgout << fgpop;
            }
            fgout << fgpop << fgnl << "All Passed.";
            return;
        }
    }
    for (size_t ii=0; ii<cmds.size(); ++ii) {
        if (cmdl == toLower(cmds[ii].name)) {
            cmds[ii].func(syntax.rest());
            if (optionAll && (cmdl == "all"))
                fgout << fgpop << fgnl << "Passed.";
            return;
        }
    }
    if (optionAll && (cmdl == "all"))
        fgout << fgpop;
    syntax.error("Invalid command",cmd);
}

static Ustring s_rootTestDir;

TestDir::TestDir(string const & name)
{
    FGASSERT(!name.empty());
    if (s_rootTestDir.empty()) {
        path = Path(dataDir());
        path.dirs.back() = "_log";      // replace 'data' with 'log'
    }
    else
        path = s_rootTestDir;
    path.dirs.push_back(s_breadcrumb+name);
    string          dt = getDateTimeFilename();
    if (s_annotateTestDir.size() > 0)
        dt += " " + s_annotateTestDir;
    path.dirs.push_back(dt);
    createPath(path.dir());
    pd.push(path.str());
    if (s_keepTempFiles)
        fgout.logFile("_log.txt");
}

TestDir::~TestDir()
{
    if (!pd.orig.empty()) {
        pd.pop();
        if (!s_keepTempFiles) {
            // Recursively delete the directory for this test:
            deleteDirectoryRecursive(path.str());
            // Remove the test name directory if empty:
            path.dirs.resize(path.dirs.size()-1);
            removeDirectory(path.str());
        }
    }
}

void
fgSetRootTestDir(Ustring const & dir)
{ s_rootTestDir = dir; }

void
fgTestCopy(string const & relPath)
{
    Ustring        name = pathToName(relPath);
    if (pathExists(name))
        fgThrow("Test copy filename collision",name);
    Ustring        source = dataDir() + relPath;
    fileCopy(source,name);
}

void
runCmd(const CmdFunc & func,string const & argStr)
{
    fgout << fgnl << argStr << " " << fgpush;
    func(splitChar(argStr));
    fgout << fgpop;
}

bool
fgKeepTempFiles()
{return s_keepTempFiles; }

bool
fgAutomatedTest(CLArgs const & args)
{
    return ((args.size() == 2) && (args[1] == "all"));
}

}
