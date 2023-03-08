//
// Copyright (c) 2022 Singular Inversions Inc. (facegen.com)
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

MatUT2D             cCholesky(MatS2D s)
{
    FGASSERT(s.m00 > 0.0);
    double          r00 = sqrt(s.m00),
                    r01 = s.m01 / r00,
                    tmp = s.m11 - sqr(r01);
    FGASSERT(tmp > 0.0);
    return {r00,sqrt(tmp),r01};
}

MatUT3D             cCholesky(MatS3D S)
{
    MatUT3D        U;
    FGASSERT(S.diag[0] > 0.0);
    U.m[0] = sqrt(S.diag[0]);
    U.m[1] = S.offd[0] / U.m[0];
    U.m[2] = S.offd[1] / U.m[0];
    double          tmp0 = S.diag[1] - sqr(S.offd[0])/S.diag[0];
    FGASSERT(tmp0 > 0.0);
    U.m[3] = sqrt(tmp0);
    double          tmp1 = S.offd[2] - S.offd[1]*S.offd[0]/S.diag[0];
    U.m[4] = tmp1 / U.m[3];
    double          tmp2 = S.diag[2] - sqr(S.offd[1])/S.diag[0] - sqr(tmp1)/tmp0;
    FGASSERT(tmp2 > 0.0);
    U.m[5] = sqrt(tmp2);
    return U;
}

void                testCholesky(CLArgs const &)
{
    randSeedRepeatable();
    size_t          N = 1000;
    for (size_t ii=0; ii<N; ++ii) {
        MatS2D          spds = MatS2D::randSpd(3.0);
        MatUT2D         ch = cCholesky(spds);
        MatS2D          lu = ch.luProduct();
        FGASSERT(isApproxEqual(spds.m00,lu.m00,epsBits(30)));
        FGASSERT(isApproxEqual(spds.m01,lu.m01,epsBits(30)));
        FGASSERT(isApproxEqual(spds.m11,lu.m11,epsBits(30)));
    }
    for (size_t ii=0; ii<N; ++ii) {
        MatS3D          spds = MatS3D::randSpd(3.0);
        MatUT3D         ch = cCholesky(spds);
        MatS3D          lu = ch.luProduct();
        FGASSERT(isApproxEqual(spds.diag,lu.diag,epsBits(30)));
        FGASSERT(isApproxEqual(spds.offd,lu.offd,epsBits(30)));
    }
}

Vec2D               solveLinear(MatS2D fr,Vec2D b)
{
    double              det = fr.m00 * fr.m11 - sqr(fr.m01),    // will be non-zero for full rank matrix
                        n0 = fr.m11 * b[0] - fr.m01 * b[1],
                        n1 = fr.m00 * b[1] - fr.m01 * b[0];
    return {n0/det,n1/det};
}
void                testSolveS2(CLArgs const &)
{
    for (size_t ii=0; ii<100; ++ii) {
        // TODO: currently just tests SPD but 'solve' should work for all full rank:
        double              lnEigRat = randUniform(0,-log(epsBits(20))),    // ln eigvalue ratio within limits
                            ev0 = exp(randNormal()),
                            ev1 = ev0 * exp(-lnEigRat),
                            theta = randUniform(-pi(),pi()),
                            c = cos(theta),
                            s = sin(theta);
        Mat22D              D {ev0, 0, 0, ev1},
                            R {c, s, -s, c},
                            M = R * D * R.transpose();
        MatS2D              S {M.rc(0,0), M.rc(1,1), M.rc(0,1)};
        Vec2D               x = Vec2D::randNormal(),
                            b = M * x,
                            t = solveLinear(S,b);
        FGASSERT(isApproxEqualPrec(t,x,20));
    }
}

Vec3D               solveLinear(MatS3D A,Vec3D b)
{
    MatUT3D             cd = cCholesky(A);
    MatUT3D             I = cd.inverse();
    Vec3D               c = I.tranposeMul(b);
    return I * c;
}

MatD                RsmEigs::rsm() const
{
    return mulTr(scaleColumns(vecs,vals),vecs);
}

ostream &           operator<<(ostream & os,RsmEigs const & rsm)
{
    return os << rsm.vals << rsm.vecs;
}

namespace {

MatD                randSymmMatrix(uint dim)
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

typedef function<void(MatD const &,Doubles &,MatD &)>  FnSolve;

void                testSymmEigenProblem(uint dim,FnSolve solve,bool print)
{
    // Random symmetric matrix, uniform distribution:
    MatD           mat = randSymmMatrix(dim);
    Doubles              eigVals;
    MatD           eigVecs;
    Timer             timer;
    solve(mat,eigVals,eigVecs);
    size_t              time = timer.elapsedMilliseconds();
    // What is the pre-diagonalization speedup:
    MatD           innerHess = eigVecs.transpose() * mat * eigVecs,
                        innerEigvecs;
    Doubles              innerEigVals;
    timer.start();
    solve(innerHess,innerEigVals,innerEigvecs);
    size_t              timeInner = timer.elapsedMilliseconds();
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
    double          tol = lims<double>::epsilon() * sqrt(dim) * 2.0;
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

void                testAsymEigs(CLArgs const &)
{
    Mat33D          mat = matRotateAxis(1.0,normalize(Vec3D{1,1,1}));
    EigsC<3>        eigs = cEigs(mat);
    // We have to test against the reconstructed matrix as the order of eigvals/vecs will
    // differ on different platforms (eg. gcc):
    Mat33D          regress = cReal(eigs.vecs * cDiagMat(eigs.vals) * cHermitian(eigs.vecs));
    double          residual = cRms(mat - regress);
    FGASSERT(residual < 0.0000001);
    fgout << eigs
        << fgnl << "Residual: " << residual;
}

void                testSymmEigenAuto(CLArgs const &)
{
    randSeedRepeatable();
    testSymmEigenProblem(10,cEigsRsm_,true);
    testSymmEigenProblem(30,cEigsRsm_,false);
#ifndef _DEBUG
    testSymmEigenProblem(100,cEigsRsm_,false);
    testSymmEigenProblem(300,cEigsRsm_,false);
#endif
}

// Random RSM timing tests (21.04, i9-9900K 3.6GHz)
// Dim/1000     eigvecs    eigval-only     mul
//      1       1.3s            14%
//      2       9.3s            13%         0.8s
//      3
//      4      80.3s            15%         6.7s
//      5       2.6m            15%        13.1s
//      6       4.5m                       22.5s
//      7       7.1m                       36s
//
void                testEigsRsmTime(CLArgs const & args)
{
    if (isAutomated(args))
        return;
    Syntax              syn(args,"<size>");
    // Random symmetric matrix, uniform distribution:
    size_t              dim = syn.nextAs<size_t>();
    randSeedRepeatable();
    MatD                mat = randSymmMatrix(uint(dim));
    Doubles             eigVals;
    MatD                eigVecs;
    Timer               timer;
    cEigsRsm_(mat,eigVals,eigVecs);
    double              time0 = timer.elapsedSeconds();
    fgout << fgnl << "With eigenvectors: " << toPrettyTime(time0);
    MatD                eigValMat(dim,dim);
    eigValMat.setZero();
    for (uint ii=0; ii<dim; ii++)
        eigValMat.rc(ii,ii) = eigVals[ii];
    MatD                recon = eigVecs * eigValMat * eigVecs.transpose();
    double              residual = 0.0;
    for (uint ii=0; ii<dim; ii++)
        for (uint jj=ii; jj<dim; jj++)
            residual += sqr(recon.rc(ii,jj) - mat.rc(ii,jj));
    residual = sqrt(residual/double(dim*dim));      // root of mean value
    // RMS residual appears to go with the square root of the matrix dimension:
    double              tol = lims<double>::epsilon() * sqrt(dim) * 2.0;
    FGASSERT(residual < tol);
    Doubles             valsOnly;
    timer.start();
    valsOnly = cEigvalsRsm(mat);
    double              time1 = timer.elapsedSeconds();
    fgout
        << fgnl << "No eigenvectors: " << toPrettyTime(time1) << " " << toStrPercent(time1/time0)
        << fgnl << "RMS Residual: " << residual << " RR/tol " << residual/tol;
    FGASSERT(valsOnly == eigVals);
}

void                testSymmEigen(CLArgs const & args)
{
    Cmds      cmds;
    cmds.push_back(Cmd(testSymmEigenAuto,"auto","Automated tests"));
    cmds.push_back(Cmd(testEigsRsmTime,"time","Eigenvector timing test"));
    doMenu(args,cmds,true);
}

}

void                testMatrixSolver(CLArgs const & args)
{
    Cmds        cmds {
        {testCholesky,"chol","Cholesky 3x3 decomposition"},
        {testAsymEigs,"asym","Arbitrary real matrix eigensystem"},
        {testSymmEigen,"symm","Real symmetric matrix eigensystem"},
        {testSolveS2,"solveS2","Mx=b solver for M 2x2 symmetric"},
    };
    doMenu(args,cmds,true);
}

}
