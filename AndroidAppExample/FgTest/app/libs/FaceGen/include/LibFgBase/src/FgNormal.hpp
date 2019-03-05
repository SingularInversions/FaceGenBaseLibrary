//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:Andrew Beatty
// Date: June 3, 2009
//

#ifndef FGSTATSNORMAL_HPP
#define FGSTATSNORMAL_HPP

#include "FgMatrixC.hpp"
#include "FgMath.hpp"
#include "FgRandom.hpp"

template<uint dim>
struct   FgNormal
{
    typedef FgMatrixC<double,dim,1>     Vec;
    typedef FgMatrixC<double,dim,dim>   Mat;

    Vec         mean;
    Mat         root;       // Square root of the concentration matrix

    FgNormal() {root.setIdentity(); }
    FgNormal(const Vec & m,const Mat & c) : mean(m), root(c) {}

    double
    operator() (const Vec & pos) const
    {
        Vec         mhlbs = root * (pos-mean);
        return (
            std::pow(2.0 * fgPi(),double(dim) * -0.5) *
            fgDeterminant(root) *
            fgExp(-0.5 * mhlbs.mag()));
    }

    Vec
    randomSample() const;

    double
    lnLikelihood(const Vec & pos) const
    {
        Vec         mhlbs = root * (pos-mean);
        return (std::log(fgDeterminant(root)) -
                0.5 * double(dim) * fgLn_2pi() -
                0.5 * mhlbs.mag());
    }
};

typedef FgNormal<3> FgNormal3D;

template<uint dim>
std::ostream &
operator<<(std::ostream & ss,const FgNormal<dim> &  norm)
{
    return 
        ss  << fgnl << "Norm" << dim
            << " Mean: " << norm.mean() 
            << fgnl << "Conc Root: " << norm.root;
}

#endif
