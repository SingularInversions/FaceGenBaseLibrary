//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Useful to create strings directly rather than going through ostringstream with 'std::hex'
// and 'std::uppercase' and uint-casting for uchar.
//
// Big-endian (ie highest order digits printed first)
//

#ifndef FGHEX_HPP
#define FGHEX_HPP

#include "FgTypes.hpp"
#include "FgStdString.hpp"

namespace Fg {

String
fgAsHex(uchar c);

// Returns hex encoding in string of length 2:
String
fgAsHex(
    const uchar *arr,
    uint        numBytes);

// Returns hex encoding in string of length 4:
String
fgAsHex(uint16 val);

// Returns hex encoding in string of length 8
String
fgAsHex(uint32 val);

// Returns hex encoding in string of length 16:
String
fgAsHex(uint64 val);

}

#endif
