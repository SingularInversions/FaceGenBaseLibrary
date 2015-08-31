//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     June 21, 2010
//

#include "stdafx.h"

#include "FgApproxFunc.hpp"
#include "FgRandom.hpp"
#include "FgMath.hpp"
#include "FgMain.hpp"

using namespace std;
using namespace fgMath;

struct  Sine
{
    double
    operator()(double x)
    {return std::sin(x); }
};

void
fgApproxFuncTest(const FgArgs &)
{
    const double        accuracy = 0.0001;
    for (uint ii=0; ii<10; ++ii)
    {
        double          base = fgRandUniform(-fgPi(),fgPi()),
                        length = fgRandUniform(0.5,1.5) * fgPi();
        Sine            sine;
        FgApproxFunc<double>    af(sine,base,base+length,256);
        double          xx,delta;
        for (uint jj=0; jj<1000; ++jj)
        {
            // Test interpolation:
            xx = fgRandUniform(base,base+length);
            delta = std::abs(sine(xx) - af(xx));
            FGASSERT(delta < accuracy);
        }

        // Test clamping:
        xx = base - 0.001;
        delta = std::abs(sine(base) - af(xx));
        FGASSERT(delta < accuracy);
        xx = base - 1.001;
        delta = std::abs(sine(base) - af(xx));
        FGASSERT(delta < accuracy);
        xx = base + length + 0.001;
        delta = std::abs(sine(base+length) - af(xx));
        FGASSERT(delta < accuracy);
        xx = base + length + 1.001;
        delta = std::abs(sine(base+length) - af(xx));
        FGASSERT(delta < accuracy);
    }
}
