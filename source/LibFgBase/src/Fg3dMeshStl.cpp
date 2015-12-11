//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: Sept 22, 2011
//

#include "stdafx.h"
#include "Fg3dMeshIo.hpp"
#include "FgFileSystem.hpp"
#include "FgException.hpp"
#include "Fg3dNormals.hpp"

using namespace std;

static
void
saveStl(FgOfstream & ff,const Fg3dMesh & mesh)
{
    Fg3dNormals     norms = fgNormals(mesh);
    for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
        const Fg3dSurface &         surf = mesh.surfaces[ss];
        const Fg3dFacetNormals &    facetNorms = norms.facet[ss];
        for (uint ii=0; ii<surf.numTris(); ++ii) {
            FgVect3UI           tri = surf.getTri(ii);
            FgVect3F            norm = facetNorms.tri[ii];
            for (uint jj=0; jj<3; ++jj)
                ff.writeb(norm[jj]);
            for (uint jj=0; jj<3; ++jj) {
                FgVect3F        vert = mesh.verts[tri[jj]];
                for (uint kk=0; kk<3; ++kk)
                    ff.writeb(vert[kk]);
            }
            ff.writeb(uint16(0));
        }
        for (uint ii=0; ii<surf.numQuads(); ++ii) {
            FgVect4UI           quad = surf.getQuad(ii);
            FgVect3F            norm = facetNorms.quad[ii];
            for (uint jj=0; jj<3; ++jj)
                ff.writeb(norm[jj]);
            for (uint jj=0; jj<3; ++jj) {
                FgVect3F        vert = mesh.verts[quad[jj]];
                for (uint kk=0; kk<3; ++kk)
                    ff.writeb(vert[kk]);
            }
            ff.writeb(uint16(0));
            for (uint jj=0; jj<3; ++jj)
                ff.writeb(norm[jj]);
            for (uint jj=2; jj<5; ++jj) {
                FgVect3F        vert = mesh.verts[quad[jj%4]];
                for (uint kk=0; kk<3; ++kk)
                    ff.writeb(vert[kk]);
            }
            ff.writeb(uint16(0));
        }
    }
}

void
fgSaveStl(const FgString & fname,const vector<Fg3dMesh> & meshes)
{
    FGASSERT(!meshes.empty());
    FgOfstream      ff(fname);
    ff.write("STL Binary file exported by FaceGen                                            ",80);
    uint32          numTris = 0;
    for (size_t ii=0; ii<meshes.size(); ++ii)
        numTris += meshes[ii].numTriEquivs();
    ff.writeb(numTris);
    for (size_t ii=0; ii<meshes.size(); ++ii)
        saveStl(ff,meshes[ii]);
}
