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
asSingleHex(uchar c4)
{
    if (c4 < 10)
        return (c4+48); // Digit
    else {
        FGASSERT(c4 < 16);
        return (c4+55); // Capital letters start at 65
    }
}

string
fgAsHex(uchar c)
{
    return
        string(1,asSingleHex(c >> 4)) +
        string(1,asSingleHex(c & 0xF));
}

string
fgAsHex(const uchar *arr,uint numBytes)
{
    string  ret;
    for (uint ii=0; ii<numBytes; ++ii)
        ret += fgAsHex(*arr++);
    return ret;
}

string
fgAsHex(uint16 val)
{
    return
        fgAsHex(uchar(val >> 8)) +
        fgAsHex(uchar(val & 0xFF));
}

string
fgAsHex(uint32 val)
{
    return
        fgAsHex(uint16(val >> 16)) +
        fgAsHex(uint16(val & 0xFFFF));
}

string
fgAsHex(uint64 val)
{
    return
        fgAsHex(uint32(val >> 32)) +
        fgAsHex(uint32(val & 0xFFFFFFFF));
}

}
