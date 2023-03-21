//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgAffine.hpp"

using namespace std;

namespace Fg {

Affine3D
solveAffine(Vec3Ds const & base,Vec3Ds const & targ)
{
    size_t          V = base.size();
    FGASSERT(targ.size() == V);
    FGASSERT(V > 3);
    // shifting the origin to the base mean before least squares minimization uncouples the
    // translation from the linear transform, making for an easy solution:
    Vec3D           bmean = cMean(base),
                    tmean = cMean(targ);
    Mat33D          bb {0},
                    tb {0};
    for (size_t vv=0; vv<V; ++vv) {
        Vec3D           b = base[vv]-bmean,
                        t = targ[vv]-tmean;
        bb += b * b.transpose();
        tb += t * b.transpose();
    }
    return Affine3D{tmean} * Affine3D{tb*cInverse(bb)} * Affine3D{-bmean};
}

}

// */
