//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGMATH_HPP
#define FGMATH_HPP

#include "FgStdVector.hpp"
#include "FgStdArray.hpp"

namespace Fg {

template <typename T>
inline T
sqr(T a)
{return (a*a); }

// Euclidean length (L2 norm):
template<typename T,size_t S>
double
cLen(std::array<T,S> const a)
{return std::sqrt(cMag(a)); }

// Dot product function base case:
inline double
cDot(double a,double b) {return a*b; }

template<class T>
double
cDot(Svec<T> const & v0,Svec<T> const & v1)
{
    double      acc(0);
    FGASSERT(v0.size() == v1.size());
    for (size_t ii=0; ii<v0.size(); ++ii)
        acc += cDot(v0[ii],v1[ii]);
    return acc;
}

template<class T>
double
cCos(Svec<T> const & v0,Svec<T> const & v1)
{
    double      mag = cMag(v0) * cMag(v1);
    FGASSERT(mag > 0.0);
    return cDot(v0,v1) / sqrt(mag);
}

template<typename T,
    FG_ENABLE_IF(T,is_unsigned),
    FG_ENABLE_IF(T,is_integral)
>
bool
isPow2(T val)
{return ((val != 0) && ((val & (val-1)) == 0)); }

uint
numLeadingZeros(uint32 xx);                   // 0 returns 32.

uint8
numNonzeroBits8(uint8 xx);
uint16
numNonzeroBits16(uint16 xx);
uint
numNonzeroBits32(uint32 xx);

inline
uint
log2Floor(uint32 xx)                          // Not valid for 0.
{return (31-numLeadingZeros(xx)); }

uint
log2Ceil(uint32 xx);                          // Not valid for 0.

inline
uint
pos2Floor(uint32 xx)                        // Not valid for 0.
{return (1 << log2Floor(xx)); }

inline
uint
pow2Ceil(uint32 xx)
{return (1 << log2Ceil(xx)); }                // Not valid for 0.

template <typename T>
inline T
cube(T a)
{return (a*a*a); }

// Safe version of 'exp' that throws when values fall outside type bounds:
template<typename T>
T
expSafe(T val,bool clampVal=false)
{
    static T    maxIn = std::log(std::numeric_limits<T>::max());
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
double          expFast(double x);

// Returns one of {-1,0,1}. Branchless.
// Not compatible with FP positive/negative for 0/Inf/Nan.
// Use the slower std::sgnbit and std::copysign for that.
template<typename T>
int
cmpTernary(T val)
{return (T(0) < val) - (val < T(0)); }

// std::fmod gives a remainder not a modulus (ie it can be negative):
template<typename T>
T
cMod(T val,T divisor)
{
    T       div = std::floor(val / divisor);
    return val - divisor * div;
}

// constexpr functions are always inline:
double constexpr    pi()        {return 3.141592653589793237462643; }
double constexpr    ln2Pi()     {return 1.837877066409345483560659; }
double constexpr    sqrt2Pi()   {return 2.506628274631000502415765; }
double constexpr    exp1()      {return 2.718281828459045235360287; }
double constexpr    degToRad(double degrees) {return degrees * pi() / 180.0; }
float  constexpr    degToRad(float degrees) {return degrees * 3.14159265f / 180.0f; }

struct   Modulo
{
    size_t      val;        // Invariant: [0,mod)
    size_t      mod;

    Modulo() {}

    Modulo(size_t v,size_t m) : val(v), mod(m)
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

typedef Svec<Modulo> Modulos;

inline double cSsd(double l,double r) {return sqr(l-r); }
template<class T>
double
cSsd(Svec<T> const & v0,Svec<T> const & v1)    // Sum of square differences
{
    FGASSERT(v0.size() == v1.size());
    double          acc = 0.0;
    for (size_t ii=0; ii<v0.size(); ++ii)
        acc += cSsd(v1[ii],v0[ii]);
    return acc;
}
template<class T>
double
cSsd(Svec<T> const & vec,T const & val)          // Sum of square differences with a constant val
{
    double          acc = 0.0;
    for (size_t ii=0; ii<vec.size(); ++ii)
        acc += cSsd(vec[ii],val);
    return acc;
}
template<class T,size_t S>
double
cSsd(Arr<T,S> const & l,Arr<T,S> const & r)
{
    FGASSERT(l.size() == r.size());
    double          acc = 0.0;
    for (size_t ii=0; ii<l.size(); ++ii)
        acc += cSsd(l[ii],r[ii]);
    return acc;
}

template<class T>
double
cRms(Svec<T> const & v)                          // Root mean squared
{return std::sqrt(cMag(v) / v.size()); }

template<class T,size_t S>
double
cRms(Arr<T,S> const & a)
{return cMag(a) / double(S); }

// Useful for recursive template stub, 3-arg min/max, and when windows.h is included (has min/max macros):
template<class T>
inline T cMax(T x1,T x2) {return std::max(x1,x2); }
template<class T> 
inline T cMax(T x1,T x2,T x3) {return std::max(std::max(x1,x2),x3); }
template<class T> 
inline T cMax(T x1,T x2,T x3,T x4) {return std::max(std::max(x1,x2),std::max(x3,x4)); }
template<class T>
inline T cMin(T x1,T x2) {return std::min(x1,x2); }
template<class T>
inline T cMin(T x1,T x2,T x3) {return std::min(std::min(x1,x2),x3); }

template<class T,size_t S>
inline T cMax(const Arr<T,S> & a) {return *std::max_element(a.begin(),a.end()); }
template<class T,size_t S>
inline T cMin(const Arr<T,S> & a) {return *std::min_element(a.begin(),a.end()); }
template<typename T>
T
cMin(Svec<T> const & v)
{
    FGASSERT(!v.empty());
    return *std::min_element(v.begin(),v.end());
}
template<class T>
T
cMax(Svec<T> const & v)
{
    FGASSERT(!v.empty());
    return *std::max_element(v.begin(),v.end());
}

template<class T,size_t S>
Arr<T,S> mapAbs(const Arr<T,S> & a)
{
    Arr<T,S>     ret;
    for (size_t ii=0; ii<S; ++ii)
        ret[ii] = std::abs(a[ii]);
    return ret;
}

// Useful for min/max with different types:
inline uint64 cMin(uint64 a,uint b) {return std::min(a,uint64(b)); }
inline uint64 cMin(uint a,uint64 b) {return std::min(uint64(a),b); }

// Avoid extra typing:
inline double constexpr epsilonD() {return std::numeric_limits<double>::epsilon(); }
inline float constexpr maxDouble() {return std::numeric_limits<double>::max(); }
inline float constexpr maxFloat() {return std::numeric_limits<float>::max(); }

// 1D convolution with zero-value boundary handling (non-optimized):
Doubles
convolve(
    Doubles const &         data,
    Doubles const &         kernel);    // Must be odd size with convolution centre in middle.

// 1D Gaussian convolution for large kernels (direct sampled kernel)
// with zero-value boundary handling:
Doubles
convolveGauss(
    Doubles const &         in,
    double                  stdev); // Kernel stdev relative to sample spacing.

// Gives relative difference of 'b' vs 'a' with a minimum given scale.
// Exact in the limit of small differences above the minimum scale (see below).
// Limit values of +/- 2 for very different values.
// 'minAbs' is used as the minimum scale for determining relative difference
// (important when one value may be very small or zero).
inline
double
cRelDiff(double a,double b,double minAbs=epsilonD())
{
    double      del = b-a,
                denom = std::abs(b)+std::abs(a);
    if (denom == 0.0)
        return 0.0;
    else if (denom < minAbs)
        denom = minAbs;
    return del * 2.0 / denom;
}

Doubles
cRelDiff(Doubles const & a,Doubles const & b,double minAbs=epsilonD());

// Return all subsets of elements of v in the given set size range. Retains order. Assumes all elements of v are different.
template<class T>
Svec<Svec<T> >
cSubsets(Svec<T> const & v,size_t min,size_t max)
{
    Svec<Svec<T> >      ret;
    if (!v.empty() && (max>=min)) {
        FGASSERT(v.size() < 30);                            // Sanity check
        uint32              sv = uint32(v.size());
        uint32              sz = 1UL << sv;                 // EUB for bit field
        for (uint32 ii=0; ii<sz; ++ii) {
            uint32          nzb = numNonzeroBits32(ii);
            if ((nzb >= min) && (nzb <= max)) {
                Svec<T>       r;
                for (uint32 jj=0; jj<sv; ++jj)
                    if (ii & (1 << jj))
                        r.push_back(v[jj]);
                ret.push_back(r);
            }
        }
    }
    return ret;
}

Doubles
solveCubicReal(double c0,double c1,double c2);

// Z-order curve aka Lebesgue curve aka Morton number
// interleave bits of v0,v1,v2 respectively right to left. Values must all be < 2^10.
size_t  zorder(size_t v0,size_t v1,size_t v2);

}

#endif
