//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Feb 25, 2011
//
// stdio related functionality - defined in platform specific libraries
//

#ifndef FGSTDIO_HPP
#define FGSTDIO_HPP

#include "FgStdLibs.hpp"
#include "FgString.hpp"

FILE *
fgOpen(
    const FgString &    filename,
    bool                write,      // false = read
    bool                binary);    // false = text

#endif

// */
