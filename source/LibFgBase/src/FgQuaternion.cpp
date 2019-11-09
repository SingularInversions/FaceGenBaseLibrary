//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

//

#include "stdafx.h"

#include "FgQuaternion.hpp"
#include "FgApproxEqual.hpp"
#include "FgRandom.hpp"
#include "FgMain.hpp"

using namespace std;

namespace Fg {

double
fgTanDeltaMag(const QuaternionD & lhs,const QuaternionD & rhs)
{
    Vec4D    lv = lhs.asVect4(),
                rv = rhs.asVect4();
    // Ensure we compare appropriate relative signs of the vectors:
    if (dotProd(lv,rv) < 0.0)
        rv *= -1.0;
    return cMag(rv-lv);
}

void
fgQuaternionTest(const CLArgs &)
{
    QuaternionD           id;
    // Axis rotations:
    randSeedRepeatable();
    for (size_t ii=0; ii<5; ++ii) {
        double          r = randNormal();
        Mat33D          qx = fgRotateX(r).asMatrix(),
                        mx = matRotateX(r),
                        dx = qx - mx;
        FGASSERT(maxEl(mapAbs(dx.m)) < numeric_limits<double>::epsilon()*8);
        Mat33D          qy = fgRotateY(r).asMatrix(),
                        my = matRotateY(r),
                        dy = qy - my;
        FGASSERT(maxEl(mapAbs(dy.m)) < numeric_limits<double>::epsilon()*8);
        Mat33D          qz = fgRotateZ(r).asMatrix(),
                        mz = matRotateZ(r),
                        dz = qz - mz;
        FGASSERT(maxEl(mapAbs(dz.m)) < numeric_limits<double>::epsilon()*8);
    }
    // asMatrix is orthonormal:
    for (size_t ii=0; ii<5; ++ii) {
        QuaternionD     q = QuaternionD::rand();
        Mat33D          m = q.asMatrix(),
                        del = m * m.transpose() - Mat33D::identity();
        FGASSERT(maxEl(mapAbs(del.m)) < numeric_limits<double>::epsilon()*8);
    }
    // Composition:
    for (size_t ii=0; ii<5; ++ii) {
        QuaternionD     q0 = QuaternionD::rand(),
                        q1 = QuaternionD::rand(),
                        q2 = q1 * q0;
        Mat33D          m0 = q0.asMatrix(),
                        m1 = q1.asMatrix(),
                        m2 = m1 * m0,
                        m2q = q2.asMatrix(),
                        del = m2q-m2;
        FGASSERT(maxEl(mapAbs(del.m)) < numeric_limits<double>::epsilon()*8);
    }
    // Inverse:
    for (size_t ii=0; ii<5; ++ii) {
        QuaternionD     q0 = QuaternionD::rand(),
                        q1 = q0.inverse(),
                        q2 = q1 * q0;
        FGASSERT(sqrt(fgTanDeltaMag(q2,id)) < numeric_limits<double>::epsilon()*8);
    }
}

}
