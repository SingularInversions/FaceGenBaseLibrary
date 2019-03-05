//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     19.01.12
//
// WARNING: libstdc++ specializes less<> for shared_ptr so custom
// operator<(shared_ptr<MyType>,shared_ptr<MyType>)
// will not work as expected with std::set etc. using libstdc++.
// Must use operator class comparators (see below).

#ifndef FGSTDPTR_HPP
#define FGSTDPTR_HPP

#include "FgStdLibs.hpp"

namespace std {

// A useful default:
template<class T>
ostream & operator<<(ostream & ss,const shared_ptr<T> & p)
{
    if (p)
        return ss << *p;
    else
        return ss << "NULL";
}

}

// * operator<(shared_ptr,shared_ptr) is defined and compares pointer values.
// * Although not what you usually want, it avoids the issue of invalid pointers.
// * less<>::operator() is specialized for builtins and just calls operator<() for class/structs.
// * libstdc++ specializes less<> for shared_ptr (it shouldn't) so we leave that alone.
// * containers default to comparisons by way of std::less.
// * Use this as a container template comparison operator for pointed value comparison:
template<class T>
struct FgSharedPtrDerefLess {
    bool
    operator()(const std::shared_ptr<T> & lhs,const std::shared_ptr<T> & rhs) const
    {
        FGASSERT(lhs && rhs);
        return std::less<T>()(*lhs,*rhs);
    }
};

#endif
