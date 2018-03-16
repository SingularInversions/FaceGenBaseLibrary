//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     March 26, 2012
//
// Codebase version; major / minor / patch

#ifndef FGVERSION_HPP
#define FGVERSION_HPP

#include "FgStdString.hpp"

inline
std::string
fgVersion(const std::string & sep)
{
    return "3" + sep + "M" + sep + "0";
}

#endif
