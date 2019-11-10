//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Make command-line syntax handling easier

#ifndef FGSYNTAX_HPP
#define FGSYNTAX_HPP

#include "FgStdLibs.hpp"
#include "FgStdString.hpp"
#include "FgMain.hpp"
#include "FgString.hpp"

namespace Fg {

struct  FgExceptionCommandSyntax
{};

struct  Syntax
{
    Syntax(
        const CLArgs &  args,   // Accepts UTF-8 here
        // Wraparound for console width is aplied to this text. Any occurence of " - "
        // sets the indent level for wraparound:
        const String &  syntax);

    ~Syntax();

    const String &
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
        Opt<T>    ret = fgFromStr<T>(next());
        if (!ret.valid())
            error("Unable to convert string to "+String(typeid(T).name()),curr());
        return ret.val();
    }

    Ustring
    nextLower()             // As above but lower case
    {return fgToLower(Ustring(next())); }

    const String &
    curr() const
    {return m_args[m_idx]; }

    bool
    more() const
    {return (m_idx+1 < m_args.size()); }

    const String &
    peekNext();

    CLArgs
    rest();             // Starting with current

    void
    error()
    {throwSyntax(); }

    void
    error(const String & errMsg);

    void
    error(const String & errMsg,const Ustring & data);

    void
    incorrectNumArgs();

    // Throws appropriate syntax error:
    void
    checkExtension(
        const Ustring & fname,
        const String & ext);
    void
    checkExtension(
        const String &          fname,
        const Strings &         exts);

    // Throws appropriate syntax error if different:
    void
    numArgsMustBe(uint numArgsNotIncludingCommand);

    // Retuns the index number of the user-specified argument in 'validValues', or throws a syntax error
    // referencing 'argDescription':
    uint
    nextSelectionIndex(const Strings & validValues,const String & argDescription);


private:
    String              m_syntax;
    Strings             m_args;      // NB: can contain UTF-8, stored as std::string due to legacy
    size_t              m_idx;

    void
    throwSyntax();
};

}

#endif

// */
