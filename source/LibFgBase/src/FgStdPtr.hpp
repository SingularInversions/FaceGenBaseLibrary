//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// WARNING: libstdc++ specializes less<> for shared_ptr so custom
// operator<(shared_ptr<MyType>,shared_ptr<MyType>)
// will not work as expected with std::set etc. using libstdc++.
// Must use operator class comparators (see below).

#ifndef FGSTDPTR_HPP
#define FGSTDPTR_HPP

#include "FgStdLibs.hpp"

namespace Fg {

template<class T>
using Sptr = std::shared_ptr<T>;

template<class T>
using Uptr = std::unique_ptr<T>;

// A useful default:
template<class T>
std::ostream &
operator<<(std::ostream & ss,std::shared_ptr<T> const & p)
{
    if (p)
        return ss << *p;
    else
        return ss << "NULL";
}

}

#endif
