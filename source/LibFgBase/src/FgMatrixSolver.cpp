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

#define EIGEN_MPL2_ONLY     // Only use permissive licensed source files from Eigen
#include "../../LibTpEigen/Eigen/Dense"
#include "../../LibTpEigen/Eigen/Core"

using namespace std;

using namespace Eigen;

void
fgEigsRsm_(const FgMatrixD & rsm,FgDbls & vals,FgMatrixD & vecs)
{
    size_t              dim = rsm.ncols;
    FGASSERT(rsm.nrows == dim);
    // Ensure exact symmetry and valid value and copy into Eigen format:
    MatrixXd            mat(dim,dim);
    for (size_t rr=0; rr<dim; ++rr) {
        for (size_t cc=rr; cc<dim; ++cc) {
            double          v = (rsm.rc(rr,cc) + rsm.rc(cc,rr)) * 0.5;
            FGASSERT(boost::math::isfinite(v));
            mat(rr,cc) = v;
            mat(cc,rr) = v;
        }
    }
    // Eigen runtime is more than 3x faster than equivalent JAMA or NRC function on 1000x1000 random RSM,
    // but yields slightly larger residual errors than JAMA, which itself is about 2x larger than NRC:
    SelfAdjointEigenSolver<MatrixXd>    es(mat);
    const MatrixXd &    eigVecs = es.eigenvectors();
    const VectorXd &    eigVals = es.eigenvalues();
    vals.resize(dim);
    vecs.resize(dim,dim);
    for (size_t rr=0; rr<dim; ++rr)
        vals[rr] = eigVals(rr);
    for (size_t rr=0; rr<dim; ++rr)
        for (size_t cc=0; cc<dim; ++cc)
            vecs.rc(rr,cc) = eigVecs(rr,cc);        // MatrixXd takes (row,col) order
}

FgEigsRsmC<3>
fgEigsRsm(const FgMatrixC<double,3,3> & rsm)
{
    Matrix3d            mat;
    for (size_t rr=0; rr<3; ++rr) {
        for (size_t cc=rr; cc<3; ++cc) {
            double          v = (rsm.rc(rr,cc) + rsm.rc(cc,rr)) * 0.5;
            FGASSERT(boost::math::isfinite(v));
            mat(rr,cc) = v;
            mat(cc,rr) = v;
        }
    }
    SelfAdjointEigenSolver<Matrix3d>    es(mat);
    const Matrix3d &    eigVecs = es.eigenvectors();
    const Vector3d &    eigVals = es.eigenvalues();
    FgEigsRsmC<3>       ret;
    for (size_t rr=0; rr<3; ++rr)
        ret.vals[rr] = eigVals(rr);
    for (size_t rr=0; rr<3; ++rr)
        for (size_t cc=0; cc<3; ++cc)
            ret.vecs.rc(rr,cc) = eigVecs(rr,cc);
    return ret;
}

FgEigsRsmC<4>
fgEigsRsm(const FgMatrixC<double,4,4> & rsm)
{
    Matrix4d            mat;
    for (size_t rr=0; rr<4; ++rr) {
        for (size_t cc=rr; cc<4; ++cc) {
            double          v = (rsm.rc(rr,cc) + rsm.rc(cc,rr)) * 0.5;
            FGASSERT(boost::math::isfinite(v));
            mat(rr,cc) = v;
            mat(cc,rr) = v;
        }
    }
    SelfAdjointEigenSolver<Matrix4d>    es(mat);
    const Matrix4d &    eigVecs = es.eigenvectors();
    const Vector4d &    eigVals = es.eigenvalues();
    FgEigsRsmC<4>       ret;
    for (size_t rr=0; rr<4; ++rr)
        ret.vals[rr] = eigVals(rr);
    for (size_t rr=0; rr<4; ++rr)
        for (size_t cc=0; cc<4; ++cc)
            ret.vecs.rc(rr,cc) = eigVecs(rr,cc);
    return ret;
}

template<size_t dim>
FgEigsC<dim>
fgEigsT(const FgMatrixC<double,dim,dim> & in)
{
    // Ensure valid value and copy into Eigen format:
    MatrixXcd           mat(dim,dim);
    for (size_t rr=0; rr<dim; ++rr) {
        for (size_t cc=0; cc<dim; ++cc) {
            double          v = in.rc(rr,cc);
            FGASSERT(boost::math::isfinite(v));
            mat(rr,cc) = dcomplex(v,0);             // MatrixXcd takes (row,col) order
        }
    }
    ComplexEigenSolver<MatrixXcd>   ces;
    ces.compute(mat);
    FgEigsC<dim>            ret;
    const VectorXcd &       eigVals = ces.eigenvalues();
    const MatrixXcd &       eigVecs = ces.eigenvectors();
    for (size_t rr=0; rr<dim; ++rr)
        ret.vals[rr] = eigVals(rr);
    for (size_t cc=0; cc<dim; ++cc)
        for (size_t rr=0; rr<dim; ++rr)
            ret.vecs.rc(rr,cc) = eigVecs(rr,cc);
    return ret;
}

FgEigsC<3>
fgEigs(const FgMat33D & mat)
{return fgEigsT<3>(mat); }

FgEigsC<4>
fgEigs(const FgMat44D & mat)
{return fgEigsT<4>(mat); }

FgMatrixD
FgEigsRsm::matrix() const
{
    FgMatrixD       rhs = vecs.transpose();
    for (size_t rr=0; rr<rhs.nrows; ++rr)
        for (size_t cc=0; cc<rhs.ncols; ++cc)
            rhs.rc(rr,cc) *= vals[cc];
    return vecs * rhs;
}

static
void
testSymmEigenProblem(uint dim,bool print)
{
    // Random symmetric matrix, uniform distribution:
    FgMatrixD  mat(dim,dim);
    for (uint ii=0; ii<dim; ii++) {
        for (uint jj=ii; jj<dim; jj++) {
            mat.rc(ii,jj) = fgRand();
            mat.rc(jj,ii) = mat.rc(ii,jj);
        }
    }
    FgDbls              eigVals;
    FgMatrixD           eigVecs;
    FgTimer             timer;
    fgEigsRsm_(mat,eigVals,eigVecs);
    size_t              time = timer.readMs();
    // What is the pre-diagonalization speedup:
    FgMatrixD           innerHess = eigVecs.transpose() * mat * eigVecs,
                        innerEigvecs;
    FgDbls              innerEigVals;
    timer.start();
    fgEigsRsm_(innerHess,innerEigVals,innerEigvecs);
    size_t              timeInner = timer.readMs();
    FgMatrixD       eigValMat(dim,dim);
    eigValMat.setZero();
    for (uint ii=0; ii<dim; ii++)
        eigValMat.rc(ii,ii) = eigVals[ii];
    FgMatrixD       recon = eigVecs * eigValMat * eigVecs.transpose();
    double          residual = 0.0;
    for (uint ii=0; ii<dim; ii++)
        for (uint jj=ii; jj<dim; jj++)
            residual += abs(recon.rc(ii,jj) - mat.rc(ii,jj));
    // We can't expect matrix diagonalization to be accurate to machine
    // precision. In fact, depending on how extreme the eigenvalues are, the precision
    // can be arbitrarily poor. Experiments with random matrices show that the 
    // average absolute error increases faster than the square of the dimension
    // So we estimate the cube:
    double          tol = (dim*dim*dim) * numeric_limits<double>::epsilon();
    fgout << fgnl << "Dim: " << dim << fgpush
        << fgnl << "Residual: " << residual << " tolerance: " << tol
        << fgnl << "Time: " << time
        << fgnl << "Inner time: " << timeInner;
    if (print) {
        FG_HI1(mat);
        FG_HI1(innerHess);
        FG_HI1(eigVals);
        FG_HI1(eigVecs);
        FG_HI1(innerEigVals);
        FG_HI1(innerEigvecs);
        FG_HI1(eigVecs.transpose() * eigVecs);
    }
    fgout << fgpop;
    FGASSERT(residual < tol);
}

static
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

static
void
testSymmEigs(const FgArgs &)
{
    fgRandSeedRepeatable();
    testSymmEigenProblem(10,true);
    testSymmEigenProblem(30,false);
#ifndef _DEBUG
    testSymmEigenProblem(300,false);
    //testSymmEigenProblem(1000,false);
#endif
}

void
fgMatrixSolverTest(const FgArgs & args)
{
    FgCmds      cmds;
    cmds.push_back(FgCmd(testAsymEigs,"asym","Arbitrary real matrix eigensystem"));
    cmds.push_back(FgCmd(testSymmEigs,"symm","Real symmetric matrix eigensystem"));
    fgMenu(args,cmds,true);
}
