//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
//

#include "stdafx.h"

#include "FgStdStream.hpp"
#include "FgHex.hpp"

using namespace std;

namespace Fg {

bool
Ofstream::open(
    Ustring const &         fname,
    bool                    appendFile,
    bool                    throwOnFail)
{
    // Opening a file can throw if ios::exceptions have been enabled (not the default):
    try
    {
        wstring         fn = fname.as_wstring();
        // Always use binary read/write. Text translation mode is complex, error-prone and unnecessary:
        ios::openmode   om = ios::binary;
        if (appendFile)
            om = om | ios::app;
        ofstream::open(fn.c_str(),om);
    }
    catch (...)
    {}
    if (!is_open() && throwOnFail) {
        DWORD               le = GetLastError();
        string              diag = toHexString(uint32(le));
        fgThrow("Unable to write file (ofstream::open)",diag,fname);
    }
    return is_open();
}

bool
Ifstream::open(
    Ustring const &         fname,
    bool                    throwOnFail)
{
    // Use try-catch in case client has enabled ios::exceptions (not enabled by default):
    try
    {
        wstring         fn = fname.as_wstring();
        ifstream::open(fn.c_str(),std::ios::binary);
    }
    catch (...)
    {}
    if (!is_open() && throwOnFail) {
        DWORD               le = GetLastError();
        string              diag = toHexString(uint32(le));
        fgThrow("Unable to read file (ifstream::open)",diag,fname);
    }
    return is_open();
}

}
