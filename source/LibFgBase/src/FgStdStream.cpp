//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Feb 24, 2011
//

#include "stdafx.h"
#include "FgStdStream.hpp"
#include "FgException.hpp"

using namespace std;

bool
FgOfstream::open(
    const FgString &        fname,
    bool                    append,
    bool                    throwOnFail)
{
    // Opening a file can throw if ios::exceptions have been enabled (not the default):
    try
    {
#ifdef _WIN32
        wstring     fn = fname.as_wstring();
#else       // Any sane OS will use UTF-8:
        string      fn = fname.as_utf8_string();
#endif
        ios::openmode   om = ios::binary;
        if (append)
            om = om | ios::app;
        ofstream::open(fn.c_str(),om);
    }
    catch (...)
    {}
    if (!is_open() && throwOnFail)
        fgThrow("Unable to open file for writing",fname);
    return is_open();
}

bool
FgIfstream::open(
    const FgString &        fname,
    bool                    throwOnFail)
{
    // Use try-catch in case client has enabled ios::exceptions (not enabled by default):
    try
    {
#ifdef _WIN32
        wstring     fn = fname.as_wstring();
#else
        string      fn = fname.as_utf8_string();
#endif
        ifstream::open(fn.c_str(),std::ios::binary);
    }
    catch (...)
    {}
    if (!is_open() && throwOnFail)
        fgThrow("Unable to open file for reading",fname);
    return is_open();
}

void
fgWriteFile(const FgString & fname,const std::string & data,bool append)
{
    FgOfstream  ofs(fname,append);
    ofs << data;
}

// */
