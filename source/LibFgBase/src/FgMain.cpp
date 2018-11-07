//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
//

#include "stdafx.h"
#include "FgMain.hpp"
#include "FgDiagnostics.hpp"
#include "FgOut.hpp"
#include "FgStdString.hpp"
#include "FgSyntax.hpp"
#include "FgFileSystem.hpp"
#include "FgTime.hpp"

using namespace std;

static
FgArgs
fgArgs(int argc,const char * argv[])
{
    FgArgs      args;
    if (argc > 0) {     // The first arg is the command path but we only want the command name:
        FgPath      path(argv[0]);
        args.push_back(path.baseExt().m_str);
    }
    for (int ii=1; ii<argc; ++ii)
        args.push_back(string(argv[ii]));
    return args;
}

static
FgArgs
fgArgs(int argc,const wchar_t * argv[])
{
    FgArgs          args;
    if (argc > 0) {     // The first arg is the command path but we only want the command name:
        FgString        tmp(argv[0]);
        FgPath          path(tmp);
        FgString        cmdName = path.baseExt();
        args.push_back(cmdName.m_str);
    }
    for (int ii=1; ii<argc; ++ii) {
        FgString        tmp(argv[ii]);
        args.push_back(tmp.m_str);
    }
    return args;
}

// Final EOL required to due fgout idiom of fgnl at beginning of line:
#ifdef _WIN32
// Windows cmd.exe automatically adds an EOL so we only need one more:
#define NEWLINE fgout << "\n"
#else
#define NEWLINE fgout << "\n\n"
#endif

static FgArgs s_mainArgs;

int
fgMainConsole(FgCmdFunc func,int argc,const char * argv[])
{
    try
    {
        s_mainArgs = fgArgs(argc,argv);
        func(s_mainArgs);
        NEWLINE;
        return 0;
    }
    catch(FgExceptionCommandSyntax const &)
    {
        // Don't use std::cout directly since errors need to be logged if logging is on:
        fgout.setDefOut(true);
        fgout << "RETURNS:"
             << "\n     0 -- Successful completion"
             << "\n    -1 -- FaceGen exception"
             << "\n    -2 -- Standard library exception"
             << "\n    -3 -- Unknown exception"
             << "\n    -4 -- This message";
        NEWLINE;
        return -4;
    }
    catch(FgException const & e)
    {
        fgout.setDefOut(true);
        fgout << "\nERROR (FG exception): " << e.no_tr_message();
        NEWLINE;
        return -1;
    }
    catch(std::bad_alloc const &)
    {
        fgout.setDefOut(true);
        fgout << fgnl << "ERROR (std::bad_alloc): OUT OF MEMORY";
#ifndef FG_64
        fgout << fgnl << "Try running a 64-bit binary instead of this 32-bit binary";
#endif
        NEWLINE;
        return -2;
    }
    catch(std::exception const & e)
    {
        fgout.setDefOut(true);
        fgout << "\nERROR (std::exception): " << e.what();
        NEWLINE;
        return -2;
    }
    catch(...)
    {
        fgout.setDefOut(true);
        fgout << "\nERROR (unknown type):";
        NEWLINE;
        return -3;
    }
}

int
fgMainConsole(FgCmdFunc func,int argc,const wchar_t * argv[])
{
    try
    {
        s_mainArgs = fgArgs(argc,argv);
        func(s_mainArgs);
        NEWLINE;
        return 0;
    }
    catch(FgExceptionCommandSyntax const &)
    {
        // Don't use std::cout directly since errors need to be logged if logging is on:
        fgout.setDefOut(true);
        fgout << "RETURNS:"
             << "\n     0 -- Successful completion"
             << "\n    -1 -- FaceGen exception"
             << "\n    -2 -- Standard library exception"
             << "\n    -3 -- Unknown exception"
             << "\n    -4 -- This message";
        NEWLINE;
        return -4;
    }
    catch(FgException const & e)
    {
        fgout.setDefOut(true);
        fgout << "\nERROR (FG exception): " << e.no_tr_message();
        NEWLINE;
        return -1;
    }
    catch(std::bad_alloc const &)
    {
        fgout.setDefOut(true);
        fgout << fgnl << "ERROR (std::bad_alloc): OUT OF MEMORY";
#ifndef FG_64
        fgout << fgnl << "Try running a 64-bit binary instead of this 32-bit binary";
#endif
        NEWLINE;
        return -2;
    }
    catch(std::exception const & e)
    {
        fgout.setDefOut(true);
        fgout << "\nERROR (std::exception): " << e.what();
        NEWLINE;
        return -2;
    }
    catch(...)
    {
        fgout.setDefOut(true);
        fgout << "\nERROR (unknown type):";
        NEWLINE;
        return -3;
    }
}

string
fgMainArgs()
{
    string      ret;
    for (size_t aa=0; aa<s_mainArgs.size(); ++aa) {
        const string &      arg = s_mainArgs[aa];
        if (fgContains(arg,' ') || fgContains(arg,'"')) {
            ret += '"';
            for (size_t cc=0; cc<arg.size(); ++cc) {
                if (cc == '"')
                    ret += "\\\"";
                else
                    ret += arg[cc];
            }
            ret += "\" ";
        }
        else
            ret += arg + " ";
    }
    return ret;
}
