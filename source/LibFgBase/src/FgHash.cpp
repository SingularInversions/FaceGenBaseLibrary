//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgHash.hpp"
#include "MurmurHash2.h"
#include "FgSyntax.hpp"
#include "FgHex.hpp"

using namespace std;

namespace Fg {

uint64
treeHash(Uint64s const & hashes)
{
    FGASSERT(hashes.size() < scast<size_t>(lims<int>::max() / 8));
    int                     len = scast<int>(hashes.size() * 8);
    string                  msg; msg.reserve(len);
    for (uint64 hash : hashes)
        msg.append(reinterpret_cast<char *>(&hash),8);
    // std::hash is not deterministic (across standard libraries or CPU architectures) so cannot be used here.
    // Don't use MurmurHash3 as it gives different results on x86 and x86_64 for its only hash larger than
    // 32 (which happens to be 128). The see was chosen at random.
    return MurmurHash64A(reinterpret_cast<void const *>(msg.data()),len,0x0B779664AC6C80E1ULL);
}

void
testHash(CLArgs const &)
{
    uint64                  in0 = 0x00C89C66E406A689ULL,        // Chosen at random
                            in1 = 0xE46B92AF98E1D9CCULL,        // "
                            out0 = treeHash({in0,in1}),
                            out1 = treeHash({in1,in0});
    fgout
        << fgnl << "Hash order 0: " << toHexString(out0)
        << fgnl << "Hash order 1: " << toHexString(out1);
    // Test determinism on all platforms:
    FGASSERT(out0 == 0x960D9C0EDFDE4928ULL);
    FGASSERT(out1 == 0x888931C0760EFC53ULL);
}

}
