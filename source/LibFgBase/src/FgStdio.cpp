//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"
#include "FgStdio.hpp"
#include "FgCommand.hpp"

using namespace std;

namespace Fg {

#ifdef _WIN32

// function definition in LibFgWin/FgStdioWin.cpp

#else

FILE *
openFile(String8 const & fname,bool write)
{
    FILE *          fPtr;
    char const *    mode = write ? "wb" : "rb";
    fPtr = fopen(fname.m_str.c_str(),mode);
    if (fPtr == nullptr) {
        string          msg = "Unable to open file for " + string(write ? "writing" : "reading");
        fgThrow(msg,fname.m_str);
    }
    return fPtr;
}

#endif

void
testFopen(CLArgs const & args)
{
    FGTESTDIR;
    char32_t        ch = 0x00004EE5;            // A Chinese character
    String8        chinese(ch);
    string          data = "test data";
    FILE *          fPtr = openFile(chinese,true);
    fwrite(data.data(),1,data.size(),fPtr);
    fclose(fPtr);
    string          test;
    test.resize(data.size());
    fPtr = openFile(chinese,false);
    size_t          sz = fread(&test[0],1,test.size(),fPtr);
    fclose(fPtr);
    FGASSERT(sz == data.size());
    FGASSERT(test == data);
}

}

// */
