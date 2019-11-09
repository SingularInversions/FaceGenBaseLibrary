//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGSTDEXTENSIONS_HPP
#define FGSTDEXTENSIONS_HPP

#include "FgStdLibs.hpp"
#include "FgStdVector.hpp"
#include "FgStdArray.hpp"
#include "FgStdMap.hpp"
#include "FgStdPair.hpp"
#include "FgStdPtr.hpp"
#include "FgStdSet.hpp"
#include "FgStdStream.hpp"
#include "FgStdString.hpp"

namespace Fg {

typedef Svec<std::string>    strings;

template<class T>
using Sfun = std::function<T>;

}

#endif
