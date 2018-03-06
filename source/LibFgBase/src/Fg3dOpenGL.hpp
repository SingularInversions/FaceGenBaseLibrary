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
//	OTCS := OpenGL Texture CS: Origin at lower left corner of image, (1,1) at
//			upper right corner.

#ifndef FG3DOPENGL_HPP
#define FG3DOPENGL_HPP

#include "FgStdLibs.hpp"
#include "FgLighting.hpp"
#include "Fg3dRenderOptions.hpp"
#include "Fg3dNormals.hpp"
#include "FgDefaultVal.hpp"

// Not yet used:
std::string
fgOglGetInfo();

void
fgOglSetup();

// Returns the OGL texture 'name':
uint
fgOglTextureAdd(const FgImgRgbaUb & img);

void
fgOglTextureUpdate(
    uint                name,       // Existing OGL texture 'name'
    const FgImgRgbaUb & img);       // Send power of 2 images whenever possible or slow resizing will have to be done.

struct  FgOglSurf
{
    FgBoolT         visible;        // Set to false if this surface is not being rendered.
    FgOpt<uint>     name;           // OpenGL texture "name". Invalid if no texture image.
    FgBoolF         transparency;   // Set to true if the image contains varying alpha values
};

struct  FgOglRendModel
{
    const Fg3dMesh *            mesh;
    const FgVerts *             verts;
    const Fg3dNormals *         norms;
    const vector<FgOglSurf> *   oglImages;
};

void
fgOglSetLighting(const FgLighting & lt);

struct  FgBgImage
{
    FgOpt<uint>                 texName;
    FgVect2UI                   origDims;
    FgVect2F                    offset;     // Relative to image max dimension
    float                       scale;      // Relative to image max dimension
};

void
fgOglRender(
    vector<FgOglRendModel>      rendModels,
    FgMat44F                    oglMvm, // MVM in column-major layout.
    FgVect6D                    frustum,
    const Fg3dRenderOptions &   rend,
    FgBgImage                   bgImage);

FgMat44F
fgOglTransform();

FgImgRgbaUb
fgOglGetRender();

void
fgOglTexRelease(uint texName);

#endif
