//
// Copyright (C); Singular Inversions Inc. (facegen.com) 2011
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     August 5, 2016
//

#ifndef FGCMD_HPP
#define FGCMD_HPP

#include "FgCommand.hpp"

FgCmd   fgCmdImgopsInfo();
FgCmd   fgCmdMeshopsInfo();
FgCmd   fgCmdMorphInfo();
FgCmd   fgCmdRenderInfo();
FgCmd   fgCmdTriexportInfo();
void    fgCmdCons(const FgArgs &);
vector<FgCmd> fgCmdViewInfos();

#endif

// */
