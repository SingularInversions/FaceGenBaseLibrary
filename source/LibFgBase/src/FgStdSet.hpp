//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Mar 2, 2013
//
// Avoid the abomination that is std::algorithm on std::set
//

#ifndef FGSTDSET_HPP
#define FGSTDSET_HPP

#include "FgStdLibs.hpp"

template<class T>
std::ostream &
operator<<(std::ostream & os,const std::set<T> & v)
{
    os << "{";
    for (typename std::set<T>::const_iterator it=v.begin(); it != v.end(); ++it)
        os << *it << ",";
    return os << "}";
}

template<class T>
void
fgAppend(std::set<T> & s0,const std::set<T> & s1)
{s0.insert(s1.begin(),s1.end()); }

template<class T>
void
fgAppend(std::set<T> & s0,const std::vector<T> & s1)
{s0.insert(s1.begin(),s1.end()); }

template<class T>
std::set<T>
fgUnion(const std::set<T> & s0,const std::set<T> & s1)
{
    // WTF is with set_union ...
    std::set<T>     ret = s0;
    ret.insert(s1.begin(),s1.end());
    return ret;
}

#endif
