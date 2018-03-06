//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: 2018.01.10
//

#include "stdafx.h"

#include "FgStdVector.hpp"
#include "FgTestUtils.hpp"

void
fgStdVectorTest(const FgArgs &)
{
    FgInts          v = { 1, 2, 3 };
    FgIntss         subs = fgSubsets(v,0,4);
    fgout << fgnl << "Subsets of 1..3 : " << subs;
    FGASSERT(subs.size() == 8);
    subs = fgSubsets(v,2,2);
    fgout << fgnl << "Of which size 2 : " << subs;
    FgIntss         chk = {{1,2},{1,3},{2,3}};
    FGASSERT(subs == chk);
}
