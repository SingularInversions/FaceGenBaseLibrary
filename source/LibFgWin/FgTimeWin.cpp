//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
//

#include "stdafx.h"
#include "FgTime.hpp"

#include <sys/types.h>
#include <sys/timeb.h>

using namespace std;

namespace Fg {

uint64              getTimeMs()
{
    struct  __timeb64   timeptr;
    errno_t             err = _ftime64_s(&timeptr);
    FGASSERT(err==0);
    return timeptr.time * 1000ULL + uint64(timeptr.millitm);
}

void                sleepSeconds(uint seconds)
{
    Sleep(seconds * 1000);
}

}
