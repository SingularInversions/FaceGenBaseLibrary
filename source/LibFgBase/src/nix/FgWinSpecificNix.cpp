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
winRegistryLookupUlong(Ustring const &,Ustring const &)
{return Opt<ulong>(); }

Opt<Ustring>
fgWinRegistryLookupString(Ustring const &,Ustring const &)
{return Opt<Ustring>(); }

}
