//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     June 27, 2014
//

#include "stdafx.h"

#include "FgMatrixC.hpp"
#include "FgSyntax.hpp"
#include "FgMath.hpp"

#define EIGEN_MPL2_ONLY     // Only use permissive licensed source files from Eigen
#include "../../LibTpEigen/Eigen/Dense"
#include "../../LibTpEigen/Eigen/Core"

using namespace std;

FgVerts
fgVertsRandNormal(size_t num,float scale)
{
    FgVerts         ret;
    ret.reserve(num);
    for (size_t ii=0; ii<num; ++ii)
        ret.push_back(fgMatRandNrm<float,3,1>()*scale);
    return ret;
}

// Gaussian elimination can be very simply explicit in this case:
FgOpt<FgVect2F>
fgSolve(FgMat22F A,FgVect2F b)
{
    FgOpt<FgVect2F>    ret;
    float                   a0 = A[0]*A[3],
                            a1 = A[1]*A[2],
                            a0s = a0*a0,
                            a1s = a1*a1;
    if (a0s > a1s) {
        float           r1 = A[1] / A[3],
                        a11 = A[0] - r1 * A[2],
                        x = (b[0] - r1 * b[1]) / a11,
                        y = (b[1] - A[2] * x) / A[3];
        ret = FgVect2F(x,y);
    }
    else if (a0s < a1s) {
        float           r1 = A[0] / A[2],
                        a12 = A[1] - r1 * A[3],
                        y = (b[0] - r1 * b[1]) / a12,
                        x = (b[1] - A[3] * y) / A[2];
        ret = FgVect2F(x,y);
    }
    return ret;
}

FgOpt<FgVect3D>
fgSolve(FgMat33D A,FgVect3D b)
{
    Eigen::Matrix3d         mat;
    Eigen::Vector3d         vec;
    for (uint rr=0; rr<3; ++rr)
        for (uint cc=0; cc<3; ++cc)
            mat(rr,cc) = A.rc(rr,cc);
    for (uint rr=0; rr<3; ++rr)
        vec(rr) = b[rr];
    FgOpt<FgVect3D>         ret;
    // There are many alternatives to this in Eigen: ParialPivLU, FullPivLU, HouseholderQR etc.
    auto                    qr = mat.colPivHouseholderQr();
    if (qr.isInvertible()) {
        Eigen::Vector3d     sol = qr.solve(vec);
        ret = FgVect3D(sol(0),sol(1),sol(2));
    }
    return ret;
}

FgOpt<FgVect4D>
fgSolve(FgMat44D A,FgVect4D b)
{
    Eigen::Matrix4d         mat;
    Eigen::Vector4d         vec;
    for (uint rr=0; rr<4; ++rr)
        for (uint cc=0; cc<4; ++cc)
            mat(rr,cc) = A.rc(rr,cc);
    for (uint rr=0; rr<4; ++rr)
        vec(rr) = b[rr];
    FgOpt<FgVect4D>         ret;
    // There are many alternatives to this in Eigen: ParialPivLU, FullPivLU, HouseholderQR etc.
    auto                    qr = mat.colPivHouseholderQr();
    if (qr.isInvertible()) {
        Eigen::Vector4d     sol = qr.solve(vec);
        ret = FgVect4D(sol(0),sol(1),sol(2),sol(3));
    }
    return ret;
}

template<uint size>
static void
testInverse()
{
    FgMatrixC<double,size,size> a,b;
    do
        a = fgMatRandNrm<double,size,size>();
    while
        (fgDeterminant(a) < 0.01);
    b = fgMatInverse(a);
    a = (a * b + b * a) * 0.5;  // cancel errors from near-singularities in matrix
    b.setIdentity();
    double          res = (a-b).length();
    FGASSERT(res < (10.0 * size * size * numeric_limits<double>::epsilon()));
}

static void     testFgMatRotateAxis()
{
    fgRandSeedRepeatable();
    for (uint ii=0; ii<100; ii++)
    {
        double          angle = fgRandUniform(-fgPi(),fgPi());
        FgVect3D        axis = fgVecRandNrm<3>();
        axis /= axis.length();
        FgMat33D     mat = fgMatRotateAxis(angle,axis);
        double          err = (mat * mat.transpose() - FgMat33D::identity()).length(),
                        err2 = (mat * axis - axis).length();

        FGASSERT(err < (std::numeric_limits<double>::epsilon() * 10.0));
        FGASSERT(err2 < (std::numeric_limits<double>::epsilon() * 10.0));
        FGASSERT(fgDeterminant(mat) > 0.0);      // Ensure SO(3) not just O(3)
    }
}

void
fgMatrixCTest(const FgArgs &)
{
    fgRandSeedRepeatable();
    for (size_t ii=0; ii<10; ++ii)
    {
        testInverse<2>();
        testInverse<3>();
    }
    testFgMatRotateAxis();
}

FgMat32D
fgTanSphere(FgVect3D v)
{
    // Find permutation that sorts 'v' smallest to largest:
    FgVect3UI           p(0,1,2);
    FgVect3D            m = fgMapSqr(v);
    if (m[0] > m[1])
        std::swap(p[0],p[1]);
    if (m[p[1]] > m[p[2]])
        std::swap(p[1],p[2]);
    if (m[p[0]] > m[p[1]])
        std::swap(p[0],p[1]);
    // Gram-Schmidt starting with least co-linear axes:
    FgVect3D        r0(0),
                    vn = fgNormalize(v);
    r0[p[0]] = 1.0;
    r0 -= vn * fgDot(vn,r0);
    r0 /= r0.length();
    FgVect3D        r1 = fgCrossProduct(vn,r0);
    return fgConcatHoriz(r0,r1);
}
