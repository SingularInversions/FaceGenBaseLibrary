//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Jan 9, 2013
//
// Visualization utilities
//

#ifndef FGVIZ_HPP
#define FGVIZ_HPP

#include "FgStdLibs.hpp"
#include "Fg3dMesh.hpp"

#include "FgMatrixC.hpp"

// Visualize function over 2 dimensions as a color-coded (auto-scaled) image:
void
fgVizFuncAsImage(
    std::function<double(FgVect2D)>   func,
    FgMat22D                         domainBounds,
    uint                                imgSize);

// Function over 2 dimensions as surface:
Fg3dMesh
fgFuncToMesh(
    std::function<double(FgVect2D)>   func,
    FgMat22D                         domainBounds,
    uint                                sz);

#endif
