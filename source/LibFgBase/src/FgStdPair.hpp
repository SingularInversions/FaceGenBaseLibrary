//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Jan 19, 2005
//

#ifndef FGSTDPAIR_HPP
#define FGSTDPAIR_HPP

#include "FgStdLibs.hpp"

template<class T,class U>
std::ostream& operator<<(std::ostream& ss,const std::pair<T,U> &pp)
{
    ss << "(" << pp.first << "," << pp.second << ")";
    return ss;
}

#endif
