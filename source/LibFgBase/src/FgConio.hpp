//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     July 12, 2010
//
// Provide platform-independent access to direct command-line keyboard input (not available through C standard).
// Currently only defined in LibFgWin.
//

#ifndef FGCONIO_HPP
#define FGCONIO_HPP

// Non-blocking check for unhandled keystroke:
bool
fgKbhit();

// Get keystroke:
char
fgGetch();

#endif
