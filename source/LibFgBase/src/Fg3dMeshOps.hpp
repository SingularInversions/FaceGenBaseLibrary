//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     July 28, 2006
//

#ifndef FG3DMESHOPS_HPP
#define FG3DMESHOPS_HPP

#include "FgStdLibs.hpp"
#include "Fg3dMesh.hpp"
#include "FgMatrixC.hpp"
#include "FgBoostLibs.hpp"

struct FgVertexDelta
{
    uint     vertex_idx;        // index into mesh vertices
    FgVect3F delta;             // vertex offset from original vertex
};

// Creates 2.5D surface from depth image:
// * Vertex created for each pixel (RCS X to X, Y to Y and pixel value to Z)
// * Vertices are re-scaled to fit bounding box [0,1]^3
// * Quads created by defined pixel grid.
// * UVs created to fit bounds of image (so for perfect correspondence the depth image pixel
//   centres should fill the domain bounds wheras the texture image bounds should map the domain
//   bounds).
Fg3dMesh
fgMeshFromImage(const FgImgD & img);

// Creates a sphere centred at the origin by subdividing a tetrahedron and renormalizing the 
// vertex distances from the origin 'subdivision' times:
Fg3dMesh
fgCreateSphere(float radius,uint subdivisions);

Fg3dMesh
fgRemoveDuplicateFacets(const Fg3dMesh &);

Fg3dMesh
fgRemoveUnusedVerts(const Fg3dMesh &);

Fg3dMesh
fgCube(bool open=false);

Fg3dMesh
fgPyramid(bool open=false);

Fg3dMesh
fgTetrahedron(bool open=false);

Fg3dMesh
fgOctahedron();

Fg3dMesh
fgNTent(uint nn);

Fg3dMesh
fgMergeSameNameSurfaces(const Fg3dMesh &);

Fg3dMesh
fgMergeIdenticalUvInds(const Fg3dMesh &);

Fg3dMesh
fgSplitSurfsByUvs(const Fg3dMesh &);

// Merge surfaces in meshes with identically sized vertex lists,
// keeping the vertex list of the first mesh:
Fg3dMesh
fgMergeMeshSurfaces(
    const Fg3dMesh &    m0,
    const Fg3dMesh &    m1);

// Only merges in the surfaces from the second mesh:
Fg3dMesh
fgMergeMeshes(
    const Fg3dMesh &    m0,
    const Fg3dMesh &    m1);

// Doesn't preserve uvs, surface points or marked verts:
Fg3dMesh
fg3dMaskFromUvs(const Fg3dMesh & mesh,const FgImage<FgBool> & mask);

FgImgRgbaUb
fgUvImage(const Fg3dMesh &,const FgImgRgbaUb & img=FgImgRgbaUb());

#endif
