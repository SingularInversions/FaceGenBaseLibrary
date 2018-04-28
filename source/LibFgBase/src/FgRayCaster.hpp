//
// Copyright (c) 2018 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     18.04.24
//
// Simple ray-casting fragment renderer.
//

#ifndef FGRAYCASTER_HPP
#define FGRAYCASTER_HPP

#include "Fg3dNormals.hpp"
#include "FgLighting.hpp"
#include "FgGridIndex.hpp"
#include "FgBestN.hpp"
#include "FgAffineCwC.hpp"

struct  FgTriInd
{
    uint32      triIdx;
    uint16      surfIdx = 0;
    uint16      meshIdx = 0;

    FgTriInd() {}
    FgTriInd(size_t triIdx_,size_t surfIdx_,size_t meshIdx_);
};
typedef vector<FgTriInd>    FgTriInds;

// Ray-casting requires caching the projected coordinates as well as their mesh and surface indices:
struct  FgRayCaster
{
    FgTrisss                trisss;         // By mesh, by surface
    FgMaterialss            materialss;     // By mesh, by surface
    FgVertss                vertss;         // By mesh, in OECS
    vector<const FgVect2Fs *>   uvsPtrs;    // By mesh, in OTCS
    Fg3dNormalss            normss;         // By mesh, in OECS
    FgAffineCw2D            itcsToIucs;
    FgVertss                iucsVertss;     // By mesh, X,Y in IUCS, Z component is inverse CCS depth
    FgGridIndex<FgTriInd>   grid;           // Index from IUCS to bin of FgTriInds
    FgLighting              lighting;
    FgRgbaF                 background;     // Must be alpha-weighted

    FgRayCaster(
        const Fg3dMeshes &      meshes,
        FgAffine3D              modelview,      // to OECS
        FgAffineCw2D            itcsToIucs,
        const FgLighting &      lighting,       // In OECS
        FgRgbaF                 background);    // Must be alpha-weighted

    FgRgbaF
    cast(FgVect2F posIucs) const;

    // Return value depth component is inverse depth if visible and >0, negative otherwise:
    FgVect3F
    oecsToIucs(FgVect3F posOecs) const;

    struct  Intersect
    {
        FgTriInd        triInd;
        FgVect3D        barycentric;

        Intersect() {}
        Intersect(FgTriInd ti,FgVect3D bc) : triInd(ti), barycentric(bc) {}
    };

    // Return closest tri intersects for given ray:
    FgBestN<float,Intersect,4>
    closestIntersects(FgVect2F posIucs) const;
};

#endif

// */
