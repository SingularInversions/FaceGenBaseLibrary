//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgMath.hpp"
#include "FgCommand.hpp"
#include "FgImgDisplay.hpp"
#include "FgImageDraw.hpp"
#include "FgTime.hpp"

using namespace std;

namespace Fg {

size_t              cNumDigits(size_t val)
{
    size_t              ret = 1;
    while ((val /= 10) > 0)
        ++ret;
    return ret;
}

// Without a hardware nlz (number of leading zeros) instruction, this function has
// to be iterative (C / C++ doesn't have any keyword for this operator):
uint                numLeadingZeros(uint32 x)
{
   uint     n = 0;
   if (x == 0) return(32);
   if (x <= 0x0000FFFF) {n = n +16; x = x <<16;}
   if (x <= 0x00FFFFFF) {n = n + 8; x = x << 8;}
   if (x <= 0x0FFFFFFF) {n = n + 4; x = x << 4;}
   if (x <= 0x3FFFFFFF) {n = n + 2; x = x << 2;}
   if (x <= 0x7FFFFFFF) {n = n + 1;}
   return n;
}

uint8               numNonzeroBits8(uint8 xx)
{
    return
        (xx & 1U) + (xx & 2U)/2 + (xx & 4U)/4 + (xx & 8U)/8 +
        (xx & 16U)/16 + (xx & 32U)/32 + (xx & 64U)/64 + (xx & 128U)/128;
}
uint16              numNonzeroBits16(uint16 xx)
{
    return numNonzeroBits8(xx & 255) + numNonzeroBits8(xx >> 8);
}
uint                numNonzeroBits32(uint32 xx)
{
    return
        numNonzeroBits8(xx & 255) + numNonzeroBits8((xx >> 8) & 255) +
        numNonzeroBits8((xx >> 16) & 255) + numNonzeroBits8((xx >> 24));
}

uint                log2Ceil(uint32 xx)
{
    uint    logFloor = log2Floor(xx);
    if (xx == (1u << logFloor))
        return (log2Floor(xx));
    else
        return (log2Floor(xx) + 1u);
}

// RETURNS: Between 1 and 3 real roots of the equation. Duplicate roots are returned in duplicate.
// The cubic term co-efficient is assumed to be 1.0.
// From Spiegel '99, Mathematical Handbook of Formulas and Tables.
Doubles             solveCubicReal(
    double          c0,         // constant term
    double          c1,         // first order coefficient
    double          c2)         // second order coefficient
{
    Doubles         retval;

    double      qq = (3.0 * c1 - sqr(c2)) / 9.0,
                rr = (9.0 * c1 * c2 - 27.0 * c0 - 2.0 * cube(c2)) / 54.0,
                dd = cube(qq) + sqr(rr);

    if (dd > 0.0) {                     // Only one real root
        double          sqdd = sqrt(dd),
                        ss = cbrt(rr + sqdd),
                        tt = cbrt(rr - sqdd);
        retval.push_back(ss+tt-(c2/3.0));
        return retval;
    }
    else if (dd == 0.0) {               // All real roots, at least 2 equal
        double          ss = cbrt(rr);
        retval.push_back(2.0 * ss - c2 / 3.0);
        retval.push_back(-2.0 * ss - c2 / 3.0);
        retval.push_back(-2.0 * ss - c2 / 3.0);
    }
    else {                              // dd < 0, all distinct real roots
        double          ss = 2.0 * sqrt(-qq),
                        theta3 = acos(rr / pow(-qq,1.5)) / 3.0,
                        c23 = c2 / 3.0;
        retval.push_back(ss * cos(theta3) - c23);
        retval.push_back(ss * cos(theta3 + 2.0 * pi / 3.0) - c23);
        retval.push_back(ss * cos(theta3 - 2.0 * pi / 3.0) - c23);
    }
    return retval;
}

Doubles             convolve(Doubles const & data,Doubles const & kernel)
{
    FGASSERT(data.size() * kernel.size() > 0);
    FGASSERT((kernel.size() % 2) == 1);
    size_t              kbase = kernel.size()/2;
    vector<double>      ret(data.size(),0.0);
    for (size_t ii=0; ii<ret.size(); ++ii) {
        for (size_t jj=kernel.size()-1; jj<kernel.size(); --jj) {
            size_t      idx = ii + kbase - jj;
            if (idx < data.size())
                ret[ii] += data[idx] * kernel[jj];
        }
    }
    return ret;
}

Doubles             convolveGauss(Doubles const & in,double stdev)
{
    FGASSERT(stdev > 0.0);
    // Create kernel w/ 6 stdevs on each side for double since this is 2 parts in 1B:
    size_t          ksize = roundT<uint>(stdev * 6);
    if (ksize == 0)
        return in;
    vector<double>  kernel(ksize*2+1);
    for (size_t ii=0; ii<ksize; ++ii) {
        double      val = expSafe(-0.5*sqr(ii/stdev));
        kernel[ksize+ii] = val;
        kernel[ksize-ii] = val;
    }
    double fac = 1.0 / cSum(kernel);
    for (size_t ii=0; ii<kernel.size(); ++ii)
        kernel[ii] *= fac;
    return convolve(in,kernel);
}

double              cRelDiff(double a,double b,double minAbs)
{
    double      del = b-a,
                denom = std::abs(b)+std::abs(a);
    if (denom == 0.0)
        return 0.0;
    else if (denom < minAbs)
        denom = minAbs;
    return del * 2.0 / denom;
}

Doubles             cRelDiff(Doubles const & a,Doubles const & b,double minAbs)
{
    Doubles         ret;
    FGASSERT(a.size() == b.size());
    ret.resize(a.size());
    for (size_t ii=0; ii<a.size(); ++ii)
        ret[ii] = cRelDiff(a[ii],b[ii],minAbs);
    return ret;
}

size_t              zorder(size_t v0,size_t v1,size_t v2)
{
    size_t          ret = 0;
    for (size_t ii=0; ii<10; ++ii) {
        size_t      sel = size_t(1) << ii,
                    shift = ii*2;           // src pos increments by 1, dest by 3, so difference by 2
        ret |=  ((v0 & sel) << shift) |
                ((v1 & sel) << (shift+1)) |
                ((v2 & sel) << (shift+2));
    }
    return ret;
}

double              logit(double f)
{
    FGASSERT((f > 0.0) && (f < 1.0));
    return log(f / (1.0-f));
}
double              sigmoidqInv(double f)
{
    double          ff = sqr(f);
    FGASSERT(ff < 1.0);
    return f / sqrt(1-ff);
}

// The following code is a modified part of the 'fastermath' library:
/* 
   Copyright (c) 2012,2013   Axel Kohlmeyer <akohlmey@gmail.com> 
   All rights reserved.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   * Neither the name of the <organization> nor the
     names of its contributors may be used to endorse or promote products
     derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
typedef union 
{
    double                      dbl;
    struct {int32_t  lo,hi;}    asInt;
}  ExpFast;
double              expFast(double x)
{
    x *=  1.4426950408889634074;        // Convert to base 2 exponent
    double              ipart = floor(x+0.5),
                        fpart = x - ipart;
    ExpFast             epart;
    epart.asInt.lo = 0;
    epart.asInt.hi = (((int) ipart) + 1023) << 20;
    double              y = fpart*fpart,
                        px = 2.30933477057345225087e-2;
    px = px*y + 2.02020656693165307700e1;
    double              qx = y + 2.33184211722314911771e2;
    px = px*y + 1.51390680115615096133e3;
    qx = qx*y + 4.36821166879210612817e3;
    px = px * fpart;
    y = 1.0 + 2.0*(px/(qx-px));
    return epart.dbl*y;
}
// End of 'fastermath' library.


// 'random_device' uses time and other system information to create a seed:
static mt19937_64   rng {random_device{}()};

uint64              cRandUint64(uint64 eub)
{
    if (eub==0)
        return rng();
    else if (eub==1)
        return 0;
    // Lightweight class, not a performance issue to construct each time:
    uniform_int_distribution<uint64> d(0,eub-1);     // Convert from inclusive to exclusive upper bound
    return d(rng);
}
void                randSeedRepeatable(uint64 seed) {rng.seed(seed); }
void                randSeedTime() {rng.seed(getTimeMs()); }

double              cRandUniform(double lo,double hi)
{
    double              rat = scast<double>(rng()) / scast<double>(numeric_limits<uint64>::max());
    return (rat * (hi-lo) + lo);
}

double              cRandNormal()
{
    // Polar (Box-Muller) method; See Knuth v2, 3rd ed, p122.
    double  x, y, r2;
    do {
        x = -1 + 2 * cRandUniform();
        y = -1 + 2 * cRandUniform();
        // see if it is in the unit circle:
        r2 = x * x + y * y;
    }
    while (r2 > 1.0 || r2 == 0);
    // Box-Muller transform:
    return y * sqrt (-2.0 * log (r2) / r2);
}

static char         randChar()
{
    uint    val = cRandUint64(26U+26U+10U);
    if (val < 10)
        return char(48+val);
    val -= 10;
    if (val < 26)
        return char(65+val);
    val -= 26;
    FGASSERT(val < 26);
    return char(97+val);
}

string              randString(uint numChars)
{
    string  ret;
    for (uint ii=0; ii<numChars; ++ii)
        ret = ret + randChar();
    return ret;
}

bool                randBool() {return (rng() & 0x01ULL); }

double              randNearUnit()
{
    double              unit = randBool() ? 1.0 : -1.0;
    return unit + cRandNormal()*0.125;
}

Doubles             randNearUnits(size_t num)
{
    return genSvec(num,[](size_t){return randNearUnit(); });
}

Sizes               cRandPermutation(size_t S)
{
    // simplest algorithm since faster ones are complicated:
    Sizes               ret; ret.reserve(S);
    Sizes               orig = genIntegers<size_t>(S);
    while (!orig.empty()) {
        size_t              idx = cRandUint64(orig.size());
        ret.push_back(orig[idx]);
        orig.erase(orig.begin()+idx);
    }
    return ret;
}

namespace {

void                testGauss(CLArgs const &)
{
    PushIndent          pind {"Gaussian with mean 0, stdev 1 : exp(-1/2 x^2)"};
    fgout << std::fixed << std::setprecision(7);
    for (size_t ii=0; ii<7; ++ii)
        fgout << fgnl << "X: " << ii << " stdev: " <<  exp(-0.5*sqr(ii))
            << " cumulative: " << erf(1.0*ii) * pi/4
            << " normalized cumulative: " << erf(1.0*ii)/2
            << " std::erf: " << erf(1.0*ii);
}

void                testLogistic(CLArgs const & args)
{
    if (isAutomated(args))
        return;
    viewImage(cGraphFunctions({logistic,logistic2,logistic2Slope},Arr2D{-5,5}));
}

void                testRand(CLArgs const &)
{
    randSeedRepeatable();
    size_t constexpr    S = 1ULL << 20;                 // ~1M samps
    Doubles             vals = genSvec(S,[](size_t){return cRandUniform(); });
    double              mean = cMean(vals),
                        meanSqr = cMag(vals)/vals.size();
    fgout << fgnl << "Mean: " << mean << fgnl << "MeanSqr: " << meanSqr;
    FGASSERT(isApproxEqual(mean,0.5,epsBits(7)));           // mean should be ~1/2 (+/- ~epsBits(10))
    FGASSERT(isApproxEqual(meanSqr,1/3.0,epsBits(7)));      // mean sqr should be ~1/3
}

void                testZorder(CLArgs const &)
{
    size_t      v0 = 0x01,
                v1 = 0x02,
                v2 = 0x04,
                r0 = zorder(v0,v1,v2),
                r1 = v0 | (v1 << 3) | (v2 << 6);
    FGASSERT(r0 == r1);
}

void                normGraph(CLArgs const & args)
{
    fgout << fgnl << "sizeof(RNG) = " << sizeof(rng);
    // Create a histogram of normal samples:
    randSeedRepeatable();
    size_t              numStdevs = 6,
                        binsPerStdev = 50,
                        S = 1000000,
                        sz = numStdevs * 2 * binsPerStdev;
    Sizes               histogram(sz,0);
    Affine1D            randToHist {-double(numStdevs),double(numStdevs),0,double(sz)};
    for (size_t ii=0; ii<S; ++ii) {
        int                 rnd = roundT<int>(randToHist * cRandNormal());
        if ((rnd >= 0) && (rnd < int(sz)))
            ++histogram[rnd];
    }
    // Make a bar graph of it:
    // S = binScale * stdNormIntegral * binsPerStdev
    double              binScale = double(S) / (sqrtTau * binsPerStdev),
                        hgtRatio = 0.9;
    ImgRgba8             img {sz,sz,Rgba8{0}};
    for (size_t xx=0; xx<sz; ++xx) {
        size_t              hgt = roundT<int>(histogram[xx] * sz * hgtRatio / binScale);
        for (size_t yy=0; yy<hgt; ++yy)
            img.xy(xx,yy) = Rgba8{255};
    }
    // Superimpose a similarly scaled Gaussian:
    Affine1D            histToRand = randToHist.inverse();
    for (size_t xx=0; xx<sz; ++xx) {
        double              val = std::exp(-0.5 * sqr(histToRand * (xx + 0.5)));
        size_t              hgt = roundT<int>(val * sz * hgtRatio);
        img.xy(xx,hgt) = Rgba8(255,0,0,255);
    }
    // Display:
    flipVertical_(img);
    if (!isAutomated(args))
        viewImage(img);
}

void                normMoments(CLArgs const &)
{
    for (size_t vv=0; vv<3; ++vv) {
        double                  stdev = pow(2.0,vv);
        PushIndent              pind {"Stdev "+toStr(stdev)};
        size_t                  S = 4096;
        Doubles                 vals = cRandNormals(S,0,stdev);
        double                  sampleMean = cMean(vals),
                                sampleStdev = sqrt(cMag(mapSub(vals,sampleMean)) / (S-1));
        fgout << fgnl << S << " std norms gives sample mean: " << sampleMean << " and sample stdev: " << sampleStdev;
    }
}

}

void                testMath(CLArgs const & args)
{
    Cmds            cmds {
        {testGauss,"gauss","view stdev values for Gaussian"},
        {testLogistic,"logit","logistic, logit and related functions"},
        {testRand,"rand","basic random function"},
        {testZorder,"zorder",""},
    };
    doMenu(args,cmds,true);
}

void                testRandom(CLArgs const & args)
{
    Cmds                cmds {
        {normGraph,"graph","graph generated normals against gaussian"},
        {normMoments,"mom","moments of generated normals"},
    };
    doMenu(args,cmds,true);
}

}
