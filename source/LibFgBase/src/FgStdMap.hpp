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

#endif
