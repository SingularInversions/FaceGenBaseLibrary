//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Jan 19, 2005
//

#ifndef FGSTDMAP_HPP
#define FGSTDMAP_HPP

#include "FgStdLibs.hpp"
#include "FgException.hpp"

// Use this when you know the Key must be present:
template<class Key,class Val>
Val
fgLookup(const std::map<Key,Val> & map,const Key & key)
{
    auto        it = map.find(key);
    if (it == map.end())
        fgThrow("fgLookup key not found");
    return it->second;
}

template<class Key,class Val>
bool
fgContains(const std::map<Key,Val> & map,const Key & key)
{return (map.find(key) != map.end()); }

template<class Key,class Val>
std::map<Key,Val>
fgMerge(const std::map<Key,Val> & l,const std::map<Key,Val> & r)
{
    std::map<Key,Val>   ret = l;
    ret.insert(r.begin(),r.end());
    return ret;
}

// Accumulate weights of type W for different values of type T in map<T,W>.
// Useful for frequency maps or weight maps:
template<class T,class W>
void
fgMapAccWeight_(std::map<T,W> & map,const T & val,W weight=W(1))
{
    auto    it = map.find(val);
    if (it == map.end())
        map[val] = weight;
    else
        it->second += weight;
}

// As above but accumulate all weights in 'rhs' to 'lhs':
template<class T,class W>
void
fgMapAccWeight_(std::map<T,W> & lhs,const std::map<T,W> & rhs)
{
    for (const auto & it : rhs)
        fgMapAccWeight_(lhs,it.first,it.second);
}

#endif
