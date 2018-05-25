//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Date: Nov 27, 2010
//
// Make command-line syntax handling easier

#ifndef FGSYNTAX_HPP
#define FGSYNTAX_HPP

#include "FgStdLibs.hpp"
#include "FgStdString.hpp"
#include "FgMain.hpp"

struct  FgExceptionCommandSyntax
{};

struct  FgSyntax
{
    FgSyntax(
        const FgArgs &  args,   // Accepts UTF-8 here
        const string &  syntax);

    ~FgSyntax();

    const string &
    next()
    {
        if (m_idx+1 == m_args.size())
            error("Expected another argument after",m_args[m_idx]);
        return m_args[++m_idx];
    }

    template<typename T>
    T
    nextAs()
    {
        FgOpt<T>    ret = fgFromStr<T>(next());
        if (!ret.valid())
            error("Unable to covnert string to "+string(typeid(T).name()),curr());
        return ret.val();
    }

    FgString
    nextLower()             // As above but lower case
    {return fgToLower(FgString(next())); }

    const string &
    curr() const
    {return m_args[m_idx]; }

    bool
    more() const
    {return (m_idx+1 < m_args.size()); }

    const string &
    peekNext();

    FgArgs
    rest();             // Starting with current

    void
    error()
    {throwSyntax(); }

    void
    error(const string & errMsg);

    void
    error(const string & errMsg,const FgString & data);

    void
    incorrectNumArgs();

    // Throws appropriate syntax error:
    void
    checkExtension(
        const FgString & fname,
        const string & ext);
    void
    checkExtension(
        const string &                 fname,
        const std::vector<string> &    exts);

    // Throws appropriate syntax error if different:
    void
    numArgsMustBe(uint numArgsNotIncludingCommand);

    // Retuns the index number of the user-specified argument in 'validValues', or throws a syntax error
    // referencing 'argDescription':
    uint
    nextSelectionIndex(const FgStrs & validValues,const string & argDescription);


private:
    string                 m_syntax;
    std::vector<string>    m_args;      // NB: can contain UTF-8, stored as std::string due to legacy
    size_t                 m_idx;

    void
    throwSyntax();
};

#endif

// */
