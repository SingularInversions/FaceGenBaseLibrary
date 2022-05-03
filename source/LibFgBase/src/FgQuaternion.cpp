//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgQuaternion.hpp"
#include "FgApproxEqual.hpp"
#include "FgRandom.hpp"
#include "FgMain.hpp"
#include "FgBounds.hpp"

using namespace std;

namespace Fg {

double              tanDeltaMag(QuaternionD const & lhs,QuaternionD const & rhs)
{
    Vec4D    lv = lhs.asVec4(),
                rv = rhs.asVec4();
    // Ensure we compare appropriate relative signs of the vectors:
    if (cDot(lv,rv) < 0.0)
        rv *= -1.0;
    return cMag(rv-lv);
}

QuaternionD         interpolate(QuaternionD q0, QuaternionD q1,double val)
{
    Vec4D       v0 = q0.asVec4(),
                v1 = q1.asVec4();
    double      dot = cDot(v0,v1);
    // Get closest representation from projective space. The degenerate case is when the dot
    // product is exactly zero so the inequality choice breaks that symmetry:
    if (dot < 0.0)
        v1 *= -1.0;
    // Just lerp then normalize - not a real exponential map but a passable approx:
    Vec4D       vi = v0 * (1.0-val) + v1 * val;
    return QuaternionD {vi}; // normalizes
}

bool                isApproxEqual(QuaternionD const & l,QuaternionD const & r,double prec)
{
    return (
        // must check the 2 equivalent representations:
        (isApproxEqual(l.real,r.real,prec) && isApproxEqual(l.imag,r.imag,prec)) ||
        (isApproxEqual(l.real,-r.real,prec) && isApproxEqual(l.imag,-r.imag,prec))
    );
}

void                testQuaternion(CLArgs const &)
{
    randSeedRepeatable();
    double constexpr    prec = lims<double>::epsilon()*8;
    QuaternionD const   id;
    // Axis rotations:
    for (size_t ii=0; ii<5; ++ii) {
        double          r = randNormal();
        Mat33D          qx = cRotateX(r).asMatrix(),
                        mx = matRotateX(r);
        FGASSERT(isApproxEqual(qx,mx,prec));
        Mat33D          qy = cRotateY(r).asMatrix(),
                        my = matRotateY(r);
        FGASSERT(isApproxEqual(qy,my,prec));
        Mat33D          qz = cRotateZ(r).asMatrix(),
                        mz = matRotateZ(r);
        FGASSERT(isApproxEqual(qz,mz,prec));
    }
    // asMatrix is orthonormal:
    for (size_t ii=0; ii<5; ++ii) {
        QuaternionD     q = QuaternionD::rand();
        Mat33D          m = q.asMatrix(),
                        del = m * m.transpose() - Mat33D::identity();
        FGASSERT(cMax(mapAbs(del.m)) < lims<double>::epsilon()*8);
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
        FGASSERT(cMax(mapAbs(del.m)) < lims<double>::epsilon()*8);
    }
    // Inverse:
    for (size_t ii=0; ii<5; ++ii) {
        QuaternionD     q0 = QuaternionD::rand(),
                        q1 = q0.inverse(),
                        q2 = q1 * q0;
        FGASSERT(sqrt(tanDeltaMag(q2,id)) < lims<double>::epsilon()*8);
    }
    // multiplication keeps values normalized:
    for (size_t ii=0; ii<10; ++ii) {
        QuaternionD         q0 = QuaternionD::rand(),
                            q1 = QuaternionD::rand(),
                            q2 = q0 * q1;
        FGASSERT(isApproxEqual(q2.mag(),1.0,prec));
    }
}

}
