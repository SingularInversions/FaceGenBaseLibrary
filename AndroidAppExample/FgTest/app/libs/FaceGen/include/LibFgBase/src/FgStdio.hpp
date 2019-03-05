//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Feb 25, 2011
//
// C-style FILE* is needed for use of C libraries and older code.
// This provides safe unicode cross-platform 'fopen'
//

#ifndef FGSTDIO_HPP
#define FGSTDIO_HPP

#include "FgStdLibs.hpp"
#include "FgString.hpp"

// Always opens in 'binary' mode. Throws a descriptive error if the file cannot be opened.
FILE *
fgOpen(
    const FgString &    filename,
    bool                write);         // false = read

#endif

// */
