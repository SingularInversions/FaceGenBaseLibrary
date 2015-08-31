//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Nov 21, 2005
//

#include "stdafx.h"

#include "FgVariant.hpp"

using namespace std;

FgVariant &
FgVariant::operator=(
    const FgVariant &   var)
{
    FgVariant   tmp;
    tmp.m_poly = var.m_poly->clone();
    std::swap(m_poly,tmp.m_poly);       // Can't throw
    return *this;
}

std::ostream &
FgVariant::print(std::ostream & ss) const
{
    ss << m_poly->typeName() << ": " << fgpush;
    m_poly->print(ss);
    ss << fgpop;
    return ss;
}

// */
