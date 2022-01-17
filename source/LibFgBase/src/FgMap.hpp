//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// An associative container alternative to std::Map for small collections:
//  * based on std::vector
//  * higher-level interface than iterators
//  * without the implicit insertion semantics of std::map
//  * the collections are assumed small enough that no sorting is done and lookup is O(n).
//  * key type must support operator==
//  * Looked at loki::AssocVector but didn't like the implicit insertion and OOPy, template-complex design.
//

#ifndef FGMAP_HPP
#define FGMAP_HPP

#include "FgStdLibs.hpp"
#include "FgDiagnostics.hpp"
#include "FgOpt.hpp"

namespace Fg {

template<typename K,typename V>
struct  Map
{
    Svec<std::pair<K,V> >    map;

    const V &
    operator[](const K & k) const       // No implicit insertion - throws if key not found
    {
        for (const std::pair<K,V> & p : map)
            if (p.first == k)
                return p.second;
        FGASSERT_FALSE;
        return map[0].second;       // Avoid warning
    }

    Opt<V>
    find(const K & k) const
    {
        for (size_t ii=0; ii<map.size(); ++ii)
            if (map[ii].first == k)
                return Opt<V>(map[ii].second);
        return Opt<V>();
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
contains(const Map<K,V> & map,const K & key)
{
    for (const std::pair<K,V> & p : map.map)
        if (p.first == key)
            return true;
    return false;
}

}

#endif
