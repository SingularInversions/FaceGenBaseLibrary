//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgTypes.hpp"

namespace Fg {

std::string         cBitsString()
{
#ifdef FG_64
    return "64";
#else
    return "32";
#endif
}

void                testPlatform()
{
#ifdef FG_64
    static_assert(sizeof(int*) == 8,"FG_64 macro is set for a 32-bit build");
#else
    static_assert(sizeof(int*) == 4,"FG_64 macro is not set for a 64-bit build");
#endif
}

}
