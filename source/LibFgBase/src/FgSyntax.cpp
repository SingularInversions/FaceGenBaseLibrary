//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"
#include "FgStdVector.hpp"
#include "FgStdString.hpp"
#include "FgSyntax.hpp"
#include "FgOut.hpp"
#include "FgFileSystem.hpp"
#include "FgParse.hpp"
#include "FgConio.hpp"

using namespace std;

namespace Fg {

Syntax::Syntax(
    CLArgs const &      args,
    string const & syntax)
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
    if ((unused > 0) && (!std::uncaught_exception()))
        fgout << fgnl << "WARNING: last " << unused
            << " argument(s) not used : " << cat(cutTail(m_args,unused)," ");
}

void
Syntax::error(string const & errMsg)
{
    fgout.setDefOut(true);    // Don't write directly to cout to ensure proper logging:
    fgout << "\nERROR: " << errMsg << '\n';
    throwSyntax();
}

void
Syntax::error(string const & errMsg,Ustring const & data)
{
    fgout.setDefOut(true);
    fgout << "\nERROR: " << errMsg << ": " << data << '\n';
    throwSyntax();
}

void
Syntax::incorrectNumArgs()
{
    error("Incorrect number of arguments");
}

void
Syntax::checkExtension(Ustring const & fname,string const & ext)
{
    if (!checkExt(fname,ext))
        error("File must have extension "+ext,fname);
}

void
Syntax::checkExtension(
    string const &                 fname,
    const std::vector<string> &    exts)
{
    string      fext = toLower(pathToExt(fname));
    for (size_t ii=0; ii<exts.size(); ++ii)
        if (fext == toLower(exts[ii]))
            return;
    error("Filename did not have on of the required extensions",fname + " : " + toStr(exts));
}

string const &
Syntax::peekNext()
{
    if (m_idx+1 == m_args.size())
        error("Expected another argument after",m_args[m_idx]);
    return m_args[m_idx+1];
}

CLArgs
Syntax::rest()
{
    CLArgs  ret(m_args.begin()+m_idx,m_args.end());
    m_idx = m_args.size()-1;
    return ret;
}

static
void
removeEndline(string & line)
{
    while (!line.empty() && fgIsCrLf(line.back()))
        line.resize(line.size()-1);
}
static
bool
hasLeadingSpaces(string const & line,size_t num)
{
    for (size_t ii=0; ii<num; ++ii)
        if (line[ii] != ' ')
            return false;
    return true;
}
static
string
formatLine(string const & line,size_t indent,size_t maxWidth)
{
    --maxWidth;         // CRLF takes up a space
    string      ret;
    size_t      done = 0;
    while (done < line.size()) {
        size_t          remaining = line.size() - done;
        size_t          nextSz;
        if (done > 0) {
            ret += string(indent,' ');
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
static
string
formatLines(string const & desc)
{
    // Convert from old-style manual formatting to long lines:
    Strings     ins = splitLines(desc),
                outs;
    size_t      indent = 0;
    for (string const & line : ins) {
        size_t      mark = line.find(" - ",0);
        if (mark == string::npos) {
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
    string              ret;
    uint                maxWidth = std::min(fgConsoleWidth(),160U);     // 160 upper limit for readability
    for (string const & line : outs) {
        size_t      mark = line.find(" - ",0);
        if (mark == string::npos)
            ret += formatLine(line,0,maxWidth);
        else
            ret += formatLine(line,mark+3,maxWidth);
    }
    return ret;
}

void
Syntax::throwSyntax()
{
    m_idx = m_args.size()-1;    // Don't print warning for unused args in this case
    std::cout << "\n" << formatLines(m_syntax);
    throw FgExceptionCommandSyntax();
}

void
Syntax::numArgsMustBe(uint num)
{
    if (num+1 != m_args.size())
        incorrectNumArgs();
}

uint
Syntax::nextSelectionIndex(Strings const & validValues,string const & argDescription)
{
    size_t      idx = findFirstIdx(validValues,next());
    if (idx == validValues.size())
        error("Invalid value for",argDescription);
    return uint(idx);
}

}
