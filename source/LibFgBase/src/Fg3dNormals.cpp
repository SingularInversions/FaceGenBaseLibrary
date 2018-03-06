//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Aug 22, 2005
//

#include "stdafx.h"

#include "Fg3dNormals.hpp"

using namespace std;

// Vertex normals are just approximated by a simple average of the facet normals of all
// facets containing the vertex:
void
fgNormals_(
    const vector<Fg3dSurface> & surfs,
    const FgVerts &             verts,
    Fg3dNormals &               norms)
{
    FgVect3F               zero(0.0f,0.0f,0.0f);
    norms.vert.clear();
    norms.vert.resize(verts.size(),zero);
    norms.facet.resize(surfs.size());

    // Calculate facet normals and accumulate unnormalized vertex normals:
    for (size_t ss=0; ss<surfs.size(); ss++) {
        const Fg3dSurface &     surf(surfs[ss]);
        Fg3dFacetNormals &      fnorms(norms.facet[ss]);
        fnorms.tri.resize(surf.numTris());
        fnorms.quad.resize(surf.numQuads());

        // TRIs
        for (uint ii=0; ii<surf.numTris(); ii++) {
            FgVect3UI   tri = surf.getTri(ii);
            FgVect3F    v0 = verts[tri[0]],
                        v1 = verts[tri[1]],
                        v2 = verts[tri[2]];
            FgVect3F    cross = fgCrossProduct((v1-v0),(v2-v0)),    // CC winding
                        norm;
            float       crossMag = cross.length();
            if (crossMag == 0.0f)
                norm = zero;
            else
                norm = cross * (1.0f / crossMag);
            fnorms.tri[ii] = norm;
            norms.vert[tri[0]] += norm;
            norms.vert[tri[1]] += norm;
            norms.vert[tri[2]] += norm;
        }

        // QUADs
        // This least squares surface normal is taken from [Mantyla 87]:
        for (uint ii=0; ii<surf.numQuads(); ii++) {
            FgVect4UI   quad = surf.getQuad(ii);
            FgVect3F    v0 = verts[quad[0]],
                        v1 = verts[quad[1]],
                        v2 = verts[quad[2]],
                        v3 = verts[quad[3]];
            FgVect3F   cross,norm;
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
            float       crossMag = cross.length();
            if (crossMag == 0.0f)
                norm = zero;
            else
                norm = cross * (1.0f / crossMag);
            fnorms.quad[ii] = norm;
            norms.vert[quad[0]] += norm;
            norms.vert[quad[1]] += norm;
            norms.vert[quad[2]] += norm;
            norms.vert[quad[3]] += norm;
        }
    }

    // Normalize vertex normals:
    for (size_t ii=0; ii<norms.vert.size(); ++ii) {
        float       val = norms.vert[ii].length();
        if(val > 0.0f)
            norms.vert[ii] *= (1.0f / val);
    }

    return;
}
