//
// Copyright (c) 2017 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     July 15, 2017
//

#include "stdafx.h"

#include "FgTensor.hpp"
#include "FgCommand.hpp"

using namespace std;

void
fgTensorTest(const FgArgs &)
{
    FgInts          data0 = fgSvec(0,1,2,3,4,5,6,7,8,9),
                    data1 = fgSvec(0,1,2,3);
    FgTensor3I      t(2,3,4,fgCat(data0,data0,data1));
    FgVect3UI       idx(0);
    FGASSERT(t[idx] == 0);
    ++idx[0];
    FGASSERT(t[idx] == 1);
    ++idx[1];
    FGASSERT(t[idx] == 3);
    ++idx[2];
    FGASSERT(t[idx] == 9);
    FgTensor3I      u = t.transpose(0,1);
    idx = FgVect3UI(0,0,2);
    FGASSERT(t[idx] == u[idx]);
    FGASSERT(t == u.transpose(0,1));
    FgTensor3I      v = u.transpose(1,2),
                    w = v.transpose(0,1);
    FGASSERT(t == w.transpose(0,2));
    FgTensor3I      x = t.reorder(FgVect3UI(1,2,0)),
                    y = t.reorder(FgVect3UI(2,1,0));
    FGASSERT(x == v);
    FGASSERT(y == w);
}

// */
