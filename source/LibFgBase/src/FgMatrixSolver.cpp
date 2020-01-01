//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
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

namespace Fg {

MatD
FgEigsRsm::matrix() const
{
    MatD       rhs = vecs.transpose();
    for (size_t rr=0; rr<rhs.nrows; ++rr)
        for (size_t cc=0; cc<rhs.ncols; ++cc)
            rhs.rc(rr,cc) *= vals[cc];
    return vecs * rhs;
}

namespace {

MatD
randSymmMatrix(uint dim)
{
    MatD  mat(dim,dim);
    for (uint ii=0; ii<dim; ii++) {
        for (uint jj=ii; jj<dim; jj++) {
            mat.rc(ii,jj) = randUniform();
            mat.rc(jj,ii) = mat.rc(ii,jj);
        }
    }
    return mat;
}

typedef function<void(const MatD &,Doubles &,MatD &)>  FnSolve;

void
testSymmEigenProblem(uint dim,FnSolve solve,bool print)
{
    // Random symmetric matrix, uniform distribution:
    MatD           mat = randSymmMatrix(dim);
    Doubles              eigVals;
    MatD           eigVecs;
    FgTimer             timer;
    solve(mat,eigVals,eigVecs);
    size_t              time = timer.readMs();
    // What is the pre-diagonalization speedup:
    MatD           innerHess = eigVecs.transpose() * mat * eigVecs,
                        innerEigvecs;
    Doubles              innerEigVals;
    timer.start();
    solve(innerHess,innerEigVals,innerEigvecs);
    size_t              timeInner = timer.readMs();
    MatD           eigValMat(dim,dim);
    eigValMat.setZero();
    for (uint ii=0; ii<dim; ii++)
        eigValMat.rc(ii,ii) = eigVals[ii];
    MatD       recon = eigVecs * eigValMat * eigVecs.transpose();
    double          residual = 0.0;
    for (uint ii=0; ii<dim; ii++)
        for (uint jj=ii; jj<dim; jj++)
            residual += sqr(recon.rc(ii,jj) - mat.rc(ii,jj));
    residual = sqrt(residual/double(dim*dim));      // root of mean value
    // RMS residual appears to go with the square root of the matrix dimension:
    double          tol = epsilonD() * sqrt(dim) * 2.0;
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
testAsymEigs(CLArgs const &)
{
    Mat33D        mat = matRotateAxis(1.0,Vec3D(1,1,1));
    FgEigsC<3>      eigs = fgEigs(mat);
    // We have to test against the reconstructed matrix as the order of eigvals/vecs will
    // differ on different platforms (eg. gcc):
    Mat33D        regress = fgReal(eigs.vecs * fgDiagonal(eigs.vals) * fgHermitian(eigs.vecs));
    double          residual = cRms(mat - regress);
    FGASSERT(residual < 0.0000001);
    fgout << eigs
        << fgnl << "Residual: " << residual;
}

void
testSymmEigenAuto(CLArgs const &)
{
    randSeedRepeatable();
    testSymmEigenProblem(10,fgEigsRsm_,true);
    testSymmEigenProblem(30,fgEigsRsm_,false);
#ifndef _DEBUG
    testSymmEigenProblem(100,fgEigsRsm_,false);
    testSymmEigenProblem(300,fgEigsRsm_,false);
#endif
}

void
testSymmEigenTime(CLArgs const & args)
{
    if (fgAutomatedTest(args))
        return;
    Syntax            syn(args,"<size>");
    // Random symmetric matrix, uniform distribution:
    size_t              dim = syn.nextAs<size_t>();
    randSeedRepeatable();
    MatD           mat = randSymmMatrix(uint(dim));
    Doubles              eigVals;
    MatD           eigVecs;
    FgTimer             timer;
    fgEigsRsm_(mat,eigVals,eigVecs);
    size_t              time = timer.readMs();
    MatD           eigValMat(dim,dim);
    eigValMat.setZero();
    for (uint ii=0; ii<dim; ii++)
        eigValMat.rc(ii,ii) = eigVals[ii];
    MatD       recon = eigVecs * eigValMat * eigVecs.transpose();
    double          residual = 0.0;
    for (uint ii=0; ii<dim; ii++)
        for (uint jj=ii; jj<dim; jj++)
            residual += sqr(recon.rc(ii,jj) - mat.rc(ii,jj));
    residual = sqrt(residual/double(dim*dim));      // root of mean value
    // RMS residual appears to go with the square root of the matrix dimension:
    double          tol = epsilonD() * sqrt(dim) * 2.0;
    fgout << fgnl << "Dim: " << dim << fgpush
        << fgnl << "RMS Residual: " << residual << " RR/tol " << residual/tol
        << fgnl << "Time: " << time;
    fgout << fgpop;
    FGASSERT(residual < tol);
}

void
testSymmEigen(CLArgs const & args)
{
    Cmds      cmds;
    cmds.push_back(Cmd(testSymmEigenAuto,"auto","Automated tests"));
    cmds.push_back(Cmd(testSymmEigenTime,"time","Timing test"));
    doMenu(args,cmds,true);
}

}

void
fgMatrixSolverTest(CLArgs const & args)
{
    Cmds      cmds;
    cmds.push_back(Cmd(testAsymEigs,"asym","Arbitrary real matrix eigensystem"));
    cmds.push_back(Cmd(testSymmEigen,"symm","Real symmetric matrix eigensystem"));
    doMenu(args,cmds,true);
}

}
