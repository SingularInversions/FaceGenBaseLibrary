//
// Copyright (c) 2018 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     18.04.14
//
// An associative container for small collections based on std::vector and without the implicit insertion
// semantics of std::map. The collections are assumed small enough that no sorting is done and lookup is O(n).
// Key type must support operator==
//
// * Looked at loki::AssocVector but didn't like the implicit insertion and OOPy, template-complex design.
//

#ifndef FGMAP_HPP
#define FGMAP_HPP

#include "FgStdLibs.hpp"
#include "FgDiagnostics.hpp"
#include "FgOpt.hpp"

template<typename K,typename V>
struct  FgMap
{
    std::vector<std::pair<K,V> >    map;

    const V &
    operator[](const K & k) const       // No implicit insertion - throws if key not found
    {
        for (const std::pair<K,V> & p : map)
            if (p.first == k)
                return p.second;
        FGASSERT_FALSE;
        return map[0].second;       // Avoid warning
    }

    FgOpt<V>
    find(const K & k) const
    {
        for (size_t ii=0; ii<map.size(); ++ii)
            if (map[ii].first == k)
                return FgOpt<V>(map[ii].second);
        return FgOpt<V>();
    }

    void
    insert(const K & k,const V & v)     // Throws if key already in use
    {
        for (const std::pair<K,V> & p : map)
            FGASSERT(!(k == p.first));
        map.push_back(std::make_pair(k,v));
    }
};

template<typename K,typename V>
bool
fgContains(const FgMap<K,V> & map,const K & key)
{
    for (const std::pair<K,V> & p : map.map)
        if (p.first == key)
            return true;
    return false;
}

#endif
