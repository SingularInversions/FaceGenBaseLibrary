//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGSYSTEMINFO_HPP
#define FGSYSTEMINFO_HPP

#include "FgStdLibs.hpp"
#include "FgStdString.hpp"
#include "FgString.hpp"

namespace Fg {

// Is the OS we're currently running on a 64-bit version ?
bool        fg64bitOS();

// Will always return Windows 8 or lower unless the manifest *requires* a higher version of Windows,
// in which case that version will be returned:
String      osDescription();

Ustring    fgComputerName();

}

#endif

// */
