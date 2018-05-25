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

FgOpt<ulong>
fgWinRegistryLookupUlong(const FgString &,const FgString &)
{return FgOpt<ulong>(); }

FgOpt<FgString>
fgWinRegistryLookupString(const FgString &,const FgString &)
{return FgOpt<FgString>(); }
