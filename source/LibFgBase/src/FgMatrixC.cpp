//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     June 27, 2014
//

#include "stdafx.h"

#include "jama_lu.h"
#include "FgMatrixC.hpp"
#include "FgSyntax.hpp"
#include "FgMath.hpp"

using namespace std;

// Gaussian elimination can be very simply explicit in this case:
FgValidVal<FgVect2F>
fgSolve(FgMat22F A,FgVect2F b)
{
    FgValidVal<FgVect2F>    ret;
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

FgValidVal<FgVect3D>
fgSolve(FgMat33D A,FgVect3D b)
{
    TNT::Array2D<double>    a(3,3,A.m);
    JAMA::LU<double>        lu(a);
    if (lu.isNonsingular()) {
        TNT::Array1D<double>    x = lu.solve(TNT::Array1D<double>(3,b.m));
        return FgValidVal<FgVect3D>(FgVect3D(x[0],x[1],x[2]));
    }
    return FgValidVal<FgVect3D>();
}

FgValidVal<FgVect3F>
fgSolve(FgMat33F A,FgVect3F b)
{
    return fgSolve(FgMat33D(A),FgVect3D(b)).cast<FgVect3F>();
}

FgValidVal<FgVect4D>
fgSolve(FgMat44D A,FgVect4D b)
{
    TNT::Array2D<double>    a(4,4,A.m);
    JAMA::LU<double>        lu(a);
    if (lu.isNonsingular()) {
        TNT::Array1D<double>    x = lu.solve(TNT::Array1D<double>(4,b.m));
        return FgValidVal<FgVect4D>(FgVect4D(x[0],x[1],x[2],x[3]));
    }
    return FgValidVal<FgVect4D>();
}

FgValidVal<FgVect4F>
fgSolve(FgMat44F A,FgVect4F b)
{
    return fgSolve(FgMat44D(A),FgVect4D(b)).cast<FgVect4F>();
}

template<uint size>
static void
testInverse()
{
    FgMatrixC<double,size,size> a,b;
    do
        a = fgMatRandNormal<size,size>();
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
        FgVect3D        axis = fgMatRandNormal<3,1>();
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
