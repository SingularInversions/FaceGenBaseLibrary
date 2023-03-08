//
// Copyright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgMath.hpp"
#include "FgRandom.hpp"
#include "FgSyntax.hpp"

using namespace std;

namespace Fg {

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
        retval.push_back(ss * cos(theta3 + 2.0 * pi() / 3.0) - c23);
        retval.push_back(ss * cos(theta3 - 2.0 * pi() / 3.0) - c23);
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
    size_t          ksize = round<uint>(stdev * 6);
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

static void         testZorder()
{
    size_t      v0 = 0x01,
                v1 = 0x02,
                v2 = 0x04,
                r0 = zorder(v0,v1,v2),
                r1 = v0 | (v1 << 3) | (v2 << 6);
    FGASSERT(r0 == r1);
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

// Test by generating 1M numbers and taking the average (should be 1/2) and RMS (should be 1/3).
static void         testFgRand()
{
    randSeedRepeatable();
    const uint      numSamples = 1000000;
    double const    num = double(numSamples);
    vector<double>  vals(numSamples);
    double          mean = 0.0;
    for (uint ii=0; ii<numSamples; ii++)
    {
        vals[ii] = randUniform();
        mean += vals[ii];
    }
    mean /= num;
    double          rms = 0.0;
    for (uint ii=0; ii<numSamples; ii++)
        rms += sqr(vals[ii]);
    rms = (rms / num);

    fgout << fgnl << "Mean: " << mean << fgnl << "RMS: " << rms;
    FGASSERT(std::abs(mean * 2.0 - 1.0) < 0.01);    // Should be good to 1 in sqrt(1M)
    FGASSERT(std::abs(rms * 3.0 - 1.0) < 0.01);
}

void                testMath(CLArgs const &)
{
    PushIndent       op("Testing rand");
    testFgRand();
    testZorder();
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
    double   dbl;
    struct {int32_t  lo,hi;} asInt;
}  FgExpFast;

double              expFast(double x)
{
    x *=  1.4426950408889634074;        // Convert to base 2 exponent
    double      ipart = floor(x+0.5),
                fpart = x - ipart;
    FgExpFast   epart;
    epart.asInt.lo = 0;
    epart.asInt.hi = (((int) ipart) + 1023) << 20;
    double      y = fpart*fpart,
                px = 2.30933477057345225087e-2;
    px = px*y + 2.02020656693165307700e1;
    double      qx = y + 2.33184211722314911771e2;
    px = px*y + 1.51390680115615096133e3;
    qx = qx*y + 4.36821166879210612817e3;
    px = px * fpart;
    y = 1.0 + 2.0*(px/(qx-px));
    return epart.dbl*y;
}

}
