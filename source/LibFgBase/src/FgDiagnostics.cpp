//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"
#include "FgDiagnostics.hpp"
#include "FgOut.hpp"
#include "FgString.hpp"

using namespace std;

namespace Fg {

string
pathToName(char const * fname)
{
    string      fs(fname);
    size_t      pos = fs.rfind('\\');
    if (pos == string::npos)
        pos = 0;
    else
        ++pos;      // Skip over character
    return fs.substr(pos);
}

string
fgDiagString(char const * fname,int line)
{return pathToName(fname) + ":" + toStr(line); }

void
fgAssert(char const * fname,int line,string const &  msg)
{
    fgThrow("Internal program error",msg+"\n"+fgDiagString(fname,line));
}

void
fgWarn(const std::string & msg,const std::string & dataUtf8)
{
    fgout << "\nWARNING: " << msg << ": " << dataUtf8;
}

void
fgWarn(char const * fname,int line,string const & msg)
{
    fgout << "\nWARNING: " <<  msg << " (" << fgDiagString(fname,line) << ")";
}

}
