//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Aug 24, 2005
//

#include "stdafx.h"

#include "FgMath.hpp"
#include "FgRandom.hpp"
#include "FgSyntax.hpp"

using namespace std;

// Without a hardware nlz (number of leading zeros) instruction, this function has
// to be iterative (C / C++ doesn't have any keyword for this operator):
uint
fgNumLeadingZeros(uint32 x)
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


uint8
fgNumNonzeroBits8(uint8 xx)
{
    return
        (xx & 1U) + (xx & 2U)/2 + (xx & 4U)/4 + (xx & 8U)/8 +
        (xx & 16U)/16 + (xx & 32U)/32 + (xx & 64U)/64 + (xx & 128U)/128;
}
uint16
fgNumNonzeroBits16(uint16 xx)
{
    return fgNumNonzeroBits8(xx & 255) + fgNumNonzeroBits8(xx >> 8);
}
uint
fgNumNonzeroBits32(uint32 xx)
{
    return
        fgNumNonzeroBits8(xx & 255) + fgNumNonzeroBits8((xx >> 8) & 255) +
        fgNumNonzeroBits8((xx >> 16) & 255) + fgNumNonzeroBits8((xx >> 24));
}

uint
fgLog2Ceil(uint32 xx)
{
    uint    logFloor = fgLog2Floor(xx);
    if (xx == (1u << logFloor))
        return (fgLog2Floor(xx));
    else
        return (fgLog2Floor(xx) + 1u);
}

// RETURNS: Between 1 and 3 real roots of the equation. Duplicate roots are returned in duplicate.
// The cubic term co-efficient is assumed to be 1.0.
// From Spiegel '99, Mathematical Handbook of Formulas and Tables.
vector<double>
fgSolveCubicReal(
    double      c0,         // constant term
    double      c1,         // first order coefficient
    double      c2)         // second order coefficient
{
    vector<double>  retval;

    double      qq = (3.0 * c1 - fgSqr(c2)) / 9.0,
                rr = (9.0 * c1 * c2 - 27.0 * c0 - 2.0 * fgCube(c2)) / 54.0,
                dd = fgCube(qq) + fgSqr(rr);

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
        retval.push_back(ss * cos(theta3 + 2.0 * fgPi() / 3.0) - c23);
        retval.push_back(ss * cos(theta3 - 2.0 * fgPi() / 3.0) - c23);
    }

    return retval;
}

// Test by generating 1M numbers and taking the average (should be 1/2) and RMS (should be 1/3).
static
void
testFgRand()
{
    fgRandSeedRepeatable();
    const uint      numSamples = 1000000;
    const double    num = double(numSamples);
    vector<double>  vals(numSamples);
    double          mean = 0.0;
    for (uint ii=0; ii<numSamples; ii++)
    {
        vals[ii] = fgRand();
        mean += vals[ii];
    }
    mean /= num;
    double          rms = 0.0;
    for (uint ii=0; ii<numSamples; ii++)
        rms += fgSqr(vals[ii]);
    rms = (rms / num);

    fgout << fgnl << "Mean: " << mean << fgnl << "RMS: " << rms;
    FGASSERT(std::abs(mean * 2.0 - 1.0) < 0.001);    // Should be good to 1 in sqrt(1M)
    FGASSERT(std::abs(rms * 3.0 - 1.0) < 0.001);
}

void
fgMathTest(const FgArgs &)
{
    FgOutPush       op("Testing rand");
    testFgRand();
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

double fgExpFast(double x)
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
