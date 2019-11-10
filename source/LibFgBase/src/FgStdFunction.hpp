//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGSTDFUNCTION_HPP
#define FGSTDFUNCTION_HPP

#include "FgStdLibs.hpp"

namespace Fg {

typedef std::function<void()>                                 FgFnVoid2Void;
typedef std::function<std::string(const std::string &)>       FgFnStr2Str;

}

#endif
