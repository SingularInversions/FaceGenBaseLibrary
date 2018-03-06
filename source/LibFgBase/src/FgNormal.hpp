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

struct   FgNormal1D
{
    double      mean;
    double      stdev;      // Always > 0.0

    typedef double Val;
    FgNormal1D() : mean(0.0), stdev(1.0) {}
    FgNormal1D(double m,double s) : mean(m), stdev(s) {FGASSERT(stdev > 0.0); }

    double
    operator()(double val) const
    {return fgMath::normal(val,mean,stdev); }

    double
    randomSample() const
    {return mean + stdev * fgRandNormal(); }

    double
    lnLikelihood(double val) const
    {return fgMath::lnNormal(val,mean,stdev); }
};

std::ostream &
operator<<(std::ostream & ss,const FgNormal1D & norm);

template<uint dim>
struct   FgNormal
{
    typedef FgMatrixC<double,dim,1>     Vec;
    typedef FgMatrixC<double,dim,dim>   Mat;

    Vec         mean;
    Mat         chol;       // Right-hand cholesky term of the concentration

    FgNormal() {chol.setIdentity(); }
    FgNormal(const Vec & m,const Mat & c) : mean(m), chol(c) {}

    double
    operator() (const Vec & pos) const
    {return fgMath::normalCholesky<dim>(pos,mean,chol); }

    Vec
    randomSample() const;

    double
    lnLikelihood(const Vec & pos) const
    {return fgMath::lnNormalCholesky<dim>(pos,mean,chol); }
};

typedef FgNormal<3> FgNormal3D;

template<uint dim>
std::ostream &
operator<<(std::ostream & ss,const FgNormal<dim> &  norm)
{
    return 
        ss  << fgnl << "Norm" << dim
            << " Mean: " << norm.mean() 
            << fgnl << "Cholesky: " << norm.cholesky();
}

#endif
