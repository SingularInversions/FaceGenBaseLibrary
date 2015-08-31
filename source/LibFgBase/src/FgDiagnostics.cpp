//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     May 7, 2004
//

#include "stdafx.h"
#include "FgDiagnostics.hpp"

using namespace std;

string
fgDiagString(
    const char *    fname,
    int             line)
{
    ostringstream   retval;
    retval << fname << ": " << line;
    return retval.str();
}

string
fgDiagString(
    const char *    fname,
    int             line,
    const string &  msg)
{
    ostringstream   retval;
    retval << fname << ": " << line;
    if (msg.size() > 0)
        retval << " Info: " << msg;
    return retval.str();
}

void
fgAssert(
    const char *    fname,
    int             line,
    const string &  msg)
{
    fgThrow("Internal program error", fgDiagString(fname,line,msg));
}

std::string
fgDateTimeString()
{
    time_t          rawtime;
    time(&rawtime);
    const int       buffSize = 256;
    char            buffer[buffSize] = {0};
    tm              *timeinfo = localtime(&rawtime);
    strftime(buffer,buffSize-1,"%y%m%d_%H%M%S",timeinfo);
    return string(buffer);
}
