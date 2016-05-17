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

FgArgs
fgArgs(int argc,const char * argv[])
{
    FgArgs  args;
    for (int ii=0; ii<argc; ++ii)
        args.push_back(string(argv[ii]));
    return args;
}

FgArgs
fgArgs(int argc,const wchar_t * argv[])
{
    FgArgs      args;
    for (int ii=0; ii<argc; ++ii) {
        FgString        tmp(argv[ii]);
        args.push_back(tmp.m_str);
    }
    return args;
}

int
fgMainConsole(FgCmdFunc func,int argc,const char * argv[])
{
// Final EOL required to due fgout idiom of fgnl at beginning of line:
#ifdef _WIN32
// Windows cmd.exe automatically adds an EOL so we only need one more:
#define FLUSH fgout << endl
#else
#define FLUSH fgout << endl << endl
#endif

    try
    {
        FgArgs  args = fgArgs(argc,argv);
        func(args);
        FLUSH;
        return 0;
    }
    catch(FgExceptionCommandSyntax const &)
    {
        // Don't use std::cout directly since errors need to be logged if logging is on:
        fgout.setCout(true);
        fgout << "RETURNS:"
             << endl << "     0 -- Successful completion"
             << endl << "    -1 -- FaceGen exception"
             << endl << "    -2 -- Standard library exception"
             << endl << "    -3 -- Unknown exception"
             << endl << "    -4 -- This message";
        FLUSH;
        return -4;
    }
    catch(FgException const & e)
    {
        fgout.setCout(true);
        fgout << endl << "ERROR (FG exception): " << e.no_tr_message();
        FLUSH;
        return -1;
    }
    catch(std::bad_alloc const &)
    {
        fgout.setCout(true);
        fgout << fgnl << "ERROR (std::bad_alloc): OUT OF MEMORY";
#ifndef FG_64
        fgout << fgnl << "Try running a 64-bit binary instead of this 32-bit binary";
#endif
        FLUSH;
        return -2;
    }
    catch(std::exception const & e)
    {
        fgout.setCout(true);
        fgout << endl << "ERROR (std::exception): " << e.what();
        FLUSH;
        return -2;
    }
    catch(...)
    {
        fgout.setCout(true);
        fgout << endl << "ERROR (unknown type):";
        FLUSH;
        return -3;
    }
}

int
fgMainConsole(FgCmdFunc func,int argc,const wchar_t * argv[])
{
    try
    {
        FgArgs  args = fgArgs(argc,argv);
        func(args);
        FLUSH;
        return 0;
    }
    catch(FgExceptionCommandSyntax const &)
    {
        // Don't use std::cout directly since errors need to be logged if logging is on:
        fgout.setCout(true);
        fgout << "RETURNS:"
             << endl << "     0 -- Successful completion"
             << endl << "    -1 -- FaceGen exception"
             << endl << "    -2 -- Standard library exception"
             << endl << "    -3 -- Unknown exception"
             << endl << "    -4 -- This message";
        FLUSH;
        return -4;
    }
    catch(FgException const & e)
    {
        fgout.setCout(true);
        fgout << endl << "ERROR (FG exception): " << e.no_tr_message();
        FLUSH;
        return -1;
    }
    catch(std::bad_alloc const &)
    {
        fgout.setCout(true);
        fgout << fgnl << "ERROR (std::bad_alloc): OUT OF MEMORY";
#ifndef FG_64
        fgout << fgnl << "Try running a 64-bit binary instead of this 32-bit binary";
#endif
        FLUSH;
        return -2;
    }
    catch(std::exception const & e)
    {
        fgout.setCout(true);
        fgout << endl << "ERROR (std::exception): " << e.what();
        FLUSH;
        return -2;
    }
    catch(...)
    {
        fgout.setCout(true);
        fgout << endl << "ERROR (unknown type):";
        FLUSH;
        return -3;
    }
}

