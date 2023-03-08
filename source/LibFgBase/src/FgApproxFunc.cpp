//
// Copyright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgApproxFunc.hpp"
#include "FgRandom.hpp"
#include "FgMath.hpp"
#include "FgMain.hpp"

using namespace std;

namespace Fg {

struct  Sine
{
    double
    operator()(double x)
    {return std::sin(x); }
};

void
fgApproxFuncTest(CLArgs const &)
{
    double const        accuracy = 0.0001;
    for (uint ii=0; ii<10; ++ii)
    {
        double          base = randUniform(-pi(),pi()),
                        len = randUniform(0.5,1.5) * pi();
        Sine            sine;
        FgApproxFunc<double>    af(sine,base,base+len,256);
        double          xx,delta;
        for (uint jj=0; jj<1000; ++jj)
        {
            // Test interpolation:
            xx = randUniform(base,base+len);
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
        xx = base + len + 0.001;
        delta = std::abs(sine(base+len) - af(xx));
        FGASSERT(delta < accuracy);
        xx = base + len + 1.001;
        delta = std::abs(sine(base+len) - af(xx));
        FGASSERT(delta < accuracy);
    }
}

}
