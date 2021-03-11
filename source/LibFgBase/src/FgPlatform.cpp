//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgPlatform.hpp"

namespace Fg {

std::string
cBitsString()
{
#ifdef FG_64
    return "64";
#else
    return "32";
#endif
}

}
