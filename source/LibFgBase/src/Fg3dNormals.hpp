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
    FgVect3Fs               tri;        // Tri facet normals for a surface
    FgVect3Fs               quad;       // Quad facet normals for a surface

    FgVect3F
    triEquiv(size_t idx) const
    {
        if (idx < tri.size())
            return tri[idx];
        idx -= tri.size();
        return quad[idx/2];
    }
};
typedef vector<Fg3dFacetNormals>    Fg3dFacetNormalss;

struct  Fg3dNormals
{
    Fg3dFacetNormalss       facet;       // Facet normals for each surface
    FgVect3Fs               vert;        // Vertex normals.
};
typedef vector<Fg3dNormals>         Fg3dNormalss;

void
fgNormals_(
    const Fg3dSurfaces &    surfs,
    const FgVerts &         verts,
    Fg3dNormals &           norms);     // RETURNED

inline
Fg3dNormals
fgNormals(const Fg3dSurfaces & surfs,const FgVerts & verts)
{
    Fg3dNormals         norms;
    fgNormals_(surfs,verts,norms);
    return norms;
}

inline
Fg3dNormals
fgNormals(const Fg3dMesh & mesh)
{return fgNormals(mesh.surfaces,mesh.verts); }

#endif
