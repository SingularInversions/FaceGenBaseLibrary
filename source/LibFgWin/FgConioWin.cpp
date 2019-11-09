//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     July 12, 2010
//

#include "stdafx.h"

#include "FgConio.hpp"
#include <conio.h>

namespace Fg {

bool
fgKbhit()
{
    return (_kbhit() != 0);
}

char
fgGetch()
{
    return char(_getch());
}

unsigned int
fgConsoleWidth()
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    BOOL        rc = GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    if (rc)
        return static_cast<unsigned int>(1 + csbi.srWindow.Right - csbi.srWindow.Left);
    else
        return 80U;
}

}
