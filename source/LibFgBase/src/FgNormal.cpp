//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Feb 3, 2009
//

#include "stdafx.h"

#include "FgNormal.hpp"

using namespace std;

std::ostream &
operator<<(std::ostream & ss,const FgNormal1D & norm)
{
    return ss << "Norm(Mean: " << norm.mean << " Stdev: " << norm.stdev << ")";
}

