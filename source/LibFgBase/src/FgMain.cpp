//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//


#include "stdafx.h"
#include "FgMain.hpp"
#include "FgDiagnostics.hpp"
#include "FgOut.hpp"
#include "FgStdString.hpp"
#include "FgSyntax.hpp"
#include "FgFileSystem.hpp"
#include "FgTime.hpp"
#include "FgGuiApiDialogs.hpp"

using namespace std;

namespace Fg {

static bool     consoleProgram  = false;

bool            isConsoleProgram() noexcept {return consoleProgram; }

#ifdef _WIN32

#define NEWLINE fgout << "\n"

#else

#define NEWLINE fgout << "\n\n"

#endif

static CLArgs s_mainArgs;

int
mainConsole(CmdFunc func,int argc,NativeUtfChar const * argv[])
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
            Ustring         tmp(argv[0]);
            Path            path(tmp);
            Ustring         cmdName = path.baseExt();
            args.push_back(cmdName.m_str);
        }
        for (int ii=1; ii<argc; ++ii) {
            Ustring         tmp(argv[ii]);
            if ((ii == 1) && (tmp == "-guiErr"))
                guiErr = true;
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
        errStr += "\nERROR (FG exception): " + e.no_tr_message();
        retval = -1;
    }
    catch(std::bad_alloc const &)
    {
        errStr += "\nERROR (std::bad_alloc): OUT OF MEMORY";
#ifndef FG_64
        errStr += "\nTry running a 64-bit binary instead of this 32-bit binary";
#endif
        retval = -2;
    }
    catch(std::exception const & e)
    {
        errStr += "\nERROR (std::exception): " + string(e.what());
        retval = -2;
    }
    catch(...)
    {
        errStr += "\nERROR (unknown type):";
        retval = -3;
    }
    if (!errStr.empty()) {
        // Don't use std::cout directly since errors need to be logged if logging is on:
        fgout.setDefOut(true);
        fgout << errStr;
#ifdef _WIN32
        if (guiErr)
            guiDialogMessage(s_mainArgs[0]+" return error code "+toStr(retval),errStr);
#else
        guiErr;     // Avoid unused variable warning
#endif
    }
    NEWLINE;
    return retval;
}

string
mainArgs()
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
