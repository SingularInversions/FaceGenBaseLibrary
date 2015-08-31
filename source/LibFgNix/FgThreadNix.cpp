//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgThread.hpp"
#include "FgDiagnostics.hpp"

// TODO: This implementation assumes that you can throw C++ exceptions
// through C stacks. This is true on Linux but not on Mac. Should
// probably modify singleton class to catch this issue.
void fgRunOnce(FgOnce & once,
               void(*init_routine)())
{
    FGASSERT(init_routine);
    FGASSERT(pthread_once(&once,init_routine) == 0);
}
