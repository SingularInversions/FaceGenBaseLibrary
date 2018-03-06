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

// A frequency map counts the number of additions of each object.
// The value for any object not added is automatically zero.
template<class T>
struct FgFreqMap
{
    std::map<T,size_t>      map;

    void add(const T & v)
    {
        auto    it = map.find(v);
        if (it == map.end())
            map[v] = 1;
        else
            ++(it->second);
    }

    void addMany(const T & v,size_t num)
    {
        auto    it = map.find(v);
        if (it == map.end())
            map[v] = num;
        else
            it->second += num;
    }

    size_t count(const T & v) const
    {
        auto    it = map.find(v);
        if (it == map.end())
            return 0;
        else
            return it->second;
    }

    std::pair<T,size_t>
    max() const
    {
        std::pair<T,size_t>     ret;
        ret.second = 0;
        for (auto it=map.begin(); it!=map.end(); ++it)
            if (it->second > ret.second)
                ret = *it;
        return ret;
    }
};

#endif
