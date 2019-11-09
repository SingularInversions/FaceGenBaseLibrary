//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

//
// Not threadsafe. If threaded RNG is needed, make explicit generator for each thread so
// that seeding policy for different threads is explicit:
//

#ifndef FGRANDOM_HPP
#define FGRANDOM_HPP

#include "FgStdLibs.hpp"
#include "FgStdExtensions.hpp"
#include "FgTypes.hpp"

namespace Fg {

void
randSeedRepeatable(uint64 seed=42);

// Returns a uniform random number in the range [0,1]
double
randUniform();

uint32
randUint();

// Returns a uniform random uint in the range [0,size)
uint
randUint(uint size);

uint64
randUint64();

// From the uniformly distributed range [lo,hi)
double
randUniform(double lo,double hi);

// Return random number from standard normal distribution:
double
randNormal();

inline float
randNormalF()
{return float(randNormal()); }

Doubles
randNormals(size_t num,double mean=0.0,double stdev=1.0);

Floats
randNormalFs(size_t num,float mean=0.0f,float stdev=1.0f);

// Alphanumerics only (including capitals):
std::string
randString(uint numChars);

bool
randBool();

// Give random values near 1 or -1 with stdev 0.125 (avoids zero, good for ACS testing):
double
randNearUnit();

Svec<double>
randNearUnits(size_t num);

}

#endif
