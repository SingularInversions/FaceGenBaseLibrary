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

string              toFilePosString(char const * path_cstr,int line)
{
    // don't use Fg::Path here since we cannot throw within this function:
    string          path = path_cstr;
    size_t          pos = 0;
    for (size_t ii=0; ii<path.size(); ++ii) {
        char            ch = path[ii];
        if ((ch == '\\') || (ch == '/'))        // win or nix
            pos = ii+1;
    }
    string          fname;
    if (pos < path.size())
        fname = path.substr(pos);               // to end of string
    return fname + ":" + toStr(line);
}

void                fgAssert(char const * fname,int line,string const &  msg)
{
    fgThrow("Internal program error",msg+"\n"+toFilePosString(fname,line));
}

void                fgWarn(const std::string & msg,const std::string & dataUtf8)
{
    fgout << "\nWARNING: " << msg << ": " << dataUtf8;
}

void                fgWarn(char const * fname,int line,string const & msg)
{
    fgout << "\nWARNING: " <<  msg << " (" << toFilePosString(fname,line) << ")";
}

}
