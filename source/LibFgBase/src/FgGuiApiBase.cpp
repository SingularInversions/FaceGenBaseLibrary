//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"
#include "FgGuiApiBase.hpp"

using namespace std;

namespace Fg {

bool
isGuiSupported()
{
#ifdef _WIN32
    return true;
#else
    return false;
#endif
}

GuiExceptHandler        g_guiDiagHandler;

}

// */
