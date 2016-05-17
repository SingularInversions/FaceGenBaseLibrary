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

#ifdef _MSC_VER
    int
    wmain(int argc,const wchar_t *argv[])
    {return fgMainConsole(fgCmdFgbl,argc,argv); }
#else
    int
    main(int argc,const char *argv[])
    {return fgMainConsole(fgCmdFgbl,argc,argv); }
#endif
