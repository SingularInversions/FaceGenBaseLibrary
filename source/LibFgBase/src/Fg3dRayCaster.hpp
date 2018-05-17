//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     April 29, 2010
//
// Keeps a separate grid index for each surface, despite the additional overhead:
// * Naturally avoids indexing areas of the image with no objects maximizing cache efficiency
// * No additional information about which surface needs to be stored with each triangle record
// * Keeps pointers into client constructor objects so never keep beyond scope lifetime.
//   We prefer pointers to references here since they need to be stored in std::vector.

#ifndef FG3DRAYCASTER_HPP
#define FG3DRAYCASTER_HPP

#include "Fg3dNormals.hpp"
#include "FgLighting.hpp"
#include "FgGridTriangles.hpp"
#include "FgBestN.hpp"
#include "FgAffineCwC.hpp"

typedef boost::function<FgRgbaF(FgVect3F,FgVect2F,FgMaterial,const FgImgRgbaUb *)>   FgFuncShader;

struct  Fg3dRayCastMesh
{
    const FgVerts *             vertsPtr;   // OECS
    const Fg3dNormals *         normsPtr;   // OECS
    const FgVect2Fs *           uvsPtr;
    Fg3dSurfaces                surfs;      // Converted to tris
    FgMaterial                  material;
    FgGridTriangles             grid;       // IUCS
    FgFlts                      invDepths;  // Inverse depth values. 1-1 with 'verts'
    FgVect2Fs                   vertsIucs;  // Vertex projections into image plane

    Fg3dRayCastMesh(
        const Fg3dMesh &        mesh,       // Ignore base vertex positions here
        const FgVerts &         verts,      // Current OECS vertex positions.
        const Fg3dNormals &     normss,     // Current OECS normals.
        FgAffineCw2F            itcsToIucs);

    FgBestN<float,FgTriPoint,8>
    cast(FgVect2F posIucs) const;

    FgRgbaF
    shade(const FgTriPoint & intersect,const FgLighting & lighting) const;
};

struct  Fg3dRayCaster
{
    const FgLighting *          lightingPtr;
    FgRgbaF                     m_background;
    vector<Fg3dRayCastMesh>     rayMesh;

    Fg3dRayCaster(
        const Fg3dMeshes &      meshes,
        const FgVertss &        vertss,         // Current OECS vertex positions. Must be 1-1 with above.
        const Fg3dNormalss &    normss,         // Current OECS normals. Must be 1-1 with above.
        const FgLighting &      lighting,
        FgAffineCw2F            itcsToIucs,
        FgRgbaF                 background);

    FgRgbaF
    cast(FgVect2F posIucs) const;

    struct Best
    {
        size_t                  surfIdx;
        FgTriPoint              intersect;

        Best() {}

        Best(size_t s,FgTriPoint i)
        : surfIdx(s), intersect(i)
        {}
    };
};

#endif

// */
