//
// Copyright (C) Singular Inversions Inc. 2018
//
// Windows-specific functionality
//
// These function will immediately return null/invalid on other platforms.

#ifndef FGWINSPECIFIC
#define FGWINSPECIFIC

#include "FgString.hpp"
#include "FgOpt.hpp"

namespace Fg {

// Returns no value if given dir/name not found:
Opt<ulong>
winRegistryLookupUlong(Ustring const & dir,Ustring const & name);

// Returns no value if given dir/name not found:
Opt<Ustring>
fgWinRegistryLookupString(Ustring const & dir,Ustring const & name);

}

#endif
