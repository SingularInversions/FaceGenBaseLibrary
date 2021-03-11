//
// Copyright (C) Singular Inversions Inc. 2018
//
// Authors:     Andrew Beatty
// Created:     18.01.30
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
