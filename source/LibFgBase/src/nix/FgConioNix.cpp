//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     July 12, 2010
//
// Non-functioning stub.

#include "stdafx.h"

#include "FgConio.hpp"
#include "FgDiagnostics.hpp"

#include <sys/ioctl.h>
#include <unistd.h>

namespace Fg {

bool
fgKbhit()
{
    return false;
}

char
fgGetch()
{
    return 0;
}

unsigned int
fgConsoleWidth()
{
    struct winsize      w;
    int                 rc = ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    if (rc == 0)
        return static_cast<unsigned int>(w.ws_col);
    else
        return 80U;
}

}
