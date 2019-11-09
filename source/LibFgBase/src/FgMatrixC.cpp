//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

//

#include "stdafx.h"

#include "FgMatrixC.hpp"
#include "FgSyntax.hpp"
#include "FgMath.hpp"
#include "FgRandom.hpp"

#ifdef _MSC_VER
    #pragma warning(push,0)     // Eigen triggers lots of warnings
#endif

#define EIGEN_MPL2_ONLY         // Only use permissive licensed source files from Eigen
#include "Eigen/Dense"
#include "Eigen/Core"

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

using namespace std;

namespace Fg {

Vec3Fs
fgVertsRandNormal(size_t num,float scale)
{
    Vec3Fs         ret;
    ret.reserve(num);
    for (size_t ii=0; ii<num; ++ii)
        ret.push_back(Vec3F::randNormal(scale));
    return ret;
}

// Gaussian elimination can be very simply explicit in this case:
Opt<Vec2F>
fgSolve(Mat22F A,Vec2F b)
{
    Opt<Vec2F>    ret;
    float                   a0 = A[0]*A[3],
                            a1 = A[1]*A[2],
                            a0s = a0*a0,
                            a1s = a1*a1;
    if (a0s > a1s) {
        float           r1 = A[1] / A[3],
                        a11 = A[0] - r1 * A[2],
                        x = (b[0] - r1 * b[1]) / a11,
                        y = (b[1] - A[2] * x) / A[3];
        ret = Vec2F(x,y);
    }
    else if (a0s < a1s) {
        float           r1 = A[0] / A[2],
                        a12 = A[1] - r1 * A[3],
                        y = (b[0] - r1 * b[1]) / a12,
                        x = (b[1] - A[3] * y) / A[2];
        ret = Vec2F(x,y);
    }
    return ret;
}

Opt<Vec3D>
fgSolve(Mat33D A,Vec3D b)
{
    Eigen::Matrix3d         mat;
    Eigen::Vector3d         vec;
    for (uint rr=0; rr<3; ++rr)
        for (uint cc=0; cc<3; ++cc)
            mat(rr,cc) = A.rc(rr,cc);
    for (uint rr=0; rr<3; ++rr)
        vec(rr) = b[rr];
    Opt<Vec3D>         ret;
    // There are many alternatives to this in Eigen: ParialPivLU, FullPivLU, HouseholderQR etc.
    auto                    qr = mat.colPivHouseholderQr();
    if (qr.isInvertible()) {
        Eigen::Vector3d     sol = qr.solve(vec);
        ret = Vec3D(sol(0),sol(1),sol(2));
    }
    return ret;
}

Opt<Vec4D>
fgSolve(Mat44D A,Vec4D b)
{
    Eigen::Matrix4d         mat;
    Eigen::Vector4d         vec;
    for (uint rr=0; rr<4; ++rr)
        for (uint cc=0; cc<4; ++cc)
            mat(rr,cc) = A.rc(rr,cc);
    for (uint rr=0; rr<4; ++rr)
        vec(rr) = b[rr];
    Opt<Vec4D>         ret;
    // There are many alternatives to this in Eigen: ParialPivLU, FullPivLU, HouseholderQR etc.
    auto                    qr = mat.colPivHouseholderQr();
    if (qr.isInvertible()) {
        Eigen::Vector4d     sol = qr.solve(vec);
        ret = Vec4D(sol(0),sol(1),sol(2),sol(3));
    }
    return ret;
}

template<uint size>
static void
testInverse()
{
    Mat<double,size,size> a,b;
    do
        a = Mat<double,size,size>::randNormal();
    while
        (determinant(a) < 0.01);
    b = fgMatInverse(a);
    a = (a * b + b * a) * 0.5;  // cancel errors from near-singularities in matrix
    b.setIdentity();
    double          res = (a-b).len();
    FGASSERT(res < (10.0 * size * size * numeric_limits<double>::epsilon()));
}

static void     testFgMatRotateAxis()
{
    randSeedRepeatable();
    for (uint ii=0; ii<100; ii++)
    {
        double          angle = randUniform(-fgPi(),fgPi());
        Vec3D        axis = Vec3D::randNormal();
        axis /= axis.len();
        Mat33D     mat = matRotateAxis(angle,axis);
        double          err = (mat * mat.transpose() - Mat33D::identity()).len(),
                        err2 = (mat * axis - axis).len();

        FGASSERT(err < (std::numeric_limits<double>::epsilon() * 10.0));
        FGASSERT(err2 < (std::numeric_limits<double>::epsilon() * 10.0));
        FGASSERT(determinant(mat) > 0.0);      // Ensure SO(3) not just O(3)
    }
}

void
fgMatrixCTest(const CLArgs &)
{
    randSeedRepeatable();
    for (size_t ii=0; ii<10; ++ii)
    {
        testInverse<2>();
        testInverse<3>();
    }
    testFgMatRotateAxis();
}

Mat32D
fgTanSphere(Vec3D v)
{
    // Find permutation that sorts 'v' smallest to largest:
    Vec3UI           p(0,1,2);
    Vec3D            m = fgMapSqr(v);
    if (m[0] > m[1])
        std::swap(p[0],p[1]);
    if (m[p[1]] > m[p[2]])
        std::swap(p[1],p[2]);
    if (m[p[0]] > m[p[1]])
        std::swap(p[0],p[1]);
    // Gram-Schmidt starting with least co-linear axes:
    Vec3D        r0(0),
                    vn = fgNormalize(v);
    r0[p[0]] = 1.0;
    r0 -= vn * dotProd(vn,r0);
    r0 /= r0.len();
    Vec3D        r1 = crossProduct(vn,r0);
    return fgJoinHoriz(r0,r1);
}

}
