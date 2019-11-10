//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGMATRIXSOLVER_HPP
#define FGMATRIXSOLVER_HPP

#include "FgStdLibs.hpp"
#include "FgMatrixC.hpp"
#include "FgMatrixV.hpp"

namespace Fg {

// Eigenvalues of a square real symmetric matrix.
// Runs in O(dim^3) time, with residual error O(dim^2.?).
void
fgEigsRsm_(
    const MatD &   rsm,    // Symmetric matrix
    Doubles &            vals,   // RETURNED: Eigenvalues, smallest to largest
    MatD &         vecs);  // RETURNED: Col vectors are respective eigenvectors

struct  FgEigsRsm
{
    Doubles              vals;   // Eigenvalues
    MatD           vecs;   // Column vectors are the respective eigenvectors

    MatD
    matrix() const;             // Return the matrix formed from this eigen system
};

// As above:
inline
FgEigsRsm
fgEigsRsm(const MatD & rsm)
{
    FgEigsRsm      ret;
    fgEigsRsm_(rsm,ret.vals,ret.vecs);
    return ret;
}

// Real eigenvalues and eigenvectors of a real symmetric square const-size matrix:
template<uint dim>
struct FgEigsRsmC
{
    Mat<double,dim,1>         vals;   // Eigenvalues
    Mat<double,dim,dim>       vecs;   // Column vectors are the respective eigenvectors.
};

FgEigsRsmC<3>   fgEigsRsm(const Mat<double,3,3> & rsm);
FgEigsRsmC<4>   fgEigsRsm(const Mat<double,4,4> & rsm);

template<uint dim>
struct FgEigsC
{
    Mat<std::complex<double>,dim,1>   vals;
    Mat<std::complex<double>,dim,dim> vecs;   // Column vectors are the respective eigenvectors.
};

template<uint dim>
std::ostream &
operator<<(std::ostream & os,const FgEigsC<dim> & e)
{
    return os
        << fgnl << "Eigenvalues: " << e.vals.transpose()
        << fgnl << "Eigenvectors as columns: " << e.vecs;
}

// Eigensolver for arbitrary matrix, returned in arbitrary order since eigenvalues
// can be complex. The eigenvectors can always be made real when the associated eigevalue
// is real but do not default to a real representation:
FgEigsC<3>      fgEigs(const Mat33D & mat);
FgEigsC<4>      fgEigs(const Mat44D & mat);

}

#endif
