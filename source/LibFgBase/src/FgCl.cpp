//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: April 21, 2011
//

#include "stdafx.h"

#include "FgCl.hpp"
#include "FgException.hpp"
#include "FgOut.hpp"

using namespace std;

namespace fgCl
{

bool preview = false;

bool
run(
    const string & cmd,
    bool throwIfError)
{
    fgout << fgnl << cmd << endl;   // DOS output lines will always start in zero'th column anyway
    int     retval = 0;
    if (!preview)
        retval = system(cmd.c_str());
    if (retval != 0)
    {
        if (throwIfError)
            fgThrow("Error while executing command",cmd);
        else
            fgout << fgnl << "Error while executing command: " << cmd;
        return false;
    }
    return true;
}

}   // namespace
