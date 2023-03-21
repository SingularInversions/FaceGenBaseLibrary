//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Not threadsafe. If threaded RNG is needed, make explicit generator for each thread so
// that seeding policy for different threads is explicit:
//
// Currently uses std::mt19937_64 which seeds itself differently at each execution but
// do not depend on this behaviour !
//

#ifndef FGRANDOM_HPP
#define FGRANDOM_HPP

#include "FgSerial.hpp"

namespace Fg {

void                randSeedRepeatable(uint64 seed=42);     // same seed gives same PRNG sequence
void                randSeedTime(); // Seed with current time in milliseconds to ensure different results each time
double              randUniform();                          // uniform random number in the range [0,1]
uint32              randUint();
uint                randUint(uint size);                    // uniform random uint in the range [0,size)
uint64              randUint64();
double              randUniform(double lo,double hi);       // From the uniformly distributed range [lo,hi)
double              randNormal();                           // random number from standard normal distribution
Doubles             cRandNormals(size_t num,double mean=0.0,double stdev=1.0);

template<size_t S>
Arr<double,S>       randNormalArr(double mean=0.0,double stdev=1.0)
{
    Arr<double,S>       ret;
    for (size_t ii=0; ii<S; ++ii)
        ret[ii] = randNormal() * stdev + mean;
    return ret;
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

}

#endif
