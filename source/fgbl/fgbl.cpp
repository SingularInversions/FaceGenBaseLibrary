//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: July 15, 2015
//

#include "FgMain.hpp"

void    fgCmdFgbl(const FgArgs &);

int
main(int argc,const char *argv[])
{
    return fgMain(fgCmdFgbl,argc,argv);
}
