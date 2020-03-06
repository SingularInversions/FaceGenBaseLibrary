//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgStdVector.hpp"
#include "FgTestUtils.hpp"
#include "FgMain.hpp"

namespace Fg {

void
fgStdVectorTest(CLArgs const &)
{
    Ints          v = { 1, 2, 3 };
    Intss         subs = cSubsets(v,0,4);
    fgout << fgnl << "Subsets of 1..3 : " << subs;
    FGASSERT(subs.size() == 8);
    subs = cSubsets(v,2,2);
    fgout << fgnl << "Of which size 2 : " << subs;
    Intss         chk = {{1,2},{1,3},{2,3}};
    FGASSERT(subs == chk);
}

}
