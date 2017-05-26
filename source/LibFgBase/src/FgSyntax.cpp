//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Date: Nov 27, 2010
//

#include "stdafx.h"
#include "FgSyntax.hpp"
#include "FgOut.hpp"
#include "FgStdString.hpp"
#include "FgFileSystem.hpp"

using namespace std;

FgSyntax::FgSyntax(
    const FgArgs &      args,
    const string & syntax)
    :
    m_args(args), m_idx(0)
{
    FGASSERT(!args.empty());
    m_syntax = args[0] + " " + syntax;
    if (args.size() == 1)
        throwSyntax();
}

FgSyntax::~FgSyntax()
{
    size_t      unused = m_args.size() - m_idx - 1;
    if ((unused > 0) && (!std::uncaught_exception()))
        fgout << fgnl << "WARNING: last " << unused
            << " argument(s) not used : " << fgCat(fgTail(m_args,unused)," ");
}

void
FgSyntax::error(const string & errMsg)
{
    fgout.setCout(true);
    fgout << endl << errMsg;
    throwSyntax();
}

void
FgSyntax::error(const string & errMsg,const FgString & data)
{
    fgout.setCout(true);
    fgout << endl << errMsg << ": " << data;
    throwSyntax();
}

void
FgSyntax::incorrectNumArgs()
{
    error("Incorrect number of arguments");
}

void
FgSyntax::checkExtension(const FgString & fname,const string & ext)
{
    if (!fgCheckExt(fname,ext))
        error("File must have extension "+ext,fname);
}

void
FgSyntax::checkExtension(
    const string &                 fname,
    const std::vector<string> &    exts)
{
    string      fext = fgToLower(fgPathToExt(fname));
    for (size_t ii=0; ii<exts.size(); ++ii)
        if (fext == fgToLower(exts[ii]))
            return;
    error("Filename did not have on of the required extensions",fname + " : " + fgToString(exts));
}

const string &
FgSyntax::peekNext()
{
    if (m_idx+1 == m_args.size())
        error("Expected another argument after",m_args[m_idx]);
    return m_args[m_idx+1];
}

FgArgs
FgSyntax::rest()
{
    FgArgs  ret(m_args.begin()+m_idx,m_args.end());
    m_idx = m_args.size()-1;
    return ret;
}

void
FgSyntax::throwSyntax()
{
    m_idx = m_args.size()-1;    // Don't print warning for unused args in this case
    std::cout << std::endl << m_syntax << std::endl;
    throw FgExceptionCommandSyntax();
}


void
FgSyntax::numArgsMustBe(uint num)
{
    if (num+1 != m_args.size())
        incorrectNumArgs();
}
