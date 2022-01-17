//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
//

#include "stdafx.h"

#include "FgStdio.hpp"
#include "FgHex.hpp"
#include "FgTime.hpp"
#include "FgGuiApiBase.hpp"

using namespace std;

namespace Fg {

FILE *
openFile(String8 const & fname,bool write)
{
    FILE *              fPtr;
    // Always use binary read/write. Text translation mode is complex, error-prone and unnecessary:
    wchar_t const *     mode = write ? L"wb" : L"rb";
    // _w: Wide char version allows us to specify a UTF16 filename (char* will be interpreted as ANSII)
    // _s: Secure version because original is deprecated
    // !fsopen: Non-sharing version because we don't need to share
    errno_t             err = _wfopen_s(&fPtr,fname.as_wstring().c_str(),mode);
    if (err == EACCES) {        // Permissions error
        sleepSeconds(1);        // See if Windows needed time to update filesystem
        err = _wfopen_s(&fPtr,fname.as_wstring().c_str(),mode);
        if (err == 0) {
            if (g_guiDiagHandler.reportError)
                g_guiDiagHandler.reportError("MESSAGE: fopen sleep fixed access error");
        }
        else {
            if (write)
                fgThrow("Windows denied permission to write file (fopen)",fname.m_str);
            else
                fgThrow("Windows denied permission to read file (fopen)",fname.m_str);
        }
    }
    else if (err != 0) {
        if (write)
            fgThrow("Unable to write file (fopen)",toStr(err),fname);
        else
            fgThrow("Unable to read file (fopen)",toStr(err),fname);
    }
    FGASSERT(fPtr != nullptr);
    return fPtr;
}

}
