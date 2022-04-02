//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Make command-line syntax handling easier

#ifndef FGSYNTAX_HPP
#define FGSYNTAX_HPP

#include "FgMain.hpp"
#include "FgString.hpp"

namespace Fg {

struct      FgExceptionCommandSyntax {};

struct      Syntax
{
    Syntax(
        CLArgs const &  args,   // Accepts UTF-8 here
        // Wraparound for console width is aplied to this text. Any occurence of " - "
        // sets the indent level for wraparound:
        String const &  syntax);

    ~Syntax();

    String const &      next()
    {
        if (m_idx+1 == m_args.size())
            error("Expected another argument after",m_args[m_idx]);
        return m_args[++m_idx];
    }
    template<typename T>
    T                   nextAs()
    {
        Opt<T>    ret = fromStr<T>(next());
        if (!ret.valid())
            error("Unable to convert string to "+String(typeid(T).name()),curr());
        return ret.val();
    }
    String8             nextLower()             // As above but lower case
    {
        return toLower(String8(next()));
    }
    String const &      curr() const
    {
        return m_args[m_idx];
    }
    bool                more() const
    {
        return (m_idx+1 < m_args.size());
    }
    String const &      peekNext();
    CLArgs              rest();             // Starting with current
    void                error() {throwSyntax(); }
    void                error(String const & errMsg);
    void                error(String const & errMsg,String8 const & data);
    void                incorrectNumArgs();
    // Throws appropriate syntax error:
    void                checkExtension(String8 const & fname,String const & ext);
    void                checkExtension(String const & fname,Strings const & exts);
    // Throws appropriate syntax error if different:
    void                numArgsMustBe(uint numArgsNotIncludingCommand);
    // Retuns the index number of the user-specified argument in 'validValues', or throws a syntax error
    // referencing 'argDescription':
    uint                nextSelectionIndex(Strings const & validValues,String const & argDescription);
    void                noMoreArgsExpected();       // Throws is the user has supplied more arguments

private:
    String              m_syntax;
    Strings             m_args;      // NB: can contain UTF-8, stored as std::string due to legacy
    size_t              m_idx;

    void                throwSyntax();
};

// Return a string of the form "( e0 | e1 | ... | )":
String              clOptionsStr(Strings const & options);

}

#endif

// */
