//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     June 22, 2005
//

#ifndef FGMATH_HPP
#define FGMATH_HPP

#include "FgStdLibs.hpp"
#include "FgTypes.hpp"
#include "FgMatrixCBase.hpp"
#include "FgAlgs.hpp"

template <typename T>
inline T
fgSqr(T a)
{return (a*a); }

inline
bool
fgIsPow2(uint xx)
{return ((xx != 0) && ((xx & (xx-1)) == 0)); }

uint
fgNumLeadingZeros(uint32 xx);                   // 0 returns 32.

inline
uint
fgLog2Floor(uint32 xx)                          // Not valid for 0.
{return (31-fgNumLeadingZeros(xx)); }

uint
fgLog2Ceil(uint32 xx);                          // Not valid for 0.

inline
uint
fgPower2Floor(uint32 xx)                        // Not valid for 0.
{return (1 << fgLog2Floor(xx)); }

inline
uint
fgPower2Ceil(uint32 xx)
{return (1 << fgLog2Ceil(xx)); }                // Not valid for 0.

template <typename T>
inline T
fgCube(T a)
{return (a*a*a); }

template <typename T>
inline T
fgCbrt(T a)
{return T(std::pow(double(a),1.0/3.0)); }    // TODO: faster version if required (eg. C++11 cbrt)

template<typename T>
T
fgExp(T val,bool clamp=false)
{
    static T    maxIn = std::log(std::numeric_limits<T>::max());
    if (std::abs(val) < maxIn)
        return std::exp(val);
    FGASSERT(clamp);
    if (val < maxIn)
        return -std::numeric_limits<T>::max();
    return std::numeric_limits<T>::max();
}

template<typename T>
T
fgSign(T val)
{return (val < T(0)) ? T(-1) : T(1); }

// std::fmod gives a remainder not a modulus (ie it can be negative):
template<typename T>
T
fgMod(T val,T divisor)
{
    T       div = std::floor(val / divisor);
    return val - divisor * div;
}

inline double   fgPi()         {return 3.141592653589793237462643; }
inline double   fgLn_pi()      {return 1.144729885849400173825117; }
inline double   fgLn_2()       {return 0.693147180559945309417232; }
inline double   fgLn_2pi()     {return 1.837877066409345483560659; }
inline double   fgSqrt_2pi()   {return 2.506628274631000502415765; }
inline double   fgRadToDeg(double radians) {return radians * 180.0 / fgPi(); };
inline double   fgDegToRad(double degrees) {return degrees * fgPi() / 180.0; };
inline float    fgRadToDeg(float radians) {return radians * 180.0f / 3.14159265f; };
inline float    fgDegToRad(float degrees) {return degrees * 3.14159265f / 180.0f; };

namespace fgMath
{

// Functions:

std::vector<double>
solveCubicReal(double c0,double c1,double c2);

// Distributions:

double
normal(double val,double mean,double stdev);

double
lnNormal(double val,double mean,double stdev);

double
lnNormalIid(
    double  dimension,      // Usually an integer corresponding to number of IID measures
    double  ssd,            // Sum of square differences between measures and means
    double  stdev);         // IID standard deviation

double
lnNormalIidIgMapConstDim(   // Use ignorance MAP value for variance and ignore dimension-const terms
    double  dimension,      // Number of IID measures
    double  ssd);           // Sum of square differences between measures and means

template<uint dim>
double
normalCholesky(
    const FgMatrixC<double,dim,1> &     pos,
    const FgMatrixC<double,dim,1> &     mean,
    const FgMatrixC<double,dim,dim> &   chol)   // Right-hand cholesky term of concentration
{
    double  det = 1.0;
    for (uint ii=0; ii<dim; ii++)
        det *= chol.elem(ii,ii);                // Cholesky is upper or lower triangular.
    FgMatrixC<double,dim,1> mhlbs = chol * (pos-mean);
    return (
        std::pow(2.0 * fgPi(),double(dim) * -0.5) *
        std::sqrt(det) *
        fgExp(-0.5 * mhlbs.lengthSqr()));
}

template<uint dim>
double
lnNormalCholesky(
    const FgMatrixC<double,dim,1> &     pos,
    const FgMatrixC<double,dim,1> &     mean,
    const FgMatrixC<double,dim,dim> &   chol)
{
    double  det = 1.0;
    for (uint ii=0; ii<dim; ii++)
        det *= chol.elem(dim,dim);
    FgMatrixC<double,dim,1> mhlbs = chol * (pos-mean);
    return (0.5 * std::log(det) -               // Cholesky has all diagonals > 0
            0.5 * double(dim) * fgLn_2pi() -
            0.5 * mhlbs.lengthSqr());
}

template<typename T,uint dim>
T
lnNormalIsotropic(
    FgMatrixC<T,dim,1>  val,
    FgMatrixC<T,dim,1>  mean,
    T                   stdev)
{
    FGASSERT_FAST(stdev > T(0));
    return (-0.5 * fgLn_2pi() * dim
            - std::log(stdev) * dim
            - 0.5 * (val-mean).lengthSqr() / fgSqr(stdev));
}

}   // namespace

#endif
