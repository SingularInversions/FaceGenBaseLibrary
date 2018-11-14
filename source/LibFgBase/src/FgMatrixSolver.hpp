//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Jun 22, 2006
//

#ifndef FGMATRIXSOLVER_HPP
#define FGMATRIXSOLVER_HPP

#include "FgStdLibs.hpp"
#include "FgMatrix.hpp"

// Eigenvalues of a square real symmetric matrix.
// Runs in O(dim^3) time, with residual error O(dim^2.?).
void
fgEigsRsm_(
    const FgMatrixD &   rsm,    // Symmetric matrix
    FgDbls &            vals,   // RETURNED: Eigenvalues, smallest to largest
    FgMatrixD &         vecs);  // RETURNED: Col vectors are respective eigenvectors

struct  FgEigsRsm
{
    FgDbls              vals;   // Eigenvalues
    FgMatrixD           vecs;   // Column vectors are the respective eigenvectors

    FgMatrixD
    matrix() const;             // Return the matrix formed from this eigen system
};

// As above:
inline
FgEigsRsm
fgEigsRsm(const FgMatrixD & rsm)
{
    FgEigsRsm      ret;
    fgEigsRsm_(rsm,ret.vals,ret.vecs);
    return ret;
}

// Real eigenvalues and eigenvectors of a real symmetric square const-size matrix:
template<uint dim>
struct FgEigsRsmC
{
    FgMatrixC<double,dim,1>         vals;   // Eigenvalues
    FgMatrixC<double,dim,dim>       vecs;   // Column vectors are the respective eigenvectors.
};

FgEigsRsmC<3>   fgEigsRsm(const FgMatrixC<double,3,3> & rsm);
FgEigsRsmC<4>   fgEigsRsm(const FgMatrixC<double,4,4> & rsm);

template<uint dim>
struct FgEigsC
{
    FgMatrixC<std::complex<double>,dim,1>   vals;
    FgMatrixC<std::complex<double>,dim,dim> vecs;   // Column vectors are the respective eigenvectors.
};

template<uint dim>
std::ostream &
operator<<(std::ostream & os,const FgEigsC<dim> & e)
{
    return os
        << fgnl << "Eigenvalues: " << e.vals.transpose()
        << fgnl << "Eigenvectors as columns: " << e.vecs;
}

// Eigenspectrum of arbitrary matrix, returned in arbitrary order since eigenvalues
// can be complex. The eigenvectors can always be made real when the associated eigevalue
// is real but do not default to a real representation:
FgEigsC<3>      fgEigs(const FgMat33D & mat);
FgEigsC<4>      fgEigs(const FgMat44D & mat);

#endif
