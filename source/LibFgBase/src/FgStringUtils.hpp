//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Sohail Somani
//

#ifndef INCLUDED_FGSTRINGUTILS_HPP
#define INCLUDED_FGSTRINGUTILS_HPP

#include "FgStdStream.hpp"

inline
void
fgSlurpStream(std::istream & in, std::string & out)
{
    std::ostringstream stream;
    stream << in.rdbuf();
    out = stream.str();
}

#endif // FGSTRINGUTILS_HPP
