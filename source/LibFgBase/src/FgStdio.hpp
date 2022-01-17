//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// C-style FILE* is needed for use of C libraries and older code.
// This provides safe unicode cross-platform 'fopen'
//

#ifndef FGSTDIO_HPP
#define FGSTDIO_HPP

#include "FgStdLibs.hpp"
#include "FgString.hpp"

namespace Fg {

// Always opens in 'binary' mode. Throws a descriptive error if the file cannot be opened.
// Avoid this function on Windows, especially for writing, as it seems to encounter denied permissions
// in situations where opening an ofstream doesn't:
FILE *
openFile(
    String8 const &    filename,
    bool                write);         // false = read

#endif

}

// */
