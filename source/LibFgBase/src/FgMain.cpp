//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//


#include "stdafx.h"

#include "FgMain.hpp"
#include "FgCommand.hpp"
#include "FgFileSystem.hpp"
#include "FgTime.hpp"
#include "FgGuiApi.hpp"

using namespace std;

namespace Fg {

static bool         consoleProgram  = false;

bool                isConsoleProgram() noexcept {return consoleProgram; }

bool                isAutomated(CLArgs const & args)
{
    if ((args.size() > 1) && (args[1] == "all"))
        return true;
    return false;
}

#ifdef _WIN32

#define NEWLINE fgout << "\n"

#else

#define NEWLINE fgout << "\n\n"

#endif

static CLArgs       s_mainArgs;

int                 mainConsole(CmdFunc func,int argc,NativeUtfChar const * argv[])
{
    consoleProgram = true;
    int         retval = 0;
    string      errStr;
    // Display caught errors in GUI dialog; useful when spawned without visible console by eg. Mercurial diffs:
    bool        guiErr = false;
    try
    {
        CLArgs          args;
        if (argc > 0) {     // The first arg is the command path but we only want the command name:
            String8         tmp(argv[0]);
            Path            path(tmp);
            String8         cmdName = path.baseExt();
            args.push_back(cmdName.m_str);
        }
        for (int ii=1; ii<argc; ++ii) {
            String8         tmp(argv[ii]);
            if ((ii == 1) && (tmp == "-guiErr")) {
                if (isGuiSupported())
                    guiErr = true;
                else
                    fgout << "WARNING: 'guiErr' selected but GUI not supported on this platform";
            }
            else
                args.push_back(tmp.m_str);
        }
        s_mainArgs = args;
        func(args);
    }
    catch(FgExceptionCommandSyntax const &)
    {
        errStr += "RETURNS:"
             "\n     0 -- Successful completion"
             "\n    -1 -- FaceGen exception"
             "\n    -2 -- Standard library exception"
             "\n    -3 -- Unknown exception"
             "\n    -4 -- This message";
        retval = -4;
    }
    catch(FgException const & e)
    {
        errStr += "\nERROR (FgException)\n" + e.englishMessage();
        retval = -1;
    }
    catch(std::bad_alloc const &)
    {
        errStr += "\nERROR (std::bad_alloc) OUT OF MEMORY";
#ifndef FG_64
        errStr += "\nTry running a 64-bit binary instead of this 32-bit binary";
#endif
        retval = -2;
    }
    catch(std::exception const & e)
    {
        errStr += "\nERROR (std::exception) " + string(e.what());
        retval = -2;
    }
    catch(...)
    {
        errStr += "\nERROR (unknown type)";
        retval = -3;
    }
    if (!errStr.empty()) {
        // Don't use std::cout directly since errors need to be logged if logging is on:
        fgout.setDefOut(true);
        fgout << errStr;
        if (guiErr)
            guiDialogMessage(s_mainArgs[0]+" return error code "+toStr(retval),errStr);
    }
    NEWLINE;
    return retval;
}

string              mainArgs()
{
    string      ret;
    for (size_t aa=0; aa<s_mainArgs.size(); ++aa) {
        string const &      arg = s_mainArgs[aa];
        if (contains(arg,' ') || contains(arg,'"')) {
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

}
