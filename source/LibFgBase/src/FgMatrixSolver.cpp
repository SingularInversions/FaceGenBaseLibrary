//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
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

MatUT3D
cCholesky(MatS3D S)
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

void
testCholesky(CLArgs const &)
{
    randSeedRepeatable();
    double          resid1 = 0.0;
    size_t          N = 1000;
    for (size_t ii=0; ii<N; ++ii) {
        MatS3D          spds = randMatSpd3D(3.0);
        MatUT3D         ch = cCholesky(spds);
        MatS3D          lu = ch.luProduct();
        Mat33D          out = lu.asMatC(),
                        spd = spds.asMatC(),
                        del = out - spd;
        resid1 += cMag(del.m) / cMag(spd.m);
        FGASSERT(isApproxEqualPrec(out,spd));
    }
    fgout << fgnl << "Cholesky unsafe RMS residual: " << sqrt(resid1/N);
}

Vec3D
solve(MatS3D A,Vec3D b)
{
    MatUT3D            cd = cCholesky(A);
    MatUT3D            I = cd.inverse();
    Vec3D               c = I.tranposeMul(b);
    return I * c;
}

MatD
RsmEigs::rsm() const
{
    return mulTr(scaleColumns(vecs,vals),vecs);
}

MatD
RsmEigs::mhlbsToWorld() const
{
    Doubles                 stdevs; stdevs.reserve(vals.size());
    for (double variance : vals) {
        FGASSERT(variance > 0.0);
        stdevs.push_back(sqrt(variance));
    }
    return scaleColumns(vecs,stdevs);      // vecs * D(stdevs)
}

MatD
RsmEigs::inverse() const
{
    auto                invert = [](double v)
    {
        FGASSERT(v != 0.0);
        return 1.0/v;
    };
    Doubles             invVals = mapCall(vals,invert);
    return mulTr(scaleColumns(vecs,invVals),vecs);
}

ostream &
operator<<(ostream & os,RsmEigs const & rsm)
{
    return os << rsm.vals << rsm.vecs;
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

typedef function<void(MatD const &,Doubles &,MatD &)>  FnSolve;

void
testSymmEigenProblem(uint dim,FnSolve solve,bool print)
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

void
testSymmEigenAuto(CLArgs const &)
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
void
testEigsRsmTime(CLArgs const & args)
{
    if (isAutomatedTest(args))
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
    double              tol = epsilonD() * sqrt(dim) * 2.0;
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

void
testSymmEigen(CLArgs const & args)
{
    Cmds      cmds;
    cmds.push_back(Cmd(testSymmEigenAuto,"auto","Automated tests"));
    cmds.push_back(Cmd(testEigsRsmTime,"time","Eigenvector timing test"));
    doMenu(args,cmds,true);
}

}

void
testMatrixSolver(CLArgs const & args)
{
    Cmds        cmds {
        {testCholesky,"chol","Cholesky 3x3 decomposition"},
        {testAsymEigs,"asym","Arbitrary real matrix eigensystem"},
        {testSymmEigen,"symm","Real symmetric matrix eigensystem"}
    };
    doMenu(args,cmds,true);
}

}
