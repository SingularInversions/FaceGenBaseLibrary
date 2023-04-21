//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
//

#include "stdafx.h"

#include "FgFileSystem.hpp"
#include "FgTime.hpp"
#include "FgGuiApi.hpp"
#include "FgThrowWindows.hpp"

using namespace std;

namespace Fg {

// some antivirus programs hook into file creation and prevent creation of JPG files by
// unknown programs (ie. this one) causing ERROR_ACCESS_DENIED. Detect this and inform user.
// This has been confirmed for at least 2 customers. One was using Avast AV, other unknown.
static bool         checkForAntivirus_(String8 const & fname,FgException & except)
{
    Path                path {fname};
    String8             ext = toLower(path.ext);
    if ((ext == "jpg") || (ext == "jpeg")) {
        // ensure the problem wasn't caused by trying to overwrite an existing file of the
        // same name which was open in another application or write-protected:
        if (!fileExists(fname)) {
            path.ext = "fgtest";
            String8             testFname = path.str();
            FILE *              fPtr2 = nullptr;
            errno_t             err2 = _wfopen_s(&fPtr2,testFname.as_wstring().c_str(),L"wb");
            if (err2 == 0) {            // JPG creation is being blocked by antivirus software
                fclose(fPtr2);
                deleteFile(testFname);
                except.addContext(
                    "Your antivirus software prevented JPG file creation. "
                    "Change your antivirus software settings to allow this program to write JPG files, "
                    "or remove your third party antivirus software (Microsoft Windows Defender is better), "
                    "or choose a different image file format."
                );
                return true;
            }
        }
    }
    return false;
}

FILE *              openFile(String8 const & fname,bool write)
{
    FILE *              fPtr = nullptr;
    // Always use binary read/write. Text translation mode is complex, error-prone and unnecessary:
    wchar_t const *     mode = write ? L"wb" : L"rb";
    // _w: Wide char version allows us to specify a UTF16 filename (char* will be interpreted as ANSII)
    // _s: Secure version because original is deprecated
    // !fsopen: Non-sharing version because we don't need to share
    errno_t             err = _wfopen_s(&fPtr,fname.as_wstring().c_str(),mode);
    if (err != 0) {
        DWORD               lastErr = GetLastError();
        FgException         except {"Unable to write file (ofstream::open)",fname.m_str};
        if (write && (lastErr == ERROR_ACCESS_DENIED))
            if (checkForAntivirus_(fname,except))
                throw except;
        String          lastErrHex = toHexString(uint32(lastErr)),
                        english = getWinErrMsgEnglish(lastErr),
                        foreign = getWinErrMsgIfNotEnglish(lastErr);
        except.addContext(english,foreign,lastErrHex);
        throw except;
    }
    FGASSERT(fPtr != nullptr);
    return fPtr;
}

bool                Ofstream::open(String8 const & fname,bool appendFile,bool throwOnFail)
{
    // Always use binary; text translation mode is complex, error-prone and unnecessary:
    ios::openmode       mode = ios::binary;
    if (appendFile)
        mode = mode | ios::app;
    // Opening a file can throw if ios::exceptions have been enabled (not the default):
    try {ofstream::open(fname.as_wstring().c_str(),mode); }
    catch (...) {}
    if (!is_open() && throwOnFail) {
        DWORD               lastErr = GetLastError();
        FgException         except {"Unable to write file (ofstream::open)",fname.m_str};
        if (lastErr == ERROR_ACCESS_DENIED)
            if (checkForAntivirus_(fname,except))
                throw except;
        String          lastErrHex = "0x"+toHexString(uint32(lastErr)),
                        english = getWinErrMsgEnglish(lastErr),
                        foreign = getWinErrMsgIfNotEnglish(lastErr);
        except.addContext(english,foreign,lastErrHex);
        throw except;
    }
    return is_open();
}

bool                Ifstream::open(String8 const & fname,bool throwOnFail)
{
    // Use try-catch in case client has enabled ios::exceptions (not enabled by default):
    try {ifstream::open(fname.as_wstring().c_str(),std::ios::binary); }
    catch (...) {}
    if (!is_open() && throwOnFail) {
        DWORD           lastErr = GetLastError();
        String          lastErrHex = "win32 code 0x"+toHexString(uint32(lastErr)),
                        english = getWinErrMsgEnglish(lastErr),
                        foreign = getWinErrMsgIfNotEnglish(lastErr);
        throw FgException {{
            {"Unable to read file (ifstream::open)",fname.m_str},
            {english,foreign,lastErrHex},
        }};
    }
    return is_open();
}

}
