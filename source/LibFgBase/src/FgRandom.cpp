//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Jan 27, 2009
//
// Not currently threadsafe.

#include "stdafx.h"

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>
#include "FgRandom.hpp"
#include "FgDiagnostics.hpp"
#include "FgImage.hpp"
#include "FgImgDisplay.hpp"
#include "FgMath.hpp"
#include "FgAffine1.hpp"
#include "FgMain.hpp"

using namespace std;

struct      RNG
{
    boost::random::mt19937      gen;
    RNG() : gen(42u) {}
};

static RNG rng;

uint32
fgRandUint32()
{return rng.gen(); }

uint
fgRandUint(uint size)
{
    uint    lim = numeric_limits<uint>::max(),
            max = lim - (lim%size),
            ret;
    while ((ret = rng.gen()) > max) {}
    return ret % size;
}

uint64
fgRandUint64()
{
    uint64  hi = rng.gen(),
            lo = rng.gen();
    return (lo + (hi << 32));
}

double
fgRand()
{return double(fgRandUint64()) / double(numeric_limits<uint64>::max()); }

void
fgRandSeedRepeatable(uint seed)
{rng.gen = boost::random::mt19937(seed); }

double
fgRandNormal()
{
    // Polar (Box-Muller) method; See Knuth v2, 3rd ed, p122.
    double  x, y, r2;
    do {
        x = -1 + 2 * fgRand();
        y = -1 + 2 * fgRand();
        // see if it is in the unit circle:
        r2 = x * x + y * y;
    }
    while (r2 > 1.0 || r2 == 0);
    // Box-Muller transform:
    return y * sqrt (-2.0 * log (r2) / r2);
}

FgDbls
fgRandNormals(size_t num,double mean,double stdev)
{
    FgDbls      ret(num);
    for (size_t ii=0; ii<num; ++ii)
        ret[ii] = mean + stdev * fgRandNormal();
    return ret;
}

static
char
randChar()
{
    uint    val = fgRandUint(26+26+10);
    if (val < 10)
        return char(48+val);
    val -= 10;
    if (val < 26)
        return char(65+val);
    val -= 26;
    FGASSERT(val < 26);
    return char(97+val);
}

string
fgRandString(uint numChars)
{
    string  ret;
    for (uint ii=0; ii<numChars; ++ii)
        ret = ret + randChar();
    return ret;
}

void
fgRandomTest(const FgArgs &)
{
    fgout << fgnl << "sizeof(RNG) = " << sizeof(RNG);
    // Create a histogram of normal samples:
    fgRandSeedRepeatable();
    size_t          numStdevs = 6,
                    binsPerStdev = 50,
                    numSamples = 1000000,
                    sz = numStdevs * 2 * binsPerStdev;
    vector<size_t>  histogram(sz,0);
    FgAffine1D      randToHist(FgVectD2(-double(numStdevs),numStdevs),FgVectD2(0,sz));
    for (size_t ii=0; ii<numSamples; ++ii) {
        int         rnd = fgRound(randToHist * fgRandNormal());
        if ((rnd >= 0) && (rnd < int(sz)))
            ++histogram[rnd]; }

    // Make a bar graph of it:
    // numSamples = binScale * stdNormIntegral * binsPerStdev
    double          binScale = double(numSamples) / (fgSqrt_2pi() * binsPerStdev),
                    hgtRatio = 0.9;
    FgImgRgbaUb     img(sz,sz,FgRgbaUB(0));
    for (size_t xx=0; xx<sz; ++xx) {
        size_t      hgt = fgRound(histogram[xx] * sz * hgtRatio / binScale);
        for (size_t yy=0; yy<hgt; ++yy)
            img.xy(xx,yy) = FgRgbaUB(255); }

    // Superimpose a similarly scaled Gaussian:
    FgAffine1D      histToRand = randToHist.inverse();
    for (size_t xx=0; xx<sz; ++xx) {
        double      val = std::exp(-0.5 * fgSqr(histToRand * (xx + 0.5)));
        size_t      hgt = fgRound(val * sz * hgtRatio);
        img.xy(xx,hgt) = FgRgbaUB(255,0,0,255); }

    // Display:
    fgImgFlipVertical(img);
    fgImgDisplay(img);
}

// */
