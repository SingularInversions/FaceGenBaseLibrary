//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGHASH_HPP
#define FGHASH_HPP

#include "FgStdVector.hpp"

namespace Fg {

// Concatenates the arguments into a message then computes a deterministic 64 bit hash of the message which:
// * Always gives the same result on any platform
// * Is not cryptographically secure
uint64          treeHash(Uint64s const & hashes);

}

#endif
