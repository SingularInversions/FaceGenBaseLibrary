//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     July 12, 2010
//

#include "stdafx.h"

#include "FgConio.hpp"
#include <conio.h>

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
