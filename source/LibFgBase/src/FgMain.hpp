//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
//

#ifndef FGMAIN_HPP
#define FGMAIN_HPP

#include "FgStdLibs.hpp"
#include "FgBoostLibs.hpp"
#include "FgStdVector.hpp"
#include "FgStdString.hpp"

typedef vector<string> FgArgs;

FgArgs
fgArgs(int argc,const char * argv[]);

typedef boost::function<void(const FgArgs &)> FgCmdFunc;

// Catches exceptions, outputs error details, returns appropriate value:
int
fgMainConsole(FgCmdFunc func,int argc,const char * argv[]);

#endif
