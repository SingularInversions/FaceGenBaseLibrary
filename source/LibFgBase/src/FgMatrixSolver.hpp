//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGMATRIXSOLVER_HPP
#define FGMATRIXSOLVER_HPP

#include "FgStdLibs.hpp"
#include "FgMatrixC.hpp"
#include "FgMatrixV.hpp"

namespace Fg {

// Returns the U of the U^T * U Cholesky decomposition of a symmetric positive definite matrix,
MatUT3D             cCholesky(MatS3D spd);      // spd cannot be singular

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

// Real symmetric matrix represented by its eigen decomposition: H = vecs * Diag(vals) * vecs^T
struct      RsmEigs
{
    Doubles             vals;                   // Eigenvalues from largest to smallest
    MatD                vecs;                   // Column vectors are the respective eigenvectors

    MatD                rsm() const;            // Compute represented real symmetric matrix
    MatD                mhlbsToWorld() const;   // Return vecs * D(vals^-0.5). Throws if there are eigenvals <= 0
    // Return inverse of hessian defined by this eigenspectrum. Throws if any 0 eigenvalues:
    MatD                inverse() const;
};
std::ostream & operator<<(std::ostream &,RsmEigs const &);

inline RsmEigs      cEigsRsm(MatD const & rsm)
{
    RsmEigs      ret;
    cEigsRsm_(rsm,ret.vals,ret.vecs);
    return ret;
}

// Real eigenvalues and eigenvectors of a real symmetric square const-size matrix:
template<uint dim>
struct      EigsRsmC
{
    Mat<double,dim,1>       vals;   // Eigenvalues
    Mat<double,dim,dim>     vecs;   // Column vectors are the respective eigenvectors.
};

template<uint dim>
std::ostream &      operator<<(std::ostream & os,EigsRsmC<dim> const & e)
{
    return os
        << fgnl << "Eigenvalues: " << e.vals.transpose()
        << fgnl << "Eigvenvector columns: " << e.vecs;
}

// Eigenvalues of a square real symmetric matrix, returned in order from smallest to largest eigval:
EigsRsmC<3>         cEigsRsm(Mat33D const & rsm);
EigsRsmC<4>         cEigsRsm(Mat44D const & rsm);

template<uint dim>
struct      EigsC
{
    Mat<std::complex<double>,dim,1>   vals;
    Mat<std::complex<double>,dim,dim> vecs;   // Column vectors are the respective eigenvectors.
};

template<uint dim>
std::ostream &      operator<<(std::ostream & os,const EigsC<dim> & e)
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
