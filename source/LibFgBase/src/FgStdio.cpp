//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"
#include "FgStdio.hpp"
#include "FgCommand.hpp"

using namespace std;

namespace Fg {

#ifdef _WIN32

FILE *
fgOpen(Ustring const & fname,bool write)
{
    FILE *          fPtr;
    const wchar_t * mode = write ? L"wb" : L"rb";
    errno_t         err = _wfopen_s(&fPtr,fname.as_wstring().c_str(),mode);
    if (err != 0) {
        FgException     e;
        e.pushMsg("_wfopen_s error code",toString(err));
        string          msg = "Unable to open file for " + string(write ? "writing" : "reading");
        e.pushMsg(msg,fname.m_str);
        throw e;
    }
    FGASSERT(fPtr != nullptr);
    return fPtr;
}

#else

FILE *
fgOpen(Ustring const & fname,bool write)
{
    FILE *          fPtr;
    const char *    mode = write ? "wb" : "rb";
    fPtr = fopen(fname.m_str.c_str(),mode);
    if (fPtr == nullptr) {
        string          msg = "Unable to open file for " + string(write ? "writing" : "reading");
        fgThrow(msg,fname.m_str);
    }
    return fPtr;
}

#endif

void
fgOpenTest(CLArgs const & args)
{
    FGTESTDIR;
    char32_t        ch = 0x00004EE5;            // A Chinese character
    Ustring        chinese(ch);
    string          data = "test data";
    FILE *          fPtr = fgOpen(chinese,true);
    fwrite(data.data(),1,data.size(),fPtr);
    fclose(fPtr);
    string          test;
    test.resize(data.size());
    fPtr = fgOpen(chinese,false);
    size_t          sz = fread(&test[0],1,test.size(),fPtr);
    fclose(fPtr);
    FGASSERT(sz == data.size());
    FGASSERT(test == data);
}

}

// */
