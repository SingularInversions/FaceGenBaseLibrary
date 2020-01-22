//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
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

typedef std::function<RgbaF(Vec3F,Vec2F,Material,const ImgC4UC *)>   FgFuncShader;

struct  Fg3dRayCastMesh
{
    Vec3Fs const *             vertsPtr;   // OECS
    const Normals *         normsPtr;   // OECS
    Vec2Fs const *           uvsPtr;
    Surfs                surfs;      // Converted to tris
    Material                  material;
    GridTriangles             grid;       // IUCS
    Floats                      invDepths;  // Inverse depth values. 1-1 with 'verts'
    Vec2Fs                   vertsIucs;  // Vertex projections into image plane

    Fg3dRayCastMesh(
        Mesh const &        mesh,       // Ignore base vertex positions here
        Vec3Fs const &         verts,      // Current OECS vertex positions.
        const Normals &     normss,     // Current OECS normals.
        AffineEw2F            itcsToIucs);

    FgBestN<float,TriPoint,8>
    cast(Vec2F posIucs) const;

    RgbaF
    shade(const TriPoint & intersect,const Lighting & lighting) const;
};

struct  Fg3dRayCaster
{
    const Lighting *          lightingPtr;
    RgbaF                     m_background;
    Svec<Fg3dRayCastMesh>     rayMesh;

    Fg3dRayCaster(
        const Meshes &      meshes,
        const Vec3Fss &        vertss,         // Current OECS vertex positions. Must be 1-1 with above.
        const Normalss &    normss,         // Current OECS normals. Must be 1-1 with above.
        const Lighting &      lighting,
        AffineEw2F            itcsToIucs,
        RgbaF                 background);

    RgbaF
    cast(Vec2F posIucs) const;

    struct Best
    {
        size_t                  surfIdx;
        TriPoint              intersect;

        Best() {}

        Best(size_t s,TriPoint i)
        : surfIdx(s), intersect(i)
        {}
    };
};

}

#endif

// */
