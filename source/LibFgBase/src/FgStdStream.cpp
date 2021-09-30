//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"
#include "FgStdStream.hpp"
#include "FgException.hpp"

using namespace std;

namespace Fg {

#ifdef _WIN32

// Defined in LibFgWin/FgStdStreamWin.cpp

#else

bool
Ofstream::open(
    String8 const &         fname,
    bool                    appendFile,
    bool                    throwOnFail)
{
    // Opening a file can throw if ios::exceptions have been enabled (not the default):
    try
    {
        string      fn = fname.as_utf8_string();
        // Always use binary read/write. Text translation mode is complex, error-prone and unnecessary:
        ios::openmode   om = ios::binary;
        if (appendFile)
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
Ifstream::open(
    String8 const &         fname,
    bool                    throwOnFail)
{
    // Use try-catch in case client has enabled ios::exceptions (not enabled by default):
    try
    {
        string      fn = fname.as_utf8_string();
        ifstream::open(fn.c_str(),std::ios::binary);
    }
    catch (...)
    {}
    if (!is_open() && throwOnFail)
        fgThrow("Unable to open file for reading",fname);
    return is_open();
}

#endif

}

// */
