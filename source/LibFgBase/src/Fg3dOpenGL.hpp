//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Sept 25, 2009
//
//	OICS := OpenGL Image CS: Origin at centre of image, lower left corner
//			of at (-1,-1), upper right corner at (1,1).
//	OXCS := OpenGL Texture CS: Origin at lower left corner of image, (1,1) at
//			upper right corner.

#ifndef FG3DOPENGL_HPP
#define FG3DOPENGL_HPP

#include "FgStdLibs.hpp"
#include "FgLighting.hpp"
#include "Fg3dRenderOptions.hpp"
#include "Fg3dNormals.hpp"

// Not yet used:
std::string
fgOglGetInfo();

void
fgOglSetup();

uint
fgOglTextureAdd(
    const FgImgRgbaUb & img);

void
fgOglTextureUpdate(
    uint                name,
    const FgImgRgbaUb & img);

struct  FgOglRendSurf
{
    bool    visible;            // Currently selected for viewing ?
    bool    xray;               // Currently selected for see-through ?
    //bool    opacity;            // Does the texture image have variable opacity ?

    FgOglRendSurf() : visible(true), xray(false) {}
};

struct  FgOglRendModel
{
    const Fg3dMesh *            mesh;
    const FgVerts *             verts;
    const Fg3dNormals *         norms;
    const vector<int> *         texNames;       // < 0 means no texture image available
    vector<FgOglRendSurf>       rendSurfs;
};

void
fgOglSetLighting(const FgLighting & lt);

void
fgOglRender(
    vector<FgOglRendModel>      rendModels,
    FgMat44F                 oglMvm, // MVM in column-major layout.
    FgVect6D                    frustum,
    const Fg3dRenderOptions &   rend);

FgMat44F
fgOglTransform();

void
fgOglTexRelease(uint texName);

#endif
