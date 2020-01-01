//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"
#include "FgHex.hpp"
#include "FgDiagnostics.hpp"

using namespace std;

namespace Fg {

// Generate single hex char. Value must be < 16:
static
char
toHexChar(uchar c4)
{
    FGASSERT(c4 < 16);
    static const char * digits = "0123456789ABCDEF";
    return digits[c4];
}

string
toHexString(uchar c)
{
    return
        string(1,toHexChar(c >> 4)) +
        string(1,toHexChar(c & 0xF));
}

string
toHexString(const uchar *arr,uint numBytes)
{
    string  ret;
    for (uint ii=0; ii<numBytes; ++ii)
        ret += toHexString(*arr++);
    return ret;
}

string
toHexString(uint16 val)
{
    return
        toHexString(uchar(val >> 8)) +
        toHexString(uchar(val & 0xFF));
}

string
toHexString(uint32 val)
{
    return
        toHexString(uint16(val >> 16)) +
        toHexString(uint16(val & 0xFFFF));
}

string
toHexString(uint64 val)
{
    return
        toHexString(uint32(val >> 32)) +
        toHexString(uint32(val & 0xFFFFFFFF));
}

}
