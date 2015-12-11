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
fgMath::solveCubicReal(
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
                        ss = fgCbrt(rr + sqdd),
                        tt = fgCbrt(rr - sqdd);
        retval.push_back(ss+tt-(c2/3.0));
        return retval;
    }
    else if (dd == 0.0) {               // All real roots, at least 2 equal
        double          ss = fgCbrt(rr);
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

double
fgMath::normal(double val,double mean,double stdev)
{
    FGASSERT(stdev > 0.0);
    return (fgExp(-0.5*fgSqr(val-mean)/fgSqr(stdev))/(stdev*fgSqrt_2pi()));
}

double
fgMath::lnNormal(double val,double mean,double stdev)
{
    FGASSERT(stdev > 0.0);
    return (-0.5 * fgLn_2pi() - std::log(stdev) - 0.5 * fgSqr(val-mean) / fgSqr(stdev));
}

double
fgMath::lnNormalIid(
    double  dimension,
    double  ssd,
    double  stdev)
{
    FGASSERT((dimension > 0.0) && (ssd >= 0.0) && (stdev > 0.0));
    return (-0.5 * dimension * fgLn_2pi() 
            - dimension * log(stdev)
            - 0.5 * ssd / fgSqr(stdev));
}

double
fgMath::lnNormalIidIgMapConstDim(
    double  dimension,      // Number of IID measures
    double  ssd)            // Sum of square differences between measures and means
{
    FGASSERT((dimension > 0.0) && (ssd >= 0.0));
    return (-0.5 * dimension * log(ssd));
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
    FgIndent    dummy("Testing rand");
    testFgRand();
}
