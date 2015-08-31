//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     August 22, 2005
//
// Compute normals for a mesh

#ifndef FG3DNORMALS_HPP
#define FG3DNORMALS_HPP

#include "FgStdLibs.hpp"

#include "FgMatrix.hpp"
#include "Fg3dMesh.hpp"

struct  Fg3dFacetNormals
{
    vector<FgVect3F>            tri;        // Tri facet normals for a surface
    vector<FgVect3F>            quad;       // Quad facet normals for a surface
};

struct  Fg3dNormals
{
    vector<Fg3dFacetNormals>    facet;       // Facet normals for each surface
    vector<FgVect3F>            vert;        // Vertex normals.
};

void
fgCalcNormals(
    const vector<Fg3dSurface> & surfs,
    const FgVerts &             verts,
    Fg3dNormals &               norms);     // RETURNED

inline
Fg3dNormals
fgNormals(
    const vector<Fg3dSurface> & surfs,
    const FgVerts &             verts)
{
    Fg3dNormals         norms;
    fgCalcNormals(surfs,verts,norms);
    return norms;
}

inline
Fg3dNormals
fgNormals(const Fg3dMesh & mesh)
{return fgNormals(mesh.surfaces,mesh.verts); }

#endif
