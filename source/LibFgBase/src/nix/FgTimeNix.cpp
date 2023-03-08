//
// Copyright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"
#include "FgTime.hpp"

#include <sys/types.h>
#include <time.h>           // timespec, clock_getttime
#include <unistd.h>         // sleep

using namespace std;

namespace Fg {

uint64
getTimeMs()
{
    timespec        spec;
    clock_gettime(CLOCK_REALTIME,&spec);
    return uint64(spec.tv_sec)*1000ULL + uint64(spec.tv_nsec/1000000);
}

void
sleepSeconds(uint seconds)
{
    sleep(seconds);
}

}
