//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgCl.hpp"
#include "FgScopeGuard.hpp"
#include "FgMain.hpp"

using namespace std;

namespace Fg {

#ifndef FG_SANDBOX

bool
clRun(string const & cmd,bool throwIfError,int rvalMask)
{
    fgout << fgnl << cmd << "\n";   // DOS output lines will always start in zero'th column anyway
    int             retval = 0;
#ifdef _WIN32
    // Windows calls 'cmd /c' for system(), which usually works UNLESS there are both spaces in the
    // command path AND spaces to options, in which case it does something magically stupid and removes
    // the quotes around the path. Solution is another set of quotes:
    string          wcmd = '"' + cmd + '"';
    retval = system(wcmd.c_str());
#else
    retval = system(cmd.c_str());
#endif
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

Opt<String>
clPopen(const String & cmd)
{
    size_t constexpr    sz = 1024;
    char                buff[sz] {};
    char const *        cp = cmd.c_str();
    char const *        mp = "rb";
    FILE *              fp {};
#ifdef _WIN32
    fp = _popen(cp,mp);
#else
    fp = popen(cp,mp);
#endif
    if (fp == nullptr)
        return Opt<String>{};
#ifdef _WIN32
    ScopeGuard          sg {[fp](){_pclose(fp);} };
#else
    ScopeGuard          sg {[fp](){pclose(fp);} };
#endif
    String              ret;
    while (fgets(buff,sz,fp) != nullptr)        // fgets always adds a NULL to the end of the copied data
        ret += String(buff);
    return Opt<String>{ret};
}

void        testPopen(CLArgs const &)
{
    Opt<String>     out = clPopen("dir");
    if (out.valid())
        fgout << fgnl << "Command 'dir' output " << out.val().size() << " characters:\n" << out.val();
    else
        fgout << fgnl << "Command 'dir' failed";
}

#ifdef _WIN32

void
clUnzip(string const & fname)
{
    clRun("\"C:\\Program Files\\7-Zip\\7z.exe\" x "+fname+" >> log.txt");
}

void
clZip(String const & dirName,String const & zipFileName)
{
    clRun("\"C:\\Program Files\\7-Zip\\7z.exe\" a " + zipFileName + " " + dirName + " >> log.txt");
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

#endif

}
