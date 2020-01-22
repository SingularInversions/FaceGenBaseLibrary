//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
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

// Squared magnitude function base cases:
inline float cMag(float v) {return v*v; }
inline double cMag(double v) {return v*v; }
inline double cMag(std::complex<double> v) {return std::norm(v); }
template<typename T,size_t S>
double
cMag(std::array<T,S> const a)
{
    T       acc(0);
    for (T const & e : a)
        acc += cMag(e);
    return acc;
}
template<class T>
double
cMag(const Svec<T> & v)              // Sum of squared magnitude values:
{
    double      ret(0);
    for (size_t ii=0; ii<v.size(); ++ii)
        ret += cMag(v[ii]);
    return ret;
}

// Euclidean length (L2 norm):
template<typename T,size_t S>
double
cLen(std::array<T,S> const a)
{return std::sqrt(cMag(a)); }
template<class T>
double
cLen(const Svec<T> & v)
{return std::sqrt(cMag(v)); }

// Dot product function base case:
inline double
cDot(double a,double b) {return a*b; }

template<class T>
double
cDot(const Svec<T> & v0,const Svec<T> & v1)
{
    double      acc(0);
    FGASSERT(v0.size() == v1.size());
    for (size_t ii=0; ii<v0.size(); ++ii)
        acc += cDot(v0[ii],v1[ii]);
    return acc;
}

inline
bool
isPow2(uint xx)
{return ((xx != 0) && ((xx & (xx-1)) == 0)); }

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
expSafe(T val,bool clamp=false)
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

inline double constexpr pi()        {return 3.141592653589793237462643; }
inline double constexpr ln2Pi()     {return 1.837877066409345483560659; }
inline double constexpr sqrt2Pi()   {return 2.506628274631000502415765; }
inline double   degToRad(double degrees) {return degrees * pi() / 180.0; }
inline float    degToRad(float degrees) {return degrees * 3.14159265f / 180.0f; }

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

template<class T>
double
cSsd(const Svec<T> & v0,const Svec<T> & v1)    // Sum of square differences
{
    FGASSERT(v0.size() == v1.size());
    double      acc = 0;
    for (size_t ii=0; ii<v0.size(); ++ii)
        acc += cMag(v1[ii]-v0[ii]);
    return acc;
}

template<class T>
double
cSsd(const Svec<T> & vec,const T & val)          // Sum of square differences with a constant val
{
    double      acc = 0;
    for (size_t ii=0; ii<vec.size(); ++ii)
        acc += cMag(vec[ii]-val);
    return acc;
}

template<class T>
double
cRms(const Svec<T> & v)                          // Root mean squared
{return std::sqrt(cMag(v) / v.size()); }

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
cMin(const Svec<T> & v)
{
    FGASSERT(!v.empty());
    return *std::min_element(v.begin(),v.end());
}
template<class T>
T
cMax(const Svec<T> & v)
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
inline double epsilonD() {return std::numeric_limits<double>::epsilon(); }
inline float maxFloat() {return std::numeric_limits<float>::max(); }

// 1D convolution with zero-value boundary handling (non-optimized):
Svec<double>
fgConvolve(
    const Doubles &         data,
    const Doubles &         kernel);    // Must be odd size with convolution centre in middle.

// 1D Gaussian convolution for large kernels (direct sampled kernel)
// with zero-value boundary handling:
Svec<double>
fgConvolveGauss(
    const Doubles &         in,
    double                  stdev); // Kernel stdev relative to sample spacing.

// Gives relative difference of 'b' vs 'a'.
// Exact in the limit of small differences above the minimum scale (see below).
// Limit values of +/- 2 for very different values.
// 'minAbs' is used as the minimum scale for determining relative difference
// (important when one value may be very small or zero).
inline
double
fgRelDiff(double a,double b,double minAbs=epsilonD())
{
    double      del = b-a,
                denom = std::abs(b)+std::abs(a);
    denom = (denom < minAbs) ? minAbs : denom;
    return del * 2.0 / denom;
}

Svec<double>
fgRelDiff(const Svec<double> & a,const Svec<double> & b,double minAbs=epsilonD());

// Return all subsets of elements of v in the given set size range. Retains order. Assumes all elements of v are different.
template<class T>
Svec<Svec<T> >
fgSubsets(const Svec<T> & v,size_t min,size_t max)
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

Svec<double>
fgSolveCubicReal(double c0,double c1,double c2);

}

#endif
