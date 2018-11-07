//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Sohail Somani
//

#include "stdafx.h"
#include "FgThread.hpp"
#include "FgOut.hpp"

using namespace std;

bool    fg_debug_thread = false;
// Cannot use fgout here since it makes use of this class. Manually enable if
// debug output is needed:
/*
#define LOG_DEBUG_THREAD(x)                                 \
    cout << "\nThread id: "                           \
    << boost::this_thread::get_id() << " " << x;
*/
#define LOG_DEBUG_THREAD(x)

