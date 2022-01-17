//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgWinSpecific.hpp"
#include "FgSyntax.hpp"

using namespace std;

namespace Fg {

Opt<ulong>
winRegistryLookupUlong(String8 const &,String8 const &)
{return Opt<ulong>(); }

Opt<String8>
fgWinRegistryLookupString(String8 const &,String8 const &)
{return Opt<String8>(); }

}
