//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgCl.hpp"
#include "FgStdString.hpp"
#include "FgException.hpp"
#include "FgString.hpp"
#include "FgOut.hpp"

using namespace std;

#ifndef FG_SANDBOX

namespace Fg {

bool
clRun(string const & cmd,bool throwIfError,int rvalMask)
{
    fgout << fgnl << cmd << "\n";   // DOS output lines will always start in zero'th column anyway
    int         retval = system(cmd.c_str());
    // Some commands such as robocopy have several non-error return codes.
    if ((retval & rvalMask) != 0)
    {
        if (throwIfError)
            fgThrow("Error exit code from command",toStr(retval));
        else
            fgout << fgnl << "Error exit code from command: " << retval;
        return false;
    }
    return true;
}

#ifdef _WIN32

void
clUnzip(string const & fname)
{
    clRun("\"C:\\Program Files\\7-Zip\\7z.exe\" x "+fname+" >> log.txt");
}

void
clZip(string const & dir,bool oldFormat)
{
    string      ext = (oldFormat ? ".zip " : ".7z ");
    clRun("\"C:\\Program Files\\7-Zip\\7z.exe\" a "+dir+ext+dir+" >> log.txt");
}

#else

void
unzip(string const &)
{
    throw FgExceptionNotImplemented();
}

void
zip(string const & ,string const & )
{
    throw FgExceptionNotImplemented();
}

#endif

}

#endif
