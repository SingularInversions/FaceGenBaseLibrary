//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: April 21, 2011
//

#include "stdafx.h"

#include "FgCl.hpp"
#include "FgException.hpp"

using namespace std;

namespace Fg {

namespace fgCl {

void
unzip(const string & fname)
{
    run("\"C:\\Program Files\\7-Zip\\7z.exe\" x "+fname+" >> log.txt");
}

void
zip(const string & dir,bool oldFormat)
{
    string      ext = (oldFormat ? ".zip " : ".7z ");
    run("\"C:\\Program Files\\7-Zip\\7z.exe\" a "+dir+ext+dir+" >> log.txt");
}

}   // namespace

}
