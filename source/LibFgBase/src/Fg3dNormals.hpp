//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Compute normals for a mesh

#ifndef FG3DNORMALS_HPP
#define FG3DNORMALS_HPP

#include "FgStdLibs.hpp"

#include "FgMatrixC.hpp"
#include "FgMatrixV.hpp"
#include "Fg3dMesh.hpp"

namespace Fg {

Vec3Ds
cVertNorms(Vec3Ds const & verts,Vec3UIs const & tris);

struct  FacetNormals
{
    Vec3Fs               tri;        // Tri facet normals for a surface
    Vec3Fs               quad;       // Quad facet normals for a surface

    Vec3F
    triEquiv(size_t idx) const
    {
        if (idx < tri.size())
            return tri[idx];
        idx -= tri.size();
        return quad[idx/2];
    }
};
typedef Svec<FacetNormals>      FacetNormalss;

struct  Normals
{
    FacetNormalss       facet;       // Facet normals for each surface
    Vec3Fs              vert;        // Vertex normals.
};
typedef Svec<Normals>           Normalss;

Normals
cNormals(Surfs const & surfs,Vec3Fs const & verts);

inline
Normals
cNormals(Mesh const & mesh)
{return cNormals(mesh.surfaces,mesh.verts); }

}

#endif
