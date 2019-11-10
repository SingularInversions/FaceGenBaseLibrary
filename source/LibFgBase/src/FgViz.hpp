//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Visualization utilities
//

#ifndef FGVIZ_HPP
#define FGVIZ_HPP

#include "FgStdLibs.hpp"
#include "Fg3dMesh.hpp"
#include "FgMatrixC.hpp"

namespace Fg {

// Visualize function over 2 dimensions as a color-coded (auto-scaled) image:
void
fgVizFuncAsImage(
    std::function<double(Vec2D)>   func,
    Mat22D                         domainBounds,
    uint                                imgSize);

// Function over 2 dimensions as surface:
Mesh
fgFuncToMesh(
    std::function<double(Vec2D)>   func,
    Mat22D                         domainBounds,
    uint                                sz);

}

#endif
