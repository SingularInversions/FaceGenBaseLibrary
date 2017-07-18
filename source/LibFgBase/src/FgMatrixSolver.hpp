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
#include "jama_eig.h"

// Eigenvalues of a square symmetric matrix.
// Runs in O(dim^3) time, with residual error O(dim^2).
// Runtime is almost identical the equivalent NRC function (37s for 1000x1000 random),
// but yields larger residual errors (10 x dim^2 x epsilon() for 1000x1000 versus 5x for NRC).
void
fgSymmEigs_(
    const FgMatrixD &   rsm,
    FgDbls &            vals,   // RETURNED: Eigenvalues, smallest to largest
    FgMatrixD &         vecs);  // RETURNED: Col vectors are respective eigenvectors

struct  FgRealEigs
{
    FgDbls              vals;   // Eigenvalues
    FgMatrixD           vecs;   // Column vectors are the respective eigenvectors

    FgMatrixD
    matrix() const;             // Return the matrix formed from this eigen system
};

// As above:
inline
FgRealEigs
fgSymmEigs(const FgMatrixD & rsm)
{
    FgRealEigs      ret;
    fgSymmEigs_(rsm,ret.vals,ret.vecs);
    return ret;
}

struct  FgEigs
{
    FgDbls              real;   // Eigenvalue real components
    FgDbls              imag;   // Eigenvalue imaginary components
    FgMatrixD           vecs;   // Column vectors are the respective eigenvectors
};

template<typename T,uint dim>
struct FgEigsC
{
    FgMatrixC<T,dim,1>          real;   // Eigenvalue real components.
    FgMatrixC<T,dim,1>          imag;   // Eigenvalue imaginary components.
    FgMatrixC<T,dim,dim>        vecs;   // Column vectors are the respective eigenvectors.
};

template<typename T,uint dim>
std::ostream &
operator<<(std::ostream & os,const FgEigsC<T,dim> & e)
{
    os << fgnl << "Eigval real components: " << e.real.transpose()
        << fgnl << "Eigval imag components: " << e.imag.transpose()
        << fgnl << "Eigvecs as columns: " << e.vecs;
    return os;
}

// If the given matrix is symmetric, eigenvalues will be ordered from smallest to largest.
// Otherwise they are not ordered as they could be complex.
// NB: The returned eigenvectors are not normalized !
template<class T,uint dim>
FgEigsC<T,dim>
fgEigs(const FgMatrixC<T,dim,dim> & rsm)
{
    FgEigsC<T,dim>       ret;
    // JAMA enters an infinite loop with NaNs:
    for (uint ii=0; ii<dim*dim; ++ii)
        FGASSERT(boost::math::isfinite(rsm[ii]));
        // We use a const cast since we know 'solver' will not modify the elements, even though
        // the Array2D object holds a non-const pointer to our data.
    JAMA::Eigenvalue<T>     solver(TNT::Array2D<T>(dim,dim,const_cast<T*>(rsm.dataPtr())));
    for (uint rr=0; rr<dim; ++rr) {
        ret.real[rr] = solver.m_d[rr];
        ret.imag[rr] = solver.e[rr];
        for (uint cc=0; cc<dim; ++cc)
            ret.vecs.rc(rr,cc) = solver.V[rr][cc];
    }
    return ret;
}

// Special case speedup for nonsymmetric 3x3:
FgEigsC<double,3>
fgEigs(const FgMat33D & mat);

#endif
