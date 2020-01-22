//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGSTATSNORMAL_HPP
#define FGSTATSNORMAL_HPP

#include "FgMatrixC.hpp"
#include "FgMath.hpp"
#include "FgRandom.hpp"

namespace Fg {

template<uint dim>
struct   FgNormal
{
    typedef Mat<double,dim,1>     Vec;

    Vec                 mean;
    Mat<double,dim,dim> root;       // Square root of the concentration matrix

    FgNormal() {root.setIdentity(); }
    FgNormal(const Vec & m,const Mat<double,dim,dim> & c) : mean(m), root(c) {}

    double
    operator() (const Vec & pos) const
    {
        Vec         mhlbs = root * (pos-mean);
        return (
            std::pow(2.0 * pi(),double(dim) * -0.5) *
            determinant(root) *
            expSafe(-0.5 * mhlbs.mag()));
    }

    Vec
    randomSample() const;

    double
    lnLikelihood(const Vec & pos) const
    {
        Vec         mhlbs = root * (pos-mean);
        return (std::log(determinant(root)) -
                0.5 * double(dim) * ln2Pi() -
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

}

#endif
