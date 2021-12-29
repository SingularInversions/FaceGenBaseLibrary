//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Codebase version; major / minor / patch

#ifndef FGVERSION_HPP
#define FGVERSION_HPP

#include "FgStdString.hpp"

namespace Fg {

inline
std::string
getSdkVersion(const std::string & sep)
{
    return "3" + sep + "S" + sep + "2";
}

}

#endif
