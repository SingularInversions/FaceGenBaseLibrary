//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Feb 28, 2005
//

#include "stdafx.h"

#include "FgMath.hpp"
#include "FgMatrixSolver.hpp"
#include "FgRandom.hpp"
#include "FgMain.hpp"

using namespace std;

static
void
testSymmEigenProblem(uint dim)
{
    FgMatrixD  mat(dim,dim);
    for (uint ii=0; ii<dim; ii++) {
        for (uint jj=ii; jj<dim; jj++) {
            mat.elem(ii,jj) = fgRand();
            mat.elem(jj,ii) = mat.elem(ii,jj);
        }
    }
    FgMatrixD  eigVals,eigVecs;
    //clock_t     clk = clock();
    fgRealSymmEigs(mat,eigVals,eigVecs);
    //float       time = ((float)clock() - (float)clk) / (float)CLOCKS_PER_SEC;
    FgMatrixD  eigValMat(dim,dim);
    eigValMat.setZero();
    for (uint ii=0; ii<dim; ii++)
        eigValMat.elem(ii,ii) = eigVals[ii];
    FgMatrixD  recon = eigVecs * eigValMat * eigVecs.transpose();
    double      residual = 0.0;
    for (uint ii=0; ii<dim; ii++)
        for (uint jj=ii; jj<dim; jj++)
            residual += abs(recon.elem(ii,jj) - mat.elem(ii,jj));
    // Now we can't expect matrix diagonalization to be accurate to machine
    // precision. In fact, depending on how extreme the eigenvalues are, the precision
    // can be arbitrarily poor. Experiments with random matrices show that the 
    // average absolute error increases slightly faster than the SQUARE of the dimension.
    // We need a fudge factor of 5 by the time we get to 300x300.
    double      tol = (dim*dim) * 5.0 * numeric_limits<double>::epsilon();
    //HI_MOM4(dim,residual,tol,time);
    FGASSERT(residual < tol);
}

void
fgMatrixSolverTest(const FgArgs &)
{
    fgRandSeedRepeatable();
    testSymmEigenProblem(10);
    testSymmEigenProblem(30);       // Larger than this is too slow in debug compile.
//    testSymmEigenProblem(100);
//    testSymmEigenProblem(1000);
}
