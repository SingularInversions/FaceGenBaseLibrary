//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     June 7, 2017
//
// Currently boost::function but will migrate:

#ifndef FGSTDFUNCTION_HPP
#define FGSTDFUNCTION_HPP

#include "FgStdLibs.hpp"
#include "FgBoostLibs.hpp"

typedef boost::function<void()>                                 FgFunc;
typedef boost::function<std::string(const std::string &)>       FgFuncStr2Str;

#endif
