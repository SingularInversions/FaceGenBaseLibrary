//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//


#include "stdafx.h"

#include "FgSerial.hpp"
#include "FgCommand.hpp"
#include "FgFileSystem.hpp"
#include "FgTime.hpp"

#include "FgBuild.hpp"
#include "FgTestUtils.hpp"
#include "FgParse.hpp"
#include "FgConio.hpp"

using namespace std;

namespace Fg {

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
    static bool         runningAll = false;     // are we recursively running all ?
    if (optionAll && (argl == "all")) {
        if (syn.more())
            syn.error("no further arguments permitted after 'all'");
        if (runningAll) {
            for (Cmd const & cmd : cmds) {
                    PushIndent          pind {cmd.name};
                    cmd.func({cmd.name,argl});
            }
        }
        else {                                  // top level run all needs to output text and indent:
            PushIndent          pi0 {"Running all tests","All tests passed"};
            runningAll = true;
            for (Cmd const & cmd : cmds) {
                    PushIndent          pind {cmd.name};
                    cmd.func({cmd.name,argl});
            }
            runningAll = false;
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
}

void                runCmd(CmdFunc const & func,String const & argStr)
{
    fgout << fgnl << argStr << " " << fgpush;
    func(splitWhitespace(argStr));
    fgout << fgpop;
}

Syntax::Syntax(CLArgs const & args,String const & syntax)
    :
    m_args(args), m_idx(0)
{
    FGASSERT(!args.empty());
    m_syntax = args[0] + " " + syntax;
    if (args.size() == 1)
        throwSyntax();
    else {
        if ((args[1] == "/?") ||
            (args[1] == "help") ||
            (args[1] == "-help") ||
            (args[1] == "--help"))
            throwSyntax();
    }
}
Syntax::~Syntax()
{
    size_t      unused = m_args.size() - m_idx - 1;
    if ((unused > 0) && (uncaught_exceptions() == 0))
        fgout << fgnl << "WARNING: last " << unused
            << " argument(s) not used : " << cat(cTail(m_args,unused)," ");
}
void                Syntax::error(String const & errMsg)
{
    fgout.setDefOut(true);    // Don't write directly to cout to ensure proper logging:
    fgout << "\nERROR: " << errMsg << '\n';
    throwSyntax();
}
void                Syntax::error(String const & errMsg,String8 const & data)
{
    fgout.setDefOut(true);
    fgout << "\nERROR: " << errMsg << ": " << data << '\n';
    throwSyntax();
}
void                Syntax::incorrectNumArgs()
{
    error("Incorrect number of arguments");
}
void                Syntax::checkExtension(String8 const & fname,String const & ext)
{
    if (!checkExt(fname,ext))
        error("File must have extension "+ext,fname);
}
void                Syntax::checkExtension(String const & fname,Strings const & exts)
{
    String      fext = toLower(pathToExt(fname));
    for (size_t ii=0; ii<exts.size(); ++ii)
        if (fext == toLower(exts[ii]))
            return;
    error("Filename did not have on of the required extensions",fname + " : " + toStr(exts));
}
String const &      Syntax::peekNext()
{
    if (m_idx+1 == m_args.size())
        error("Expected another argument after",m_args[m_idx]);
    return m_args[m_idx+1];
}
CLArgs              Syntax::rest()
{
    CLArgs  ret(m_args.begin()+m_idx,m_args.end());
    m_idx = m_args.size()-1;
    return ret;
}
CLArgs              Syntax::nextRest()
{
    CLArgs              ret;
    while (m_idx+1 < m_args.size()) {
        ++m_idx;
        ret.push_back(m_args[m_idx]);
    }
    return ret;
}
static void         removeEndline(String & line)
{
    while (!line.empty() && isCrLf(line.back()))
        line.resize(line.size()-1);
}
static bool         hasLeadingSpaces(String const & line,size_t num)
{
    for (size_t ii=0; ii<num; ++ii)
        if (line[ii] != ' ')
            return false;
    return true;
}
static String       formatLine(String const & line,size_t indent,size_t maxWidth)
{
    --maxWidth;         // CRLF takes up a space
    String      ret;
    size_t      done = 0;
    while (done < line.size()) {
        size_t          remaining = line.size() - done;
        size_t          nextSz;
        if (done > 0) {
            ret += String(indent,' ');
            nextSz = std::min(remaining,maxWidth-indent);
        }
        else
            nextSz = std::min(remaining,maxWidth);
        if (nextSz < remaining) {
            size_t          nextSzWord = nextSz;
            while ((nextSzWord > 0) && (line[done+nextSzWord] != ' '))
                --nextSzWord;
            if ((nextSzWord < nextSz) && (nextSzWord > 0)) {
                ret += line.substr(done,nextSzWord);
                done += nextSzWord + 1;         // Eat the space
            }
            else {
                ret += line.substr(done,nextSz);
                done += nextSz;
            }
        }
        else {
            ret += line.substr(done,nextSz);
            done += nextSz;
        }
        ret += "\n";
    }
    return ret;
}
static size_t       findFirstMarker(String const & str)
{
    return std::min(str.find(" - ",0),str.find(" * ",0));
}
static String       formatLines(String const & desc)
{
    // Convert from old-style manual formatting to long lines:
    Strings             ins = splitLines(desc),
                        outs;
    size_t              indent = 0;
    for (String const & line : ins) {
        size_t              mark = findFirstMarker(line);
        if (mark == String::npos) {
            if ((indent > 0) && (line.size() > indent) && (hasLeadingSpaces(line,indent))) {
                // This line is a continuation:
                removeEndline(outs.back());
                outs.back() += line.substr(indent-1);
            }
            else {
                // Not a continuation or a format markup:
                indent = 0;
                outs.push_back(line);
            }
        }
        else {
            // A new format markup:
            indent = mark + 3;
            outs.push_back(line);
        }
    }
    // Now format the lines:
    String              ret;
    uint                maxWidth = std::min(fgConsoleWidth(),160U);     // 160 upper limit for readability
    for (String const & line : outs) {
        size_t              mark = findFirstMarker(line);
        if (mark == String::npos)
            ret += formatLine(line,0,maxWidth);
        else
            ret += formatLine(line,mark+3,maxWidth);
    }
    return ret;
}
void                Syntax::throwSyntax()
{
    m_idx = m_args.size()-1;    // Don't print warning for unused args in this case
    fgout << fgnl << formatLines(m_syntax);
    throw FgExceptionCommandSyntax();
}
void                Syntax::numArgsMustBe(uint num)
{
    if (num+1 != m_args.size())
        incorrectNumArgs();
}
uint                Syntax::nextSelectionIndex(Strings const & validValues,String const & argDescription)
{
    size_t      idx = findFirstIdx(validValues,next());
    if (idx == validValues.size())
        error("Invalid value for",argDescription);
    return uint(idx);
}
void                Syntax::noMoreArgsExpected()
{
    if (more())
        error("too many arguments supplied");
}

String              clOptionsStr(Strings const & options)
{
    FGASSERT(!options.empty());
    String          ret = "(";
    ret += options[0];
    for (size_t ii=1; ii<options.size(); ++ii)
        ret += " | " + options[ii];
    ret += ")";
    return ret;
}

}
