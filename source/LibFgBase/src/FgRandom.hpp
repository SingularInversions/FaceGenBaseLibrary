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
uint64              randUniformUint(uint64 eub);            // uniform number in [0,eub)
uint64              randUint64();
double              randUniform(double lo,double hi);       // From the uniformly distributed range [lo,hi)
double              randNormal();                           // random number from standard normal distribution
Doubles             cRandNormals(size_t num,double mean=0.0,double stdev=1.0);

template<class T,size_t S,FG_ENABLE_IF(T,is_floating_point)>
Arr<T,S>            randNormalArr(T mean=0.0,T stdev=1.0)
{
    auto                fn = [mean,stdev](size_t){return randNormal()*stdev + mean; };
    return genArr<T,S>(fn);
}
template<class T,size_t S,FG_ENABLE_IF(T,is_floating_point)>
Arr<T,S>            randUniformArr(T lo,T hi)
{
    auto                fn = [lo,hi](size_t){return randUniform(lo,hi); };
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
