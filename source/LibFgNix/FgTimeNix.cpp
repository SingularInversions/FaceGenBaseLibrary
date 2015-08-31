//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: Nov 7, 2011
//

#include "stdafx.h"
#include "FgTime.hpp"

#include <sys/types.h>
#include <sys/timeb.h>
#include <unistd.h>

using namespace std;

uint64
fgTimeMs()
{
    struct  timeb   timeptr;
    ftime(&timeptr);
    return uint64(timeptr.time) * 1000ULL + uint64(timeptr.millitm);
}

void
fgSleep(uint seconds)
{
    sleep(seconds);
}

