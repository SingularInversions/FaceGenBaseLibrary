//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
//

#include "stdafx.h"

#include "FgStdStream.hpp"
#include "FgHex.hpp"
#include "FgTime.hpp"
#include "FgGuiApiBase.hpp"

using namespace std;

namespace Fg {

bool
Ofstream::open(
    String8 const &         fname,
    bool                    appendFile,
    bool                    throwOnFail)
{
    wstring             fn = fname.as_wstring();
    // Always use binary read/write. Text translation mode is complex, error-prone and unnecessary:
    ios::openmode       om = ios::binary;
    if (appendFile)
        om = om | ios::app;
    // Opening a file can throw if ios::exceptions have been enabled (not the default):
    try
    {
        ofstream::open(fn.c_str(),om);
    }
    catch (...)
    {}
    if (!is_open() && (GetLastError() == ERROR_ACCESS_DENIED)) {
        sleepSeconds(1);            // See if Windows needed time to update filesystem
        ofstream::open(fn.c_str(),om);
        if (is_open()) {
            if (g_guiDiagHandler.reportError)
                g_guiDiagHandler.reportError("MESSAGE: ofstrream::open sleep fixed access error");
        }
        else if (throwOnFail)
            fgThrow("Windows denied permission to write file (ofstream::open)",fname);
    }
    if (!is_open() && throwOnFail) {
        DWORD               gle = GetLastError();
        string              diag = toHexString(uint32(gle));
        fgThrow("Unable to write file (ofstream::open)",diag,fname);
    }
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
