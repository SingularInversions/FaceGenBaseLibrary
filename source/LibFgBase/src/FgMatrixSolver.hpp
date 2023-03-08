//
// Copyright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGMATRIXSOLVER_HPP
#define FGMATRIXSOLVER_HPP

#include "FgSerial.hpp"
#include "FgMatrixC.hpp"
#include "FgMatrixV.hpp"

namespace Fg {

// Returns the U of the U^T * U Cholesky decomposition of a symmetric positive definite matrix,
MatUT2D             cCholesky(MatS2D spd);      // spd must be symmetric positive definite
MatUT3D             cCholesky(MatS3D spd);      // "

struct      UTUDecomp
{
    MatUT3D         U;      // Upper triangular part of the U^T * U decomposition
    Vec3UI          p;      // Permutation map from input index to solution index
};

// Solve the Matrix equation Ax = b when A is full rank:
Vec2D               solveLinear(MatS2D fullRank,Vec2D b);
// Solve the Matrix equation Ax = b when A is symmetric positive definite (this is not checked):
Vec3D               solveLinear(MatS3D SPD,Vec3D b);
// Solve the matrix equation Ax = b.
// If A is singular, this returns a solution vector with one or more components equal to zero:
Vec3D               solve(Mat33D A,Vec3D b);
// Compute only the eigenvalues (smallest to largest) of a real symmetrix matrix:
Doubles             cEigvalsRsm(MatD const & rsm);

// Compute eigenvalues and eigenvectors of a real symmetric matrix:
// Runs in O(dim^3) time, with residual error O(dim^2.?).
void                cEigsRsm_(
    MatD const &        rsm,                    // Real symmetric matrix
    Doubles &           vals,                   // RETURNED: Eigenvalues, smallest to largest
    MatD &              vecs);                  // RETURNED: Col vectors are respective eigenvectors

// Real symmetric matrix represented by its eigen decomposition: RSM = vecs * Diag(vals) * vecs^T
struct      RsmEigs
{
    Doubles             vals;                   // Eigenvalues
    MatD                vecs;                   // Column vectors are the respective eigenvectors
    FG_SER2(vals,vecs)

    MatD                rsm() const;            // Compute represented real symmetric matrix
    // turns a covariance matrix RsmEigs into a precision matrix and vice versa.
    // throws if any of 'vals' are zero:
    RsmEigs             inverse() const
    {
        return {
            mapCall(vals,[](double v){FGASSERT(v!=0); return 1.0/v; }),
            vecs
        };
    }
};
std::ostream & operator<<(std::ostream &,RsmEigs const &);

// Real symmetric matrix represented by its square root eigen decomposition RSM = sqrt * sqrt^T
// where: sqrt = vecs * Diag(vals)
struct      RsmSqrt
{
    Doubles             vals;       // must all be > 0
    MatD                vecs;
    FG_SER2(vals,vecs)

    // if the matrix represented is a covariance matrix, these are the Mahalanobis <-> world
    // coordinate transforms:
    Doubles             mhlbsToWorld(Doubles const & mhlbs) const {return vecs * mapMul(mhlbs,vals); }
    Doubles             worldToMhlbs(Doubles const & world) const {return mapDiv(world,vals) * vecs; }
};

inline RsmSqrt      cRsmSqrt(RsmEigs const & re)        // throws if any eigenvalues are <= 0
{
    return {
        mapCall(re.vals,[](double v){FGASSERT(v>0); return sqrt(v); }),
        re.vecs
    };
}

inline RsmEigs      cEigsRsm(MatD const & rsm)
{
    RsmEigs      ret;
    cEigsRsm_(rsm,ret.vals,ret.vecs);
    return ret;
}

// Real eigenvalues and eigenvectors of a real symmetric square const-size matrix:
template<uint D>
struct      EigsRsmC
{
    Mat<double,D,1>         vals;   // Eigenvalues
    Mat<double,D,D>         vecs;   // Column vectors are the respective eigenvectors.
};
typedef EigsRsmC<3>         EigsRsm3;
typedef EigsRsmC<4>         EigsRsm4;

template<uint D>
std::ostream &      operator<<(std::ostream & os,EigsRsmC<D> const & e)
{
    return os
        << fgnl << "Eigenvalues: " << e.vals.transpose()
        << fgnl << "Eigvenvector columns: " << e.vecs;
}

// Eigenvalues of a square real symmetric matrix, returned in order from smallest to largest eigval:
EigsRsm3            cEigsRsm(Mat33D const & rsm);
EigsRsm3            cEigsRsm(MatS3D const & rsm);
EigsRsm4            cEigsRsm(Mat44D const & rsm);

template<uint D>
struct      EigsC
{
    Mat<std::complex<double>,D,1>       vals;
    Mat<std::complex<double>,D,D>       vecs;   // Column vectors are the respective eigenvectors.
};

template<uint D>
std::ostream &      operator<<(std::ostream & os,const EigsC<D> & e)
{
    return os
        << fgnl << "Eigenvalues: " << e.vals.transpose()
        << fgnl << "Eigenvector columns: " << e.vecs;
}

// Eigensolver for arbitrary matrix, returned in arbitrary order since eigenvalues
// can be complex. The eigenvectors can always be made real when the associated eigevalue
// is real but do not default to a real representation:
EigsC<3>            cEigs(Mat33D const & mat);
EigsC<4>            cEigs(Mat44D const & mat);

}

#endif
