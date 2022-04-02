//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGSTDMAP_HPP
#define FGSTDMAP_HPP

#include "FgStdLibs.hpp"
#include "FgException.hpp"

namespace Fg {

template<class Key,class Val>
std::map<Key,Val>   operator+(std::map<Key,Val> const & lhs,std::map<Key,Val> const & rhs)
{
    std::map<Key,Val>    ret = lhs;
    ret.insert(rhs.begin(),rhs.end());
    return ret;
}

template<class Key,class Val>
void                operator+=(std::map<Key,Val> & lhs,std::map<Key,Val> const & rhs)
{lhs.insert(rhs.begin(),rhs.end()); }

template<class Key,class Val,class Lt>
bool                contains(const std::map<Key,Val,Lt> & map,const Key & key)
{return (map.find(key) != map.end()); }

// Accumulate weights of type W for different values of type T in map<T,W>.
// Useful for frequency maps or weight maps:
template<class T,class W>
void                mapAccWeight_(std::map<T,W> & map,T const & val,W weight=W(1))
{
    auto    it = map.find(val);
    if (it == map.end())
        map[val] = weight;
    else
        it->second += weight;
}

// As above but accumulate all weights in 'rhs' to 'lhs':
template<class T,class W>
void                mapAccWeight_(std::map<T,W> & lhs,const std::map<T,W> & rhs)
{
    for (auto const & it : rhs)
        mapAccWeight_(lhs,it.first,it.second);
}

}

#endif
