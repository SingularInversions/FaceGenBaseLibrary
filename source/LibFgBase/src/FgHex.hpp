//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Useful to create strings directly rather than going through ostringstream with 'std::hex'
// and 'std::uppercase' and uint-casting for uchar.
//
// Big-endian (ie highest order digits printed first)
// Signed and unsigned are treated identically (hex representation of bit pattern)

#ifndef FGHEX_HPP
#define FGHEX_HPP

#include "FgTypes.hpp"
#include "FgStdString.hpp"

namespace Fg {

String  toHexString(uchar c);

// Returns hex encoding in string of length 2:
String  toHexString(const uchar *arr,uint numBytes);

// Returns hex encoding in string of length 4:
String  toHexString(uint16 val);
inline String toHexString(int16 val) {return toHexString(scast<uint16>(val)); }

// Returns hex encoding in string of length 8
String  toHexString(uint32 val);
inline String toHexString(int32 val) {return toHexString(scast<uint32>(val)); }

// Returns hex encoding in string of length 16:
String  toHexString(uint64 val);
inline String toHexString(int64 val) {return toHexString(scast<uint64>(val)); }

}

#endif
