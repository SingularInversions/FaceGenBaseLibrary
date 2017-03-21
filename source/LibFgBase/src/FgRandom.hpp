//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Jan 27, 2009
//

#ifndef FGRANDOM_HPP
#define FGRANDOM_HPP

#include "FgStdLibs.hpp"
#include "FgTypes.hpp"

void
fgRandSeedRepeatable(uint seed=42);

// Returns a uniform random number in the range [0,1]
double
fgRand();

uint32
fgRandUint32();

// Returns a uniform random uint in the range [0,size)
uint
fgRandUint(uint size);

uint64
fgRandUint64();

// From the uniformly distributed range [lo,hi)
inline double
fgRandUniform(double lo,double hi) 
{return (fgRand() * (hi-lo) + lo); }

double
fgRandNormal(double mean=0.0,double stdev=1.0);

std::vector<double>
fgRandNormals(size_t num,double mean=0.0,double stdev=1.0);

// Exponential distribution:
inline double
fgRandExp()
{return -std::log(1.0-fgRand()); }

std::string
fgRandString(uint numChars);

#endif
