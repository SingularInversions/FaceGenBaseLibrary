//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     2005
//

#include "stdafx.h"

#include "FgQuaternion.hpp"
#include "FgApproxEqual.hpp"
#include "FgRandom.hpp"
#include "FgMain.hpp"

using namespace std;

void
fgQuaternionTest(const FgArgs &)
{
    FgQuaternionD          q1(1,FgVect3D(0.001 * fgPi(),0,0)),
                            q2,q3;
    q2.setIdentity();
    q3.setIdentity();
    for (uint ii=0; ii<100; ii++)
        q2 = q2 * q1;
    for (uint ii=0; ii<1000; ii++)
        q3 = q3 * q1;
    FgMat33D            m1,
                            m2 = q2.asMatrix(),
                            m3 = q3.asMatrix(),
                            m4 = m2 * m2.transpose();
    m1.setIdentity();
    FGASSERT((m3-m1).length() < 0.0001);
    FGASSERT((m4-m1).length() < 0.0001);

    // test inverse
    FgQuaternionD   inv1(fgMatRandNormal<4,1>()),
                    inv2 = inv1.inverse(),
                    inv3 = inv1 * inv2;
    FGASSERT(fgApproxEqual(inv3.real(),1.0,10));
}
