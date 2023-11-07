//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGMATH_HPP
#define FGMATH_HPP

#include "FgSerial.hpp"

namespace Fg {

template <typename T>
inline constexpr T  sqr(T a) {return (a*a); }
template <typename T>
inline T            cube(T a) {return (a*a*a); }
// Euclidean length (L2 norm):
template<typename T,size_t S>
double              cLen(std::array<T,S> const a) {return std::sqrt(cMag(a)); }
// Dot product is defined here as a recursive multiply-accumulate reduction on binary arguments
// of the same type that returns a (double) scalar:
inline double       cDot(double a,double b) {return a*b; }  // adding a float signature results in ambiguity
template<class T,size_t S>
T               cDot(Arr<T,S> const & lhs,Arr<T,S> const & rhs)
{
    double          ret {0};
    for (size_t ii=0; ii<S; ++ii)
        ret += cDot(lhs[ii],rhs[ii]);
    return ret;
}
template<class T>
double              cDot(Svec<T> const & v0,Svec<T> const & v1)
{
    double          acc {0};
    FGASSERT(v0.size() == v1.size());
    for (size_t ii=0; ii<v0.size(); ++ii)
        acc += cDot(v0[ii],v1[ii]);
    return acc;
}

template<class T>
double              cCos(Svec<T> const & v0,Svec<T> const & v1)
{
    double          mag = cMag(v0) * cMag(v1);
    FGASSERT(mag > 0.0);
    return cDot(v0,v1) / sqrt(mag);
}

template<typename T,FG_ENABLE_IF(T,is_integral)>
bool                isPow2(T val) {return ((val > 0) && ((val & (val-1)) == 0)); }

uint                numLeadingZeros(uint32 xx);         // 0 returns 32.
uint8               numNonzeroBits8(uint8 xx);
uint16              numNonzeroBits16(uint16 xx);
uint                numNonzeroBits32(uint32 xx);
inline uint         log2Floor(uint32 xx) {return (31-numLeadingZeros(xx)); }    // Not valid for 0.
uint                log2Ceil(uint32 xx);                                        // "
inline uint         pow2Ceil(uint32 xx){return (1 << log2Ceil(xx)); }           // "

// Safe version of 'exp' that throws when values fall outside type bounds:
template<typename T>
T                   expSafe(T val,bool clampVal=false)
{
    static T            maxIn = std::log(std::numeric_limits<T>::max());
    if (std::abs(val) < maxIn)
        return std::exp(val);
    FGASSERT(clampVal);
    if (val < maxIn)
        return -std::numeric_limits<T>::max();
    return std::numeric_limits<T>::max();
}

// Fast exp with no check for NaNs, overflow or underflow.
// This was necessary as GNU's libm 'exp' (used by gcc and clang) is very slow.
// (Microsoft's is actually a bit faster than this one):
double              expFast(double x);

// Returns one of {-1,0,1}. Branchless.
// Not compatible with FP positive/negative for 0/Inf/Nan.
// Use the slower std::sgnbit and std::copysign for that.
template<typename T>
int                 cmpTernary(T val) {return (T(0) < val) - (val < T(0)); }

// std::fmod gives a remainder not a modulus (ie it can be negative):
template<typename T>
T                   cMod(T val,T divisor)
{
    T               div = std::floor(val / divisor);
    return val - divisor * div;
}

// constexpr functions are always inline:
double constexpr    pi()        {return 3.141592653589793237462643; }
double constexpr    ln2Pi()     {return 1.837877066409345483560659; }
double constexpr    sqrt2Pi()   {return 2.506628274631000502415765; }
double constexpr    exp1()      {return 2.718281828459045235360287; }
double constexpr    degToRad(double degrees) {return degrees * pi() / 180.0; }
float  constexpr    degToRad(float degrees) {return degrees * 3.14159265f / 180.0f; }

struct      Modulo
{
    size_t          val;        // Invariant: [0,mod)
    size_t          mod;

    Modulo() {}
    Modulo(size_t v,size_t m) : val(v), mod(m) {FGASSERT(val < mod); }

    void            operator++()
    {
        ++val;
        if (val == mod)
            val = 0;
    }

    void            operator--()
    {
        if (val == 0)
            val = mod;
        --val;
    }
};

typedef Svec<Modulo>    Modulos;

inline double       cSsd(uchar l,uchar r) {return sqr(double(l)-double(r)); }
inline double       cSsd(float l,float r) {return sqr(l-r); }
inline double       cSsd(double l,double r) {return sqr(l-r); }

template<class T,size_t S>
double              cSsd(Arr<T,S> const & l,Arr<T,S> const & r)
{
    FGASSERT(l.size() == r.size());
    double          acc = 0.0;
    for (size_t ii=0; ii<l.size(); ++ii)
        acc += cSsd(l[ii],r[ii]);
    return acc;
}
template<class T>
double              cSsd(Svec<T> const & v0,Svec<T> const & v1)    // Sum of square differences
{
    FGASSERT(v0.size() == v1.size());
    double          acc = 0.0;
    for (size_t ii=0; ii<v0.size(); ++ii)
        acc += cSsd(v0[ii],v1[ii]);
    return acc;
}

template<class T>
double              cRms(Svec<T> const & v) {return std::sqrt(cMag(v) / v.size()); }
template<class T,size_t S>
double              cRms(Arr<T,S> const & a) {return cMag(a) / double(S); }

template<class T,size_t S>
Arr<T,S>            mapAbs(const Arr<T,S> & a)
{
    Arr<T,S>     ret;
    for (size_t ii=0; ii<S; ++ii)
        ret[ii] = std::abs(a[ii]);
    return ret;
}

// Useful for min/max with different types:
inline uint64       cMin(uint64 a,uint b) {return std::min(a,uint64(b)); }
inline uint64       cMin(uint a,uint64 b) {return std::min(uint64(a),b); }

// 1D convolution with zero-value boundary handling (non-optimized):
Doubles             convolve(
    Doubles const &         data,
    Doubles const &         kernel);    // Must be odd size with convolution centre in middle.

// 1D Gaussian convolution for large kernels (direct sampled kernel)
// with zero-value boundary handling:
Doubles             convolveGauss(
    Doubles const &         in,
    double                  stdev); // Kernel stdev relative to sample spacing.

// Gives relative difference of 'b' vs 'a' with a minimum given scale.
// Exact in the limit of small differences above the minimum scale (see below).
// Limit values of +/- 2 for very different values.
// 'minAbs' is used as the minimum scale for determining relative difference
// (important when one value may be very small or zero).
double              cRelDiff(double a,double b,double minAbs=lims<double>::epsilon());
Doubles             cRelDiff(Doubles const & a,Doubles const & b,double minAbs=lims<double>::epsilon());

Doubles             solveCubicReal(double c0,double c1,double c2);
// Z-order curve aka Lebesgue curve aka Morton number
// interleave bits of v0,v1,v2 respectively right to left. Values must all be < 2^10.
size_t              zorder(size_t v0,size_t v1,size_t v2);
inline double       logistic(double x) {return 1.0 / (1.0 + exp(-x)); } // Logistic function: x{R} -> (0,1)
double              logit(double f);                                    // & inverse: f(0,1) -> R
inline double       sigmoidq(double x) {return x / sqrt(1+sqr(x)); }    // signed quadratic sigmoid: x{R} -> (-1,1)
double              sigmoidqInv(double f);                              // & inverse: f(-1,1) -> R

}

#endif
