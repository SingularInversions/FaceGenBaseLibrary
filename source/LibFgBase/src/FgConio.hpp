//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     July 12, 2010
//
// Provide platform-independent access to console functionality not available through C/C++ standards.

#ifndef FGCONIO_HPP
#define FGCONIO_HPP

// Non-blocking check for unhandled keystroke:
bool
fgKbhit();

// Get keystroke:
char
fgGetch();

// Get console character visible display width (windows also has a buffer width).
// Returns 80 if not available.
unsigned int
fgConsoleWidth();

#endif
