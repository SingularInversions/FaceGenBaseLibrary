//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGMATRIXSOLVER_HPP
#define FGMATRIXSOLVER_HPP

#include "FgMatrixV.hpp"

namespace Fg {

// Returns the U of the U^T * U Cholesky decomposition of a symmetric positive definite (SPD) matrix.
// Throws if matrix not PD. Potential loss of precision if matrix has high condition number.
MatUT2D             cCholesky(MatS2D spd);
MatUT3D             cCholesky(MatS3D const & spd);

struct      UTUDecomp
{
    MatUT3D         U;      // Upper triangular part of the U^T * U decomposition
    Vec3UI          p;      // Permutation map from input index to solution index
};

// Solve linear system of equations of the form Mx = b. Assumes matrix is well-conditioned:
Vec2D               solveLinear(Mat22D const & M,Vec2D const & b);

// Solve the Matrix equation Mx = b when M is full rank:
Vec2D               solveLinear(MatS2D M,Vec2D b);
// Solve the Matrix equation Mx = b when M is symmetric positive definite (this is not checked):
Vec3D               solveLinear(MatS3D SPD,Vec3D b);
// Solve the matrix equation Mx = b when M is any matrix.
// If M is singular, this returns a solution vector with one or more components equal to zero:
Vec3D               solveLinearRobust(Mat33D const & M,Vec3D const & b);
// M must be full rank:
Vec3D               solveLinear(Mat33D const & M,Vec3D const & b);
Vec4D               solveLinear(Mat44D const & M,Vec4D const & b);

// Compute only the eigenvalues (smallest to largest) of a real symmetrix matrix:
Doubles             cEigvalsRsm(MatD const & rsm);

// Real symmetric matrix represented by its eigen decomposition: RSM = vecs * Diag(vals) * vecs^T
struct      RsmEigs
{
    Doubles             vals;                   // Eigenvalues, smallest to largest
    MatD                vecs;                   // Column vectors are the respective eigenvectors
    FG_SER2(vals,vecs)

    inline MatSD        asMatS() const {return selfDiagTransposeProduct(vecs,vals); }
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

// PD real symmetric matrix represented by its left square root of the eigen decomposition RSM = sqrt * sqrt^T
// where: sqrt = vecs * Diag(vals)
struct      RsmSqrt
{
    Doubles             vals;       // eigenvalue square roots. all > 0
    MatD                vecs;       // columns are eigenvectors of the RSM
    FG_SER2(vals,vecs)

    // if this is a sqrt covariance matrix, the left matrix is the Mhlbs -> World transform:
    Doubles             leftMatMul(Doubles const & v) const {return vecs * mapMul(v,vals); }
    MatD                leftMatrix() const {return scaleColumns(vecs,vals); }
    // if this is a sqrt covariance matrix, the inverse right matrix is the World -> Mhlbs transform:
    MatD                invRightMatrix() const {return scaleRows(transpose(vecs),mapDiv(1.0,vals)); }
    // if this is a covariance matrix, this will generate random samples from the distribution:
    Doubles             rand() const {return leftMatMul(cRandNormals(vals.size())); }
};

inline RsmSqrt      cRsmSqrt(RsmEigs const & re)        // throws if any eigenvalues are <= 0
{
    return {
        mapCall(re.vals,[](double v){FGASSERT(v>0); return sqrt(v); }),
        re.vecs
    };
}

// Compute eigenvalues and eigenvectors of a real symmetric matrix in O(dim^3) time
// eigenvectors are returned in order of smallest to largest:
RsmEigs             cRsmEigs(MatSD const & rsm);
RsmEigs             cRsmEigs(MatD const & rsm);     // deprecated
MatSD               cInverse(MatSD const & rsm,double minInvCond);

// solve a linear system with a symmetric definite (ie non-zero determinant) matrix.
// Throws if the matrix is ill conditioned:
Doubles             solveLinear(MatSD const & M,Doubles b);

// Real eigenvalues and eigenvectors of a real symmetric square const-size matrix:
template<size_t D>
struct      EigsRsmC
{
    Mat<double,D,1>         vals;   // Eigenvalues
    Mat<double,D,D>         vecs;   // Column vectors are the respective eigenvectors.
};
typedef EigsRsmC<3>         EigsRsm3;
typedef EigsRsmC<4>         EigsRsm4;

template<size_t D>
std::ostream &      operator<<(std::ostream & os,EigsRsmC<D> const & e)
{
    return os
        << fgnl << "Eigenvalues: " << e.vals.transpose()
        << fgnl << "Eigvenvector columns: " << e.vecs;
}

// Eigenvalues of a square real symmetric matrix, returned in order from smallest to largest eigval:
EigsRsm3            cRsmEigs(Mat33D const & rsm);
EigsRsm3            cRsmEigs(MatS3D const & rsm);
EigsRsm4            cRsmEigs(Mat44D const & rsm);

template<size_t D>
struct      EigsC
{
    Mat<std::complex<double>,D,1>       vals;
    Mat<std::complex<double>,D,D>       vecs;   // Column vectors are the respective eigenvectors.
};

template<size_t D>
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

// affine measurement of a vector (ie. linear plus offset):
struct      LinOffset
{
    Doubles             covector;
    double              offset;
    FG_SER2(covector,offset)

    double              getVal(Doubles const & crd) const {return cDot(covector,crd) + offset; }
    Doubles             deltaCrd(double deltaVal) const {return covector * (deltaVal / cMag(covector)); }
};

// ordinary linear least squares regression solver:
LinOffset           cOrdinaryLLS(
    MatD const &        coords,     // row: sample (of S), col: coeff (of M)
    Doubles const &     atts);      // attribute being regressed (of S)

}

#endif
