//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
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

// Like C++17 std::data() but better named:
template <class _Elem>
static
constexpr const _Elem* dataPtr(std::initializer_list<_Elem> _Ilist) noexcept
{return _Ilist.begin(); }

}

#endif
