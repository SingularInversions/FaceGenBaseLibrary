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

// Runs in O(dim^3) time, with residual error O(dim^2).
// Runtime is almost identical the equivalent NRC function (37s for 1000x1000 random),
// but yields larger residual errors (10 x dim^2 x epsilon() for 1000x1000 versus 
// 5x for NRC).
template<class T>
void
fgRealSymmEigs(
    const FgMatrixV<T>    &rsm,
    FgMatrixV<T>          &val, // RETURNED: Col vector of eigenvalues, smallest to largest
    FgMatrixV<T>          &vec) // RETURNED: Col vectors are respective eigenvectors
{
    // JAMA enters an infinite loop with NaNs:
    for (size_t ii=0; ii<rsm.m_data.size(); ++ii)
        FGASSERT(boost::math::isfinite(rsm.m_data[ii]));
    uint                    dim = rsm.numRows();
    FGASSERT(rsm.numCols() == dim);
        // We use a const cast since we know 'solver' will not modify the elements, even though
        // the Array2D object holds a non-const pointer to our data.
    JAMA::Eigenvalue<T>
        solver(TNT::Array2D<T>(rsm.numRows(),rsm.numCols(),const_cast<T*>(rsm.dataPtr())));
    TNT::Array2D<T>         vecs;
    TNT::Array1D<T>         vals;
    solver.getV(vecs);
    solver.getRealEigenvalues(vals);
    val.resize(dim,1);
    vec.resize(dim,dim);
    int                     idim = static_cast<int>(dim);
    for (int row=0; row<idim; row++) {
        val[row] = vals[row];
        for (uint col=0; col<dim; col++)
            vec.elem(row,col) = vecs[row][col];
    }
}

template<typename T,uint dim>
struct FgEigs
{
    FgMatrixC<T,dim,dim>        vals;   // Diagonal matrix of eigenvalues from smallest to largest.
    FgMatrixC<T,dim,dim>        vecs;   // Column vectors are the respective eigenvectors.
};

// As above for FgMatrixC
template<class T,uint dim>
FgEigs<T,dim>
fgRealSymmEigs(const FgMatrixC<T,dim,dim> & rsm)
{
    FgEigs<T,dim>       ret;
    // JAMA enters an infinite loop with NaNs:
    for (uint ii=0; ii<dim*dim; ++ii)
        FGASSERT(boost::math::isfinite(rsm[ii]));
        // We use a const cast since we know 'solver' will not modify the elements, even though
        // the Array2D object holds a non-const pointer to our data.
    JAMA::Eigenvalue<T>     solver(TNT::Array2D<T>(dim,dim,const_cast<T*>(rsm.dataPtr())));
    TNT::Array2D<T>         vecs;
    TNT::Array1D<T>         vals;
    solver.getV(vecs);
    solver.getRealEigenvalues(vals);
    for (uint row=0; row<dim; row++) {
        for (uint col=0; col<dim; col++) {
            ret.vals.elem(row,col) = T(0);
            ret.vecs.elem(row,col) = vecs[int(row)][int(col)];
        }
        ret.vals.elem(row,row) = vals[int(row)];
    }
    return ret;
}

#endif
