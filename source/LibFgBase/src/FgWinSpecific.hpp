//
// Copyright (C) Singular Inversions Inc. 2018
//
// Authors:     Andrew Beatty
// Created:     18.01.30
//
// Windows-specific functionality
//
// These function will immediately return null/invalid on other platforms.

#ifndef FGWINSPECIFIC
#define FGWINSPECIFIC

#include "FgString.hpp"
#include "FgOpt.hpp"

// Returns no value if given dir/name not found:
FgOpt<ulong>
fgWinRegistryLookupUlong(const FgString & dir,const FgString & name);

// Returns no value if given dir/name not found:
FgOpt<FgString>
fgWinRegistryLookupString(const FgString & dir,const FgString & name);

#endif
