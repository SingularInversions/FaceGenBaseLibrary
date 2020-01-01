//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "Fg3dMeshOps.hpp"
#include "Fg3dMeshIo.hpp"
#include "Fg3dNormals.hpp"
#include "FgStdStream.hpp"
#include "FgFileSystem.hpp"
#include "FgCommand.hpp"
#include "FgTestUtils.hpp"

using namespace std;

namespace Fg {

void
savePly(
    Ustring const &        fname,
    const vector<Mesh> & meshes,
    string                  imgFormat)
{
    Mesh    mesh = mergeMeshes(meshes);
    Path      path(fname);
    path.ext = "ply";
    Ofstream  ofs(path.str());
    ofs <<
        "ply\n"
        "format ascii 1.0\n"
        "comment created by FaceGen\n";
    size_t      imgCnt = 0;
    for (size_t ii=0; ii<mesh.surfaces.size(); ++ii) {
        if (mesh.surfaces[ii].material.albedoMap) {
            Ustring    texFile = path.base + toStr(ii) + "." + imgFormat;
            imgSaveAnyFormat(path.dir()+texFile,*mesh.surfaces[ii].material.albedoMap);
            ofs << "comment TextureFile " << texFile << "\n";
        }
    }
    ofs << "element vertex " << mesh.verts.size() << "\n"
        "property float x\n"
        "property float y\n"
        "property float z\n"
        "property float nx\n"
        "property float ny\n"
        "property float nz\n"
        "element face " << mesh.numTriEquivs() << "\n"
        "property list uchar int vertex_indices\n"
        "property list uchar float texcoord\n"
        "property int texnumber\n"
        "end_header\n";
    Normals         norms = cNormals(mesh);
    for (size_t vv=0; vv<mesh.verts.size(); ++vv) {
        Vec3F    pos = mesh.verts[vv],
                    nrm = norms.vert[vv];
        ofs << pos[0] << " " << pos[1] << " " << pos[2] << " " << nrm[0] << " " << nrm[1] << " " << nrm[2] << "\n";
    }
    imgCnt = 0;
    for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
        const Surf &     surf = mesh.surfaces[ss];
        FacetInds<3>          tris = surf.getTriEquivs();
        for (size_t ii=0; ii<tris.size(); ++ii) {
            Vec3UI           vinds = tris.vertInds[ii];
            ofs << "3 " << vinds[0] << " " << vinds[1] << " " << vinds[2] << " 6 ";
            if (surf.tris.uvInds.empty())
                ofs << "0 0 0 0 0 0 ";
            else {
                Vec3UI   uvInds = tris.uvInds[ii];
                for (uint vv=0; vv<3; ++vv) {
                    Vec2F    uv = mesh.uvs[uvInds[vv]];
                    ofs << uv[0] << " " << uv[1] << " ";
                }
            }
            ofs << imgCnt << "\n";
        }
        if (mesh.surfaces[ss].material.albedoMap)
            ++imgCnt;
    }
}

void
fgSavePlyTest(CLArgs const & args)
{
    FGTESTDIR
    Ustring            dd = dataDir();
    string              rd = "base/";
    Mesh            mouth = loadTri(dd+rd+"Mouth.tri");
    mouth.surfaces[0].setAlbedoMap(imgLoadAnyFormat(dd+rd+"MouthSmall.png"));
    Mesh            glasses = loadTri(dd+rd+"Glasses.tri");
    glasses.surfaces[0].setAlbedoMap(imgLoadAnyFormat(dd+rd+"Glasses.tga"));
    savePly("meshExportPly",fgSvec(mouth,glasses));
    regressFileRel("meshExportPly.ply","base/test/");
    regressFileRel("meshExportPly0.png","base/test/");
    regressFileRel("meshExportPly1.png","base/test/");
}

}

// */
