//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "Fg3dNormals.hpp"

using namespace std;

namespace Fg {

template<class T>
static Vec3D
cFacetNorm(Svec<Mat<T,3,1> > const & verts,Vec3UI tri)
{
    Vec3D       v0(verts[tri[0]]),
                v1(verts[tri[1]]),
                v2(verts[tri[2]]);
    Vec3D       cross = crossProduct((v1-v0),(v2-v0)),    // CC winding
                norm;
    double      crossLen = cross.len();
    if (crossLen == 0.0)
        norm = Vec3D(0);
    else
        norm = cross * (1.0 / crossLen);
    return norm;
}

Vec3Ds
cVertNorms(Vec3Ds const & verts,Vec3UIs const & tris)
{
    Vec3Ds          vertNorms(verts.size(),Vec3D(0));
    for (Vec3UI tri : tris) {
        Vec3D       norm = cFacetNorm(verts,tri);
        vertNorms[tri[0]] += norm;
        vertNorms[tri[1]] += norm;
        vertNorms[tri[2]] += norm;
    }
    Vec3Ds          ret;
    ret.reserve(verts.size());
    for (Vec3D norm : vertNorms) {
        double          len = cLen(norm);
        if(len > 0.0)
            ret.push_back(norm/len);
        else
            ret.push_back(Vec3D(0,0,1));     // Arbitrary
    }
    return ret;
}

// Vertex normals are just approximated by a simple average of the facet normals of all
// facets containing the vertex:
MeshNormals
cNormals(Surfs const & surfs,Vec3Fs const & verts)
{
    MeshNormals             norms;
    norms.facet.resize(surfs.size());
    Vec3Ds              vertNorms(verts.size(),Vec3D(0));
    // Calculate facet normals and accumulate unnormalized vertex normals:
    for (size_t ss=0; ss<surfs.size(); ss++) {
        Surf const &        surf = surfs[ss];
        FacetNormals &      fnorms = norms.facet[ss];
        fnorms.tri.reserve(surf.numTris());
        fnorms.quad.reserve(surf.numQuads());
        // TRIs
        for (Vec3UI tri : surf.tris.posInds) {
            Vec3D       norm = cFacetNorm(verts,tri);
            vertNorms[tri[0]] += norm;
            vertNorms[tri[1]] += norm;
            vertNorms[tri[2]] += norm;
            Vec3F       normf(norm);
            fnorms.tri.push_back(normf);
        }
        // QUADs
        // This least squares surface normal is taken from [Mantyla 87]:
        for (Vec4UI quad : surf.quads.posInds) {
            Vec3D       v0(verts[quad[0]]),
                        v1(verts[quad[1]]),
                        v2(verts[quad[2]]),
                        v3(verts[quad[3]]);
            Vec3D       cross,norm;
            cross[0] =  (v0[1]-v1[1]) * (v0[2]+v1[2]) +
                        (v1[1]-v2[1]) * (v1[2]+v2[2]) +
                        (v2[1]-v3[1]) * (v2[2]+v3[2]) +
                        (v3[1]-v0[1]) * (v3[2]+v0[2]);
            cross[1] =  (v0[2]-v1[2]) * (v0[0]+v1[0]) +
                        (v1[2]-v2[2]) * (v1[0]+v2[0]) +
                        (v2[2]-v3[2]) * (v2[0]+v3[0]) +
                        (v3[2]-v0[2]) * (v3[0]+v0[0]);
            cross[2] =  (v0[0]-v1[0]) * (v0[1]+v1[1]) +
                        (v1[0]-v2[0]) * (v1[1]+v2[1]) +
                        (v2[0]-v3[0]) * (v2[1]+v3[1]) +
                        (v3[0]-v0[0]) * (v3[1]+v0[1]);
            double      crossMag = cross.len();
            if (crossMag == 0.0)
                norm = Vec3D(0);
            else
                norm = cross * (1.0 / crossMag);
            vertNorms[quad[0]] += norm;
            vertNorms[quad[1]] += norm;
            vertNorms[quad[2]] += norm;
            vertNorms[quad[3]] += norm;
            Vec3F       normf(norm);
            fnorms.quad.push_back(normf);
        }
    }
    // Normalize vertex normals:
    norms.vert.reserve(verts.size());
    for (Vec3D norm : vertNorms) {
        double          len = cLen(norm);
        if(len > 0.0)
            norms.vert.push_back(Vec3F(norm/len));
        else
            norms.vert.push_back(Vec3F(0,0,1));     // Arbitrary
    }
    return norms;
}

}
