//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Oct. 5, 2009
//

#ifndef FG3DRENDEROPTIONS_HPP
#define FG3DRENDEROPTIONS_HPP

#include "FgStdLibs.hpp"

#include "FgMatrix.hpp"
#include "Fg3dMesh.hpp"
#include "FgDepGraph.hpp"

struct  Fg3dRenderOptions
{
    bool        facets;
    bool        useTexture;
    bool        shiny;
    bool        wireframe;
    bool        flatShaded;
    bool        surfPoints;
    bool        markedVerts;
    bool        allVerts;
    bool        twoSided;
    FgVect3F    backgroundColor;
    // Set by render impl not by client:
    FgBoolF     colorBySurface;
};

#endif
