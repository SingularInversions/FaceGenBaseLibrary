//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

//

#ifndef FGSTDPAIR_HPP
#define FGSTDPAIR_HPP

#include "FgStdLibs.hpp"

namespace Fg {

template<class T,class U>
std::ostream &
operator<<(std::ostream & ss,std::pair<T,U> const & pp)
{
    return ss << "(" << pp.first << "," << pp.second << ")";
}

}

#endif
