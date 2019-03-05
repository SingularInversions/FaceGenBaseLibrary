//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Jan 19, 2007
//

#ifndef FGSYSTEMINFO_HPP
#define FGSYSTEMINFO_HPP

#include "FgStdLibs.hpp"
#include "FgStdString.hpp"
#include "FgString.hpp"

// Is the OS we're currently running on a 64-bit version ?
bool        fg64bitOS();

// Note that this will return the EMULATED OS version if the install manifest is for an earlier
// version of Windows. For example, building with vs2012 will return Windows 8 for Win8.1 and Win10
// hosts:
string      fgOsName();

FgString    fgComputerName();

#endif

// */
