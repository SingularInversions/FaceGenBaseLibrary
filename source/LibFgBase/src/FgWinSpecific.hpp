//
// Copyright (C) Singular Inversions Inc. 2018
//
// Windows-specific functionality
//
// These function will immediately return null/invalid on other platforms.

#ifndef FGWINSPECIFIC
#define FGWINSPECIFIC

#include "FgSerial.hpp"

namespace Fg {

// Returns no value if given dir/name not found:
Opt<ulong>
winRegistryLookupUlong(String8 const & dir,String8 const & name);

// Returns no value if given dir/name not found:
Opt<String8>
fgWinRegistryLookupString(String8 const & dir,String8 const & name);

}

#endif
