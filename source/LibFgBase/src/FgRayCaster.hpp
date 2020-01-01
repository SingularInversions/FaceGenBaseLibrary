//
// Copyright (c) 2018 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
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

namespace Fg {

struct  TriInd
{
    uint32      triIdx;
    uint16      surfIdx = 0;
    uint16      meshIdx = 0;

    TriInd() {}
    TriInd(size_t triIdx_,size_t surfIdx_,size_t meshIdx_);
};
typedef Svec<TriInd>    TriInds;

// Ray-casting requires caching the projected coordinates as well as their mesh and surface indices:
struct  RayCaster
{
    Trisss                  trisss;         // By mesh, by surface
    Materialss              materialss;     // By mesh, by surface
    Vec3Fss                 vertss;         // By mesh, in OECS
    Svec<Vec2Fs const *>    uvsPtrs;        // By mesh, in OTCS
    Normalss                normss;         // By mesh, in OECS
    AffineEw2D              itcsToIucs;
    Vec3Fss                 iucsVertss;     // By mesh, X,Y in IUCS, Z component is inverse CCS depth
    GridIndex<TriInd>       grid;           // Index from IUCS to bin of TriInds
    Lighting                lighting;
    RgbaF                   background;     // Must be alpha-weighted

    RayCaster(
        Meshes const &      meshes,
        Affine3D            modelview,      // to OECS
        AffineEw2D          itcsToIucs,
        Lighting const &    lighting,       // In OECS
        RgbaF               background);    // Must be alpha-weighted

    RgbaF
    cast(Vec2F posIucs) const;

    // Return value depth component is inverse depth if visible and >0, negative otherwise:
    Vec3F
    oecsToIucs(Vec3F posOecs) const;

    struct  Intersect
    {
        TriInd          triInd;
        Vec3D           barycentric;

        Intersect() {}
        Intersect(TriInd ti,Vec3D bc) : triInd(ti), barycentric(bc) {}
    };

    // Return closest tri intersects for given ray:
    FgBestN<float,Intersect,4>
    closestIntersects(Vec2F posIucs) const;
};

}

#endif

// */
