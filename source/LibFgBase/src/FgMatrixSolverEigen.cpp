//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Make use of the Eigen library. To avoid absurd compile times with MSVC the library
// must be modified by changing ~Eigen/src/ore/util/Macros.h    from:
// #define EIGEN_STRONG_INLINE __forceinline                    to:
// #define EIGEN_STRONG_INLINE inline
// (this cannot be overridden in this source code as it occurs within the Eigen headers)
// Real symmetric matrix solver at 1000x1000 and 2000x2000 was timed before and after the change
// with little difference (in fact slightly faster when keeping eigenvectors).

#include "stdafx.h"

#include "FgMath.hpp"
#include "FgMatrixSolver.hpp"
#include "FgRandom.hpp"
#include "FgMain.hpp"
#include "FgTime.hpp"
#include "FgCommand.hpp"
#include "FgApproxEqual.hpp"


#ifdef _MSC_VER
    #pragma warning(push,0)     // Eigen triggers lots of warnings
#endif

#define EIGEN_MPL2_ONLY     // Only use permissive licensed source files from Eigen
#include "Eigen/Dense"
#include "Eigen/Core"

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

using namespace std;

using namespace Eigen;

namespace Fg {

namespace {

// Ensure exact symmetry and valid finite floating point values while converting to Eigen format
// (any asymmetry will cause totally incorrect results from RSM solver):
void                convertRsm_(MatD const & rsm,MatrixXd & ret)
{
    size_t          dim = rsm.ncols;
    FGASSERT(rsm.nrows == dim);
    for (size_t rr=0; rr<dim; ++rr) {
        for (size_t cc=rr; cc<dim; ++cc) {
            double          v = (rsm.rc(rr,cc) + rsm.rc(cc,rr)) * 0.5;
            FGASSERT(isfinite(v));
            ret(rr,cc) = v;
            ret(cc,rr) = v;
        }
    }
}

MatrixXd            toEigen(MatSD const & rsm)
{
    size_t              D = rsm.dim;
    MatrixXd            ret (D,D);
    double const *      ptr = &rsm.data[0];
    for (size_t rr=0; rr<D; ++rr) {
        double const *      ptrM = &rsm.data[0]+rr;
        size_t              step = D-1;
        for (size_t cc=0; cc<rr; ++cc) {
            ret(rr,cc) = *ptrM;
            ptrM += step;
            --step;
        }
        for (size_t cc=rr; cc<D; ++cc)
            ret(rr,cc) = *ptr++;
    }
    return ret;
}

}

// 2D case allows for simple explicit formula:
Vec2D               solveLinear(Mat22D const & A,Vec2D const & b)
{
    double              det = A[0]*A[3] - A[1]*A[2];
    FGASSERT(det != 0.0);
    return {
        (b[0]*A[3] - b[1]*A[1])/det,
        (b[1]*A[0] - b[0]*A[2])/det
    };
}

Vec3D               solveLinear(Mat33D const & M,Vec3D const & b)
{
    Eigen::Matrix3d         mat;
    Eigen::Vector3d         vec;
    for (uint rr=0; rr<3; ++rr)
        for (uint cc=0; cc<3; ++cc)
            mat(rr,cc) = M.rc(rr,cc);
    for (uint rr=0; rr<3; ++rr)
        vec(rr) = b[rr];
    // There are many alternatives to this in Eigen: ParialPivLU, FullPivLU, HouseholderQR etc.
    auto                    qr = mat.colPivHouseholderQr();
    FGASSERT(qr.isInvertible());
    Eigen::Vector3d         sol = qr.solve(vec);
    return {sol(0),sol(1),sol(2)};
}

Vec3D               solveLinearRobust(Mat33D const & A_,Vec3D const & b_)
{
    Matrix3d        A;
    Vector3d        b;
    for (uint rr=0; rr<3; ++rr) {
        for (uint cc=0; cc<3; ++cc)
            A(rr,cc) = A_.rc(rr,cc);
        b[rr] = b_[rr];
    }
    // If the matrix is singular, this returns a solution vector with one or more components equal to zero:
    ColPivHouseholderQR<Matrix3d>   alg(A);
    Vector3d        r = alg.solve(b);
    return Vec3D {r[0],r[1],r[2]};
}

Vec4D               solveLinear(Mat44D const & M,Vec4D const & b)
{
    Eigen::Matrix4d         mat;
    Eigen::Vector4d         vec;
    for (uint rr=0; rr<4; ++rr)
        for (uint cc=0; cc<4; ++cc)
            mat(rr,cc) = M.rc(rr,cc);
    for (uint rr=0; rr<4; ++rr)
        vec(rr) = b[rr];
    // There are many alternatives to this in Eigen: ParialPivLU, FullPivLU, HouseholderQR etc.
    auto                    qr = mat.colPivHouseholderQr();
    FGASSERT(qr.isInvertible());
    Eigen::Vector4d         sol = qr.solve(vec);
    return {sol(0),sol(1),sol(2),sol(3)};
}

Doubles             cEigvalsRsm(MatD const & rsm)
{
    size_t              dim = rsm.ncols;
    MatrixXd            mat(dim,dim);
    convertRsm_(rsm,mat);
    SelfAdjointEigenSolver<MatrixXd>    es(mat,EigenvaluesOnly);
    VectorXd const &                    eigVals = es.eigenvalues();
    Doubles                             ret(dim);
    for (size_t rr=0; rr<dim; ++rr)
        ret[rr] = eigVals(rr);
    return ret;
}

RsmEigs             cRsmEigs(MatSD const & rsm)
{
    RsmEigs             ret;
    size_t              D = rsm.dim;
    if (D > 1) {
        MatrixXd                mat = toEigen(rsm);
        // Eigen runtime is more than 3x faster than equivalent JAMA or NRC function on 1000x1000 random RSM,
        // but yields slightly larger residual errors than JAMA, which itself is about 2x larger than NRC:
        SelfAdjointEigenSolver<MatrixXd> es(mat);
        MatrixXd const &        eigVecs = es.eigenvectors();
        VectorXd const &        eigVals = es.eigenvalues();
        ret.vals.resize(D);
        ret.vecs.resize(D,D);
        for (size_t rr=0; rr<D; ++rr)
            ret.vals[rr] = eigVals(rr);
        for (size_t rr=0; rr<D; ++rr)
            for (size_t cc=0; cc<D; ++cc)
                ret.vecs.rc(rr,cc) = eigVecs(rr,cc);        // MatrixXd takes (row,col) order
    }
    else if (D == 1)
        ret = {{rsm.data[0]},{1,1,Doubles{1.0}}};           // 'Doubles' avoids clang warning
    return ret;
}

MatSD               cInverse(MatSD const & rsm,double minInvCond)
{
    RsmEigs             eigs = cRsmEigs(rsm);
    VecD2               bounds = cBounds(eigs.vals);
    FGASSERT(bounds[0]/bounds[1] > minInvCond);
    return eigs.inverse().asMatS();
}

RsmEigs             cRsmEigs(MatD const & rsm)
{
    size_t              D = rsm.numCols();
    if (D == 1)
        return {{rsm.m_data[0]},{1,1,Doubles{1.0}}};        // 'Doubles' prevents clang warning
    RsmEigs             ret;
    MatrixXd            mat(D,D);
    convertRsm_(rsm,mat);
    // Eigen runtime is more than 3x faster than equivalent JAMA or NRC function on 1000x1000 random RSM,
    // but yields slightly larger residual errors than JAMA, which itself is about 2x larger than NRC:
    SelfAdjointEigenSolver<MatrixXd>    es(mat);
    const MatrixXd &    eigVecs = es.eigenvectors();
    const VectorXd &    eigVals = es.eigenvalues();
    ret.vals.resize(D);
    ret.vecs.resize(D,D);
    for (size_t rr=0; rr<D; ++rr)
        ret.vals[rr] = eigVals(rr);
    for (size_t rr=0; rr<D; ++rr)
        for (size_t cc=0; cc<D; ++cc)
            ret.vecs.rc(rr,cc) = eigVecs(rr,cc);        // MatrixXd takes (row,col) order
    return ret;
}

EigsRsm3            cRsmEigs(MatS3D const & rsm)
{
    for (double v : rsm.diag)
        FGASSERT(isfinite(v));
    for (double v : rsm.offd)
        FGASSERT(isfinite(v));
    Matrix3d            mat;
    for (size_t rr=0; rr<3; ++rr) {
        mat(rr,rr) = rsm.diag[rr];
        for (size_t cc=rr+1; cc<3; ++cc) {
            double          v = rsm.rc(rr,cc);
            mat(rr,cc) = v;
            mat(cc,rr) = v;
        }
    }
    // Eigen supposedly has 3x3 specialization of this in closed-form:
    SelfAdjointEigenSolver<Matrix3d>    es(mat);
    const Matrix3d &    eigVecs = es.eigenvectors();
    const Vector3d &    eigVals = es.eigenvalues();
    EigsRsm3            ret;
    for (size_t rr=0; rr<3; ++rr)
        ret.vals[rr] = eigVals(rr);
    for (size_t rr=0; rr<3; ++rr)
        for (size_t cc=0; cc<3; ++cc)
            ret.vecs.rc(rr,cc) = eigVecs(rr,cc);
    return ret;
}

EigsRsm3            cRsmEigs(Mat33D const & rsm)
{
    MatS3D              mat {
        {rsm.rc(0,0),rsm.rc(1,1),rsm.rc(2,2),},
        {
            (rsm.rc(0,1)+rsm.rc(1,0)) * 0.5,
            (rsm.rc(0,2)+rsm.rc(2,0)) * 0.5,
            (rsm.rc(1,2)+rsm.rc(2,1)) * 0.5,
        },
    };
    return cRsmEigs(mat);
}

EigsRsm4            cRsmEigs(Mat44D const & rsm)
{
    Matrix4d            mat;
    for (size_t rr=0; rr<4; ++rr) {
        for (size_t cc=rr; cc<4; ++cc) {
            double          v = (rsm.rc(rr,cc) + rsm.rc(cc,rr)) * 0.5;
            FGASSERT(isfinite(v));
            mat(rr,cc) = v;
            mat(cc,rr) = v;
        }
    }
    SelfAdjointEigenSolver<Matrix4d>    es(mat);
    const Matrix4d &    eigVecs = es.eigenvectors();
    const Vector4d &    eigVals = es.eigenvalues();
    EigsRsmC<4>       ret;
    for (size_t rr=0; rr<4; ++rr)
        ret.vals[rr] = eigVals(rr);
    for (size_t rr=0; rr<4; ++rr)
        for (size_t cc=0; cc<4; ++cc)
            ret.vecs.rc(rr,cc) = eigVecs(rr,cc);
    return ret;
}

template<size_t dim>
EigsC<dim>          fgEigsT(const Mat<double,dim,dim> & in)
{
    // Ensure valid value and copy into Eigen format:
    MatrixXd            mat(dim,dim);
    for (size_t rr=0; rr<dim; ++rr) {
        for (size_t cc=0; cc<dim; ++cc) {
            double          v = in.rc(rr,cc);
            FGASSERT(isfinite(v));
            mat(rr,cc) = v;                 // MatrixXd takes (row,col) order
        }
    }
    EigenSolver<MatrixXd>   es;
    es.compute(mat);
    EigsC<dim>            ret;
    const VectorXcd &       eigVals = es.eigenvalues();
    const MatrixXcd &       eigVecs = es.eigenvectors();
    for (size_t rr=0; rr<dim; ++rr)
        ret.vals[rr] = eigVals(rr);
    for (size_t cc=0; cc<dim; ++cc)
        for (size_t rr=0; rr<dim; ++rr)
            ret.vecs.rc(rr,cc) = eigVecs(rr,cc);
    return ret;
}

// Eigen does not have specialized solutions of 'EigenSolve' for small values
// so we just insantiate generic version above:
EigsC<3>            cEigs(Mat33D const & mat)
{
    return fgEigsT<3>(mat);
}
EigsC<4>            cEigs(Mat44D const & mat)
{
    return fgEigsT<4>(mat);
}

template<size_t D>
void                testSolveLinearT()
{
    for (size_t ii=0; ii<256; ++ii) {
        Mat<double,D,D>     M = Mat<double,D,D>::randNormal();
        Mat<double,D,1>     b = Mat<double,D,1>::randNormal(),
                            x = solveLinear(M,b);
        if (abs(cDeterminant(M)) < epsBits(20))         // don't test with ill conditioned
            continue;
        FGASSERT(isApproxEqualPrec(M*x,b,30));
    }
}

void                testSolveLinear(CLArgs const &)
{
    randSeedRepeatable();
    testSolveLinearT<2>();
    testSolveLinearT<3>();
    testSolveLinearT<4>();
}

}
