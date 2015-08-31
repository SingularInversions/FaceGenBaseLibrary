//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     April 7, 2010
//
// Anti-aliased ray-casting software renderer
//

#ifndef FG_SOFTRENDER_HPP
#define FG_SOFTRENDER_HPP

#include "Fg3dMesh.hpp"
#include "Fg3dNormals.hpp"
#include "FgLighting.hpp"
#include "FgImage.hpp"

FgImgRgbaUb
fgSoftRender(
    FgVect2UI                   pixelSize,
    const vector<Fg3dMesh> &    meshes,
    const FgLighting &          light,                  // in OECS (not transformed)
    FgAffine3D                  modelview,              // Transform verts into OECS
    // This fully specifies the projection since depth is not transformed and there are
    // no clip planes (except for the implicit clipping at Z=0):
    FgAffineCw2D                itcsToIucs,
    FgRgbaF                     backgroundColor,        // PRE-WEIGHTED values in range [0,255]
    uint                        antiAliasBitDepth=3);   // in [1,8], higher is slower

#endif

// */
