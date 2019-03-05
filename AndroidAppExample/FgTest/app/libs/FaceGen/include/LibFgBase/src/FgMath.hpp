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

uint8
fgNumNonzeroBits8(uint8 xx);
uint16
fgNumNonzeroBits16(uint16 xx);
uint
fgNumNonzeroBits32(uint32 xx);

inline
uint
fgLog2Floor(uint32 xx)                          // Not valid for 0.
{return (31-fgNumLeadingZeros(xx)); }

uint
fgLog2Ceil(uint32 xx);                          // Not valid for 0.

inline
uint
fgPow2Floor(uint32 xx)                        // Not valid for 0.
{return (1 << fgLog2Floor(xx)); }

inline
uint
fgPow2Ceil(uint32 xx)
{return (1 << fgLog2Ceil(xx)); }                // Not valid for 0.

template <typename T>
inline T
fgCube(T a)
{return (a*a*a); }

// Safe version of 'exp' that throws when values fall outside type bounds:
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

// Fast exp with no check for NaNs, overflow or underflow.
// This was necessary as GNU's libm 'exp' (used by gcc and clang) is very slow.
// (Microsoft's is actually a bit faster than this one):
double fgExpFast(double x);

// Returns one of {-1,0,1}. Branchless.
// Not compatible with FP positive/negative for 0/Inf/Nan.
// Use the slower std::sgnbit and std::copysign for that.
template<typename T>
int
fgSign(T val)
{return (T(0) < val) - (val < T(0)); }

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
inline double   fgRadToDeg(double radians) {return radians * 180.0 / fgPi(); }
inline double   fgDegToRad(double degrees) {return degrees * fgPi() / 180.0; }
inline float    fgRadToDeg(float radians) {return radians * 180.0f / 3.14159265f; }
inline float    fgDegToRad(float degrees) {return degrees * 3.14159265f / 180.0f; }

struct   FgModulo
{
    size_t      val;        // Invariant: [0,mod)
    size_t      mod;

    FgModulo() {}

    FgModulo(size_t v,size_t m) : val(v), mod(m)
    {FGASSERT(val < mod); }

    void
    operator++()
    {
        ++val;
        if (val == mod)
            val = 0;
    }

    void
    operator--()
    {
        if (val == 0)
            val = mod;
        --val;
    }
};

typedef std::vector<FgModulo> FgModulos;

// Return all subsets of elements of v in the given set size range. Retains order. Assumes all elements of v are different.
template<class T>
vector<vector<T> >
fgSubsets(const vector<T> & v,size_t min,size_t max)
{
    vector<vector<T> >      ret;
    if (!v.empty() && (max>=min)) {
        FGASSERT(v.size() < 30);                            // Sanity check
        uint32              sv = uint32(v.size());
        uint32              sz = 1UL << sv;                 // EUB for bit field
        for (uint32 ii=0; ii<sz; ++ii) {
            uint32          nzb = fgNumNonzeroBits32(ii);
            if ((nzb >= min) && (nzb <= max)) {
                vector<T>       r;
                for (uint32 jj=0; jj<sv; ++jj)
                    if (ii & (1 << jj))
                        r.push_back(v[jj]);
                ret.push_back(r);
            }
        }
    }
    return ret;
}

std::vector<double>
fgSolveCubicReal(double c0,double c1,double c2);

#endif
