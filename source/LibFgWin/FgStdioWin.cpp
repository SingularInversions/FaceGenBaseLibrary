//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
//

#include "stdafx.h"

#include "FgStdio.hpp"
#include "FgHex.hpp"

using namespace std;

namespace Fg {

FILE *
openFile(Ustring const & fname,bool write)
{
    FILE *              fPtr;
    // Always use binary read/write. Text translation mode is complex, error-prone and unnecessary:
    wchar_t const *     mode = write ? L"wb" : L"rb";
    // _w: Wide char version allows us to specify a UTF16 filename (char* will be interpreted as ANSII)
    // _s: Secure version because original is deprecated
    // !fsopen: Non-sharing version because we don't need to share
    errno_t             err = _wfopen_s(&fPtr,fname.as_wstring().c_str(),mode);
    if (err != 0) {
        if (err == EACCES) {
            string          diag;
            HANDLE          hndl = CreateFile(
                    fname.as_wstring().c_str(),
                    write ? GENERIC_WRITE : GENERIC_READ,
                    0,                              // not shared
                    NULL,                           // no security options
                    write ? CREATE_ALWAYS : OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);                          // don't get template info
            if (hndl == INVALID_HANDLE_VALUE) {
                DWORD       le = GetLastError();
                diag = "CF test fail " + toHexString(uint32(le));
            }
            else {
                CloseHandle(hndl);
                diag = "CF test positive";
            }
            if (write)
                fgThrow("Windows denied permission to write file (fopen)",diag,fname.m_str);
            else
                fgThrow("Windows denied permission to read file (fopen)",diag,fname.m_str);
        }
        else {
            if (write)
                fgThrow("Unable to write file (fopen)",toStr(err),fname);
            else
                fgThrow("Unable to read file (fopen)",toStr(err),fname);
        }
    }
    FGASSERT(fPtr != nullptr);
    return fPtr;
}

}
