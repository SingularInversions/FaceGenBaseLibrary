//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
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


// fgout the desired command, parse 'argStr' into an CLArgs, and run with indent:
void            runCmd(CmdFunc const & func,String const & argStr);

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
        if (!ret.has_value())
            error("Unable to convert string to "+String(typeid(T).name()),curr());
        return ret.value();
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
    CLArgs              rest();             // starting with *current*
    CLArgs              nextRest();         // starting with *next*
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
    size_t              m_idx;      // to current argument being processed (initially the current command selection)

    void                throwSyntax();
};

// Return a string of the form "( e0 | e1 | ... | )":
String              clOptionsStr(Strings const & options);

}

#endif
