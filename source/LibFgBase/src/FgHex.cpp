//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"
#include "FgHex.hpp"
#include "FgSerial.hpp"

using namespace std;

namespace Fg {

// Generate single hex char. Value must be < 16:
static char         toHexChar(uchar c4)
{
    FGASSERT(c4 < 16);
    static char const * digits = "0123456789ABCDEF";
    return digits[c4];
}
string              toHexString(uchar c)
{
    return
        string(1,toHexChar(c >> 4)) +
        string(1,toHexChar(c & 0xF));
}
string              toHexString(uint16 val)
{
    return
        toHexString(uchar(val >> 8)) +
        toHexString(uchar(val & 0xFF));
}
string              toHexString(uint32 val)
{
    return
        toHexString(uint16(val >> 16)) +
        toHexString(uint16(val & 0xFFFF));
}
string              toHexString(uint64 val)
{
    return
        toHexString(uint32(val >> 32)) +
        toHexString(uint32(val & 0xFFFFFFFF));
}
string              bytesToHexString(const uchar *arr,uint numBytes)
{
    string  ret;
    for (uint ii=0; ii<numBytes; ++ii)
        ret += toHexString(*arr++);
    return ret;
}
string              toHex64Readable(uint64 id)
{
    uint16          parts[4];
    parts[0] = uint16((id >> 48) & 0xFFFF);
    parts[1] = uint16((id >> 32) & 0xFFFF);
    parts[2] = uint16((id >> 16) & 0xFFFF);
    parts[3] = uint16(id & 0xFFFF);
    string          ret;
    uint16          crc = 0;
    for (uint ii=0; ii<4; ++ii) {
        ret += toHexString(parts[ii]) + "-";
        crc = crc ^ parts[ii];
    }
    ret += toHexString(crc);
    return  ret;
}
static Valid<uint>  get4(char ch)
{
    Valid<uint>       ret;
    if ((ch >= '0') && (ch <= '9'))
        ret = uint(ch) - uint('0');
    else if ((ch >= 'a') && (ch <= 'f'))
        ret = uint(ch) - uint('a') + 10;
    else if ((ch >= 'A') && (ch <= 'F'))
        ret = uint(ch) - uint('A') + 10;
    else if ((ch == 'O') || (ch == 'o'))    // User confused 0 with letter O:
        ret = 0U;
    else if ((ch == 'I') || (ch == 'l'))    // User confused 1 with letter I or l
        ret = 1U;
    return ret;
}
static Valid<uint>  get16(istringstream & iss)
{
    uint            val = 0;
    uint            cnt = 0;
    for (;;) {
        int                 chi = iss.get();
        FGASSERT(chi < 256);
        if (iss.eof())
            return Valid<uint>();
        Valid<uint>       chVal = get4(scast<char>(chi));
        if (chVal.valid()) {
            val = (val << 4) + chVal.val();
            if (++cnt == 4)
                return Valid<uint>(val);
        }
    }
}
uint64              fromHex64Readable(String const & uk)
{
    uint64          ret = 0;
    uint16          crc = 0;
    istringstream   iss(uk);
    for (uint ii=0; ii<4; ++ii) {
        Valid<uint>       val = get16(iss);
        if (!val.valid())
            return 0;
        ret = (ret << 16) + val.val();
        crc = crc ^ static_cast<uint16>(val.val());
    }
    Valid<uint>   chk = get16(iss);
    if (chk.valid() && (chk.val() == crc))
        return ret;
    return 0;
}

}
