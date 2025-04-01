//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "Fg3dMesh.hpp"
#include "Fg3dMeshIo.hpp"
#include "FgFile.hpp"
#include "FgFileSystem.hpp"
#include "FgCommand.hpp"
#include "FgTestUtils.hpp"

using namespace std;

namespace Fg {

void                savePly(String8 const & fname,Meshes const & meshes,String imgFormat)
{
    Mesh                mesh = mergeMeshes(meshes);
    Path                path {fname};
    path.ext = "ply";
    Ofstream            ofs {path.str()};
    ofs <<
        "ply\n"
        "format ascii 1.0\n"
        "comment created by FaceGen\n";
    size_t              imgCnt = 0;
    for (size_t ii=0; ii<mesh.surfaces.size(); ++ii) {
        if (mesh.surfaces[ii].material.albedoMap) {
            String8    texFile = path.base + toStr(ii) + "." + imgFormat;
            saveImage(*mesh.surfaces[ii].material.albedoMap,path.dir()+texFile);
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
    SurfNormals         norms = cNormals(mesh);
    for (size_t vv=0; vv<mesh.verts.size(); ++vv) {
        Vec3F               pos = mesh.verts[vv],
                            nrm = norms.vert[vv];
        ofs << pos[0] << " " << pos[1] << " " << pos[2] << " " << nrm[0] << " " << nrm[1] << " " << nrm[2] << "\n";
    }
    imgCnt = 0;
    for (Surf const & surf : mesh.surfaces) {
        NPolys<3>          tris = surf.getTriEquivs();
        for (size_t ii=0; ii<tris.size(); ++ii) {
            Arr3UI           vinds = tris.vertInds[ii];
            ofs << "3 " << vinds[0] << " " << vinds[1] << " " << vinds[2] << " 6 ";
            if (surf.tris.uvInds.empty())
                ofs << "0 0 0 0 0 0 ";
            else {
                Arr3UI   uvInds = tris.uvInds[ii];
                for (uint vv=0; vv<3; ++vv) {
                    Vec2F    uv = mesh.uvs[uvInds[vv]];
                    ofs << uv[0] << " " << uv[1] << " ";
                }
            }
            ofs << imgCnt << "\n";
        }
        if (surf.material.albedoMap)
            ++imgCnt;
    }
}

void                testSavePly(CLArgs const & args)
{
    FGTESTDIR
    String8             dd = dataDir();
    string              rd = "base/";
    Mesh                mouth = loadTri(dd+rd+"Mouth.tri");
    mouth.surfaces[0].setAlbedoMap(loadImage(dd+rd+"MouthSmall.png"));
    Mesh                glasses = loadTri(dd+rd+"Glasses.tri");
    glasses.surfaces[0].setAlbedoMap(loadImage(dd+rd+"Glasses.tga"));
    savePly("meshExportPly",{mouth,glasses});
    if (is64Bit() && (getCurrentCompiler()==Compiler::vs22))      // precision differences otherwise
        regressFileRel("meshExportPly.ply","base/test/");
    regressFileRel("meshExportPly0.png","base/test/");
    regressFileRel("meshExportPly1.png","base/test/");
}

}

// */
