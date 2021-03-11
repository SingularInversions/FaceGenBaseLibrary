//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
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
struct   NormalDist
{
    typedef Mat<double,dim,1>     Vec;

    Vec                 mean;
    Mat<double,dim,dim> root;       // Square root of the concentration matrix

    NormalDist() {root.setIdentity(); }
    NormalDist(const Vec & m,const Mat<double,dim,dim> & c) : mean(m), root(c) {}

    double
    operator() (const Vec & pos) const
    {
        Vec         mhlbs = root * (pos-mean);
        return (
            std::pow(2.0 * pi(),double(dim) * -0.5) *
            cDeterminant(root) *
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

typedef NormalDist<3> NormalDist3D;

template<uint dim>
std::ostream &
operator<<(std::ostream & ss,const NormalDist<dim> &  norm)
{
    return 
        ss  << fgnl << "Norm" << dim
            << " Mean: " << norm.mean() 
            << fgnl << "Conc Root: " << norm.root;
}

}

#endif
