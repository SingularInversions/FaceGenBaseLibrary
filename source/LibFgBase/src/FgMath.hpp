//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// math related functions not found in or extending the std lib:

#ifndef FGMATH_HPP
#define FGMATH_HPP

#include "FgSerial.hpp"

namespace Fg {

// Return the epsilon (relative to one) for the given number of bits of precision:
inline double constexpr epsBits(size_t bits)
{
    if (bits<2) throw FgException{"epsBits() arg too small",toStr(bits)};
    if (bits>63) throw FgException{"epsBits() arg too large",toStr(bits)};
    return 1.0 / scast<double>(1ULL << bits);
}

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
    double          mag = cMagD(v0) * cMagD(v1);
    FGASSERT(mag > 0.0);
    return cDot(v0,v1) / sqrt(mag);
}

template<typename T,FG_ENABLE_IF(T,is_integral)>
bool                isPow2(T val) {return ((val > 0) && ((val & (val-1)) == 0)); }

size_t              cNumDigits(size_t val);
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

// doubles can be converted to 17 decimals and back without error:
double constexpr    sqrt2 =     1.4142135623730950;
double constexpr    invSqrt2 =  0.70710678118654752;
double constexpr    pi =        3.1415926535897932;
double constexpr    tau =       6.2831853071795865;
double constexpr    lnTau =     1.8378770664093455;
double constexpr    sqrtTau =   2.5066282746310006;
double constexpr    degToRad(double degrees) {return degrees * pi / 180.0; }
float  constexpr    degToRad(float degrees) {return degrees * scast<float>(pi) / 180.0f; }

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

inline double       cSsd(uchar l,uchar r) {return sqr(scast<double>(l)-scast<double>(r)); }
inline double       cSsd(float l,float r) {return sqr(l-r); }       // no advantage in up-typing before subtraction
inline double       cSsd(double l,double r) {return sqr(l-r); }

template<class T,size_t S>
double              cSsd(Arr<T,S> const & l,Arr<T,S> const & r)
{
    double              acc {0};
    for (size_t ii=0; ii<S; ++ii)
        acc += cSsd(l[ii],r[ii]);
    return acc;
}
template<class T>
double              cSsd(Svec<T> const & v0,Svec<T> const & v1)    // Sum of square differences
{
    FGASSERT(v0.size() == v1.size());
    double              acc {0};
    for (size_t ii=0; ii<v0.size(); ++ii)
        acc += cSsd(v0[ii],v1[ii]);
    return acc;
}

template<class T>
double              cRms(Svec<T> const & v) {return std::sqrt(cMagD(v) / v.size()); }
template<class T,size_t S>
double              cRms(Arr<T,S> const & a) {return cMagD(a) / double(S); }

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

inline double       logistic(double x) {return 1.0 / (1.0 + exp(-x)); } // R -> (0,1)
double              logit(double l);                                    // (0,1) -> R
inline double       logistic2(double x) {return 2/(1+exp(-x))-1; }      // R -> (-1,1)
inline double       logistic2Slope(double x){double e = exp(-x); return 2*e/sqr(1+e); }
inline double       logit2(double l) {return logit(l/2 + 0.5); }        // (-1,1) -> R
inline double       sigmoidq(double x) {return x / sqrt(1+sqr(x)); }    // signed quadratic sigmoid: x{R} -> (-1,1)
double              sigmoidqInv(double f);                              // & inverse: f(-1,1) -> R
inline double       sigunitq(double x) {return 0.5 + 0.5 * sigmoidq(x); }   // as above but range (0,1)
inline double       sigunitqInv(double f) {return sigmoidqInv(2*f-1); }     // & inverse

// return the normalized (ie. 1 when x=inf) integral of exp{-1/2 * x^2} from -inf to 'stdPos':
inline double       integrateGaussian(double stdPos) {return std::erfc(-stdPos*invSqrt2) * 0.5; }

// 1D [121] smooth for any floating point aggregate:
template<class T>
Svec<T>             smoothOpen(Svec<T> const & v)       // preserves endpoint values
{
    size_t              S = v.size();
    Svec<T>             ret; ret.reserve(S);
    if (S==0) return ret;
    ret.push_back(v[0]);
    if (S==1) return ret;
    for (size_t ii=1; ii+1<S; ++ii)
        ret.push_back(v[ii-1]*0.25+v[ii]*.5+v[ii+1]*0.25);
    ret.push_back(v[S-1]);
    return ret;
}

// 1D closed manifold simple subdivision of sample values returns a vector of twice the size:
template<class T>
Svec<T>             subdivideSimpleClosed(Svec<T> const & v)
{
    size_t              S = v.size();
    Svec<T>             ret; ret.reserve(S*2);
    for (size_t ii=0; ii<S; ++ii) {
        T const &           e = v[ii];
        ret.push_back(e);
        ret.push_back((e + v[(ii+1)%S]) / 2);
    }
    return ret;
}
// 1D open manifold simple subdivision of sample values returns a vector size 2N-1:
template<class T>
Svec<T>             subdivideSimpleOpen(Svec<T> const & v)
{
    size_t              S = v.size();
    Svec<T>             ret;
    if (S == 0) return ret;
    ret.reserve(S*2-1);
    T const             *prev = &v[0],
                        *next = prev;
    for (size_t ii=1; ii<S; ++ii) {
        next = &v[ii];
        ret.push_back(*prev);
        ret.push_back((*prev + *next)/2);
        prev = next;
    }
    ret.push_back(*next);
    return ret;
}
// 1D open manifold smoothing subdivision of sample values returns a vector size 2N-1 (end points left fixed):
template<class T>
Svec<T>             subdivideSmoothOpen(Svec<T> const & v)
{
    size_t              S = v.size();
    Svec<T>             ret;
    if (S == 0) return ret;
    ret.reserve(S*2-1);
    ret.push_back(v[0]);                        // end point fixed
    if (S == 1) return ret;
    T const             *prev = &v[0],
                        *curr = &v[1],
                        *next = curr;
    for (size_t ii=2; ii<S; ++ii) {
        next = &v[ii];
        ret.push_back((*prev + *curr)*0.5);
        // control points move as [121] smooth with their [11] interpolated neighbours, which yields [161]:
        ret.push_back((*prev + (*curr)*6 + *next)*0.125);
        prev = curr;
        curr = next;
    }
    ret.push_back((*prev + *curr)*0.5);
    ret.push_back(*next);                       // end point fixed
    return ret;
}

void                randSeedRepeatable(uint64 seed=42);     // same seed gives same PRNG sequence
void                randSeedTime(); // Seed with current time in milliseconds to ensure different results each time
uint64              cRandUint64(uint64 eub=0);               // uniform number in [0,eub-1]:
double              cRandUniform(double lo=0,double hi=1);  // From the uniformly distributed range [lo,hi]
double              cRandNormal();                          // random number from standard normal distribution
inline Doubles      cRandNormals(size_t N,double mean=0,double stdev=1)
{
    return genSvec(N,[mean,stdev](size_t){return mean+stdev*cRandNormal(); });
}
template<class T,size_t S,FG_ENABLE_IF(T,is_floating_point)>
Arr<T,S>            cRandArrNormal(T mean=0,T stdev=1)
{
    auto                fn = [mean,stdev](size_t){return cRandNormal()*stdev + mean; };
    return genArr<T,S>(fn);
}
template<class T,size_t S,FG_ENABLE_IF(T,is_floating_point)>
Arr<T,S>            cRandArrUniform(T lo,T hi)
{
    auto                fn = [lo,hi](size_t){return cRandUniform(lo,hi); };
    return genArr<T,S>(fn);
}

std::string         randString(uint numChars);          // alphanumerics only (including capitals):
bool                randBool();
// Give random values near 1 or -1 with stdev 0.125 (avoids zero, handy for ACS testing):
double              randNearUnit();
Doubles             randNearUnits(size_t num);

template<size_t S>
Arr<double,S>       randNearUnitsArr()
{
    Arr<double,S>       ret;
    for (size_t ss=0; ss<S; ++ss)
        ret[ss] = randNearUnit();
    return ret;
}

Sizes               cRandPermutation(size_t S);     // Returns indices of a random permutation of S elements

}

#endif
