//
// Copyright (C); Singular Inversions Inc. (facegen.com) 2011
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGCMD_HPP
#define FGCMD_HPP

#include "FgCommand.hpp"

namespace Fg {

Cmd   fgCmdImgopsInfo();
Cmd   fgCmdMeshopsInfo();
Cmd   fgCmdMorphInfo();
Cmd   fgCmdRenderInfo();
Cmd   fgCmdTriexportInfo();
void    fgCmdCons(CLArgs const &);
Svec<Cmd> fgCmdViewInfos();

}

#endif

// */
