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
#include "FgTime.hpp"
#include "FgCommand.hpp"
#include "FgApproxEqual.hpp"
#include "FgSyntax.hpp"

using namespace std;

FgMatrixD
FgEigsRsm::matrix() const
{
    FgMatrixD       rhs = vecs.transpose();
    for (size_t rr=0; rr<rhs.nrows; ++rr)
        for (size_t cc=0; cc<rhs.ncols; ++cc)
            rhs.rc(rr,cc) *= vals[cc];
    return vecs * rhs;
}

namespace {

FgMatrixD
randSymmMatrix(uint dim)
{
    FgMatrixD  mat(dim,dim);
    for (uint ii=0; ii<dim; ii++) {
        for (uint jj=ii; jj<dim; jj++) {
            mat.rc(ii,jj) = fgRand();
            mat.rc(jj,ii) = mat.rc(ii,jj);
        }
    }
    return mat;
}

typedef function<void(const FgMatrixD &,FgDbls &,FgMatrixD &)>  FnSolve;

void
testSymmEigenProblem(uint dim,FnSolve solve,bool print)
{
    // Random symmetric matrix, uniform distribution:
    FgMatrixD           mat = randSymmMatrix(dim);
    FgDbls              eigVals;
    FgMatrixD           eigVecs;
    FgTimer             timer;
    solve(mat,eigVals,eigVecs);
    size_t              time = timer.readMs();
    // What is the pre-diagonalization speedup:
    FgMatrixD           innerHess = eigVecs.transpose() * mat * eigVecs,
                        innerEigvecs;
    FgDbls              innerEigVals;
    timer.start();
    solve(innerHess,innerEigVals,innerEigvecs);
    size_t              timeInner = timer.readMs();
    FgMatrixD           eigValMat(dim,dim);
    eigValMat.setZero();
    for (uint ii=0; ii<dim; ii++)
        eigValMat.rc(ii,ii) = eigVals[ii];
    FgMatrixD       recon = eigVecs * eigValMat * eigVecs.transpose();
    double          residual = 0.0;
    for (uint ii=0; ii<dim; ii++)
        for (uint jj=ii; jj<dim; jj++)
            residual += fgSqr(recon.rc(ii,jj) - mat.rc(ii,jj));
    residual = sqrt(residual/double(dim*dim));      // root of mean value
    // RMS residual appears to go with the square root of the matrix dimension:
    double          tol = numeric_limits<double>::epsilon() * sqrt(dim) * 2.0;
    fgout << fgnl << "Dim: " << dim << fgpush
        << fgnl << "RMS Residual: " << residual << " RR/tol " << residual/tol
        << fgnl << "Time: " << time
        << fgnl << "Inner time: " << timeInner;
    if (print) {
        FGOUT1(mat);
        FGOUT1(innerHess);
        FGOUT1(eigVals);
        FGOUT1(eigVecs);
        FGOUT1(innerEigVals);
        FGOUT1(innerEigvecs);
        FGOUT1(eigVecs.transpose() * eigVecs);
    }
    fgout << fgpop;
    FGASSERT(residual < tol);
}

void
testAsymEigs(const FgArgs &)
{
    FgMat33D        mat = fgMatRotateAxis(1.0,FgVect3D(1,1,1));
    FgEigsC<3>      eigs = fgEigs(mat);
    // We have to test against the reconstructed matrix as the order of eigvals/vecs will
    // differ on different platforms (eg. gcc):
    FgMat33D        regress = fgReal(eigs.vecs * fgDiagonal(eigs.vals) * fgHermitian(eigs.vecs));
    double          residual = fgRms(mat - regress);
    FGASSERT(residual < 0.0000001);
    fgout << eigs
        << fgnl << "Residual: " << residual;
}

void
testSymmEigenAuto(const FgArgs &)
{
    fgRandSeedRepeatable();
    testSymmEigenProblem(10,fgEigsRsm_,true);
    testSymmEigenProblem(30,fgEigsRsm_,false);
#ifndef _DEBUG
    testSymmEigenProblem(100,fgEigsRsm_,false);
    testSymmEigenProblem(300,fgEigsRsm_,false);
#endif
}

void
testSymmEigenTime(const FgArgs & args)
{
    if (fgAutomatedTest(args))
        return;
    FgSyntax            syn(args,"<size>");
    // Random symmetric matrix, uniform distribution:
    size_t              dim = syn.nextAs<size_t>();
    fgRandSeedRepeatable();
    FgMatrixD           mat = randSymmMatrix(uint(dim));
    FgDbls              eigVals;
    FgMatrixD           eigVecs;
    FgTimer             timer;
    fgEigsRsm_(mat,eigVals,eigVecs);
    size_t              time = timer.readMs();
    FgMatrixD           eigValMat(dim,dim);
    eigValMat.setZero();
    for (uint ii=0; ii<dim; ii++)
        eigValMat.rc(ii,ii) = eigVals[ii];
    FgMatrixD       recon = eigVecs * eigValMat * eigVecs.transpose();
    double          residual = 0.0;
    for (uint ii=0; ii<dim; ii++)
        for (uint jj=ii; jj<dim; jj++)
            residual += fgSqr(recon.rc(ii,jj) - mat.rc(ii,jj));
    residual = sqrt(residual/double(dim*dim));      // root of mean value
    // RMS residual appears to go with the square root of the matrix dimension:
    double          tol = numeric_limits<double>::epsilon() * sqrt(dim) * 2.0;
    fgout << fgnl << "Dim: " << dim << fgpush
        << fgnl << "RMS Residual: " << residual << " RR/tol " << residual/tol
        << fgnl << "Time: " << time;
    fgout << fgpop;
    FGASSERT(residual < tol);
}

void
testSymmEigen(const FgArgs & args)
{
    FgCmds      cmds;
    cmds.push_back(FgCmd(testSymmEigenAuto,"auto","Automated tests"));
    cmds.push_back(FgCmd(testSymmEigenTime,"time","Timing test"));
    fgMenu(args,cmds,true);
}

}

void
fgMatrixSolverTest(const FgArgs & args)
{
    FgCmds      cmds;
    cmds.push_back(FgCmd(testAsymEigs,"asym","Arbitrary real matrix eigensystem"));
    cmds.push_back(FgCmd(testSymmEigen,"symm","Real symmetric matrix eigensystem"));
    fgMenu(args,cmds,true);
}
