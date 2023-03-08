//
// Copyright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"
#include "Fg3dMeshIo.hpp"
#include "FgFileSystem.hpp"
#include "FgSerial.hpp"

using namespace std;

namespace Fg {

static
void
saveStl(Ofstream & ff,Mesh const & mesh)
{
    MeshNormals     norms = cNormals(mesh);
    for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
        Surf const &         surf = mesh.surfaces[ss];
        const FacetNormals &    facetNorms = norms.facet[ss];
        for (uint ii=0; ii<surf.numTris(); ++ii) {
            Vec3UI           tri = surf.tris.vertInds[ii];
            Vec3F            norm = facetNorms.tri[ii];
            for (uint jj=0; jj<3; ++jj)
                writeBinRaw_(ff,norm[jj]);
            for (uint jj=0; jj<3; ++jj) {
                Vec3F        vert = mesh.verts[tri[jj]];
                for (uint kk=0; kk<3; ++kk)
                    writeBinRaw_(ff,vert[kk]);
            }
            writeBinRaw_(ff,uint16(0));
        }
        for (uint ii=0; ii<surf.numQuads(); ++ii) {
            Vec4UI           quad = surf.quads.vertInds[ii];
            Vec3F            norm = facetNorms.quad[ii];
            for (uint jj=0; jj<3; ++jj)
                writeBinRaw_(ff,norm[jj]);
            for (uint jj=0; jj<3; ++jj) {
                Vec3F        vert = mesh.verts[quad[jj]];
                for (uint kk=0; kk<3; ++kk)
                    writeBinRaw_(ff,vert[kk]);
            }
            writeBinRaw_(ff,uint16(0));
            for (uint jj=0; jj<3; ++jj)
                writeBinRaw_(ff,norm[jj]);
            for (uint jj=2; jj<5; ++jj) {
                Vec3F        vert = mesh.verts[quad[jj%4]];
                for (uint kk=0; kk<3; ++kk)
                    writeBinRaw_(ff,vert[kk]);
            }
            writeBinRaw_(ff,uint16(0));
        }
    }
}

void
saveStl(String8 const & fname,Meshes const & meshes)
{
    FGASSERT(!meshes.empty());
    Ofstream      ff(fname);
    ff.write("STL Binary file exported by FaceGen                                            ",80);
    uint32          numTris = 0;
    for (size_t ii=0; ii<meshes.size(); ++ii)
        numTris += meshes[ii].numTriEquivs();
    writeBinRaw_(ff,numTris);
    for (size_t ii=0; ii<meshes.size(); ++ii)
        saveStl(ff,meshes[ii]);
}

}
