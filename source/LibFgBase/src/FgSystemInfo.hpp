//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGSYSTEMINFO_HPP
#define FGSYSTEMINFO_HPP

#include "FgSerial.hpp"

namespace Fg {

// Is the OS we're currently running on a 64-bit version ?
bool        is64bitOS();

// Will always return Windows 8 or lower unless the manifest *requires* a higher version of Windows,
// in which case that version will be returned:
String      osDescription();

// Only currently implemented on Windows:
String8     getDefaultGpuDescription();

String8    fgComputerName();

}

#endif

// */
