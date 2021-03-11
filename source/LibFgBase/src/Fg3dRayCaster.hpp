//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
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

namespace Fg {

struct  Fg3dRayCastMesh
{
    Vec3Fs const *          vertsPtr;   // OECS
    MeshNormals const *         normsPtr;   // OECS
    Vec2Fs const *          uvsPtr;
    Surfs                   surfs;      // Converted to tris
    Material                material;
    GridTriangles           grid;       // IUCS
    Floats                  invDepths;  // Inverse depth values. 1-1 with 'verts'
    Vec2Fs                  vertsIucs;  // Vertex projections into image plane

    Fg3dRayCastMesh(
        Mesh const &        mesh,       // Ignore base vertex positions here
        Vec3Fs const &      verts,      // Current OECS vertex positions.
        MeshNormals const &     normss,     // Current OECS normals.
        AffineEw2F          itcsToIucs);

    BestN<float,TriPoint,8>
    cast(Vec2F posIucs) const;

    RgbaF
    shade(const TriPoint & intersect,Lighting const & lighting) const;
};

struct  Fg3dRayCaster
{
    Lighting const *        lightingPtr;
    RgbaF                   m_background;
    Svec<Fg3dRayCastMesh>   rayMesh;

    Fg3dRayCaster(
        Meshes const &      meshes,
        Vec3Fss const &     vertss,         // Current OECS vertex positions. Must be 1-1 with above.
        MeshNormalss const &    normss,         // Current OECS normals. Must be 1-1 with above.
        Lighting const &    lighting,
        AffineEw2F          itcsToIucs,
        RgbaF               background);

    RgbaF
    cast(Vec2F posIucs) const;

    struct Intersect
    {
        size_t              surfIdx;
        TriPoint            intersect;

        Intersect() {}

        Intersect(size_t s,TriPoint i)
        : surfIdx(s), intersect(i)
        {}
    };
};

}

#endif

// */
