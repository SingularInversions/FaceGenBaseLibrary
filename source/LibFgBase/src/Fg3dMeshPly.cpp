//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: Oct 21, 2015
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

void
fgSavePly(
    const FgString &        fname,
    const vector<Fg3dMesh> & meshes,
    string                  imgFormat)
{
    Fg3dMesh    mesh = fgMergeMeshes(meshes);
    FgPath      path(fname);
    path.ext = "ply";
    FgOfstream  ofs(path.str());
    ofs <<
        "ply\n"
        "format ascii 1.0\n"
        "comment created by FaceGen\n";
    size_t      imgCnt = 0;
    for (size_t ii=0; ii<mesh.texImages.size(); ++ii) {
        if (!mesh.texImages.empty()) {
            FgString    texFile = path.base + fgToString(ii) + "." + imgFormat;
            fgSaveImgAnyFormat(path.dir()+texFile,mesh.texImages[ii]);
            ofs << "comment TextureFile " << texFile << endl;
        }
    }
    ofs << "element vertex " << mesh.verts.size() << endl
        << "property float x" << endl
        << "property float y" << endl
        << "property float z" << endl
        << "property float nx" << endl
        << "property float ny" << endl
        << "property float nz" << endl
        << "element face " << mesh.numTriEquivs() << endl
        << "property list uchar int vertex_indices" << endl
        << "property list uchar float texcoord" << endl
        << "property int texnumber" << endl
        << "end_header" << endl;
    Fg3dNormals         norms = fgNormals(mesh);
    for (size_t vv=0; vv<mesh.verts.size(); ++vv) {
        FgVect3F    pos = mesh.verts[vv],
                    nrm = norms.vert[vv];
        ofs << pos[0] << " " << pos[1] << " " << pos[2] << " " << nrm[0] << " " << nrm[1] << " " << nrm[2] << endl;
    }
    imgCnt = 0;
    for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
        const Fg3dSurface &     surf = mesh.surfaces[ss];
        FgFacetInds<3>          tris = surf.getTriEquivs();
        for (size_t ii=0; ii<tris.vertInds.size(); ++ii) {
            FgVect3UI           vinds = tris.vertInds[ii];
            ofs << "3 " << vinds[0] << " " << vinds[1] << " " << vinds[2] << " 6 ";
            if (surf.tris.uvInds.empty())
                ofs << "0 0 0 0 0 0 ";
            else {
                FgVect3UI   uvInds = tris.uvInds[ii];
                for (uint vv=0; vv<3; ++vv) {
                    FgVect2F    uv = mesh.uvs[uvInds[vv]];
                    ofs << uv[0] << " " << uv[1] << " ";
                }
            }
            ofs << imgCnt << endl;
        }
        if ((ss < mesh.texImages.size()) && (!mesh.texImages[ss].empty()))
            ++imgCnt;
    }
}

void
fgSavePlyTest(const FgArgs & args)
{
    FGTESTDIR
    FgString            dd = fgDataDir();
    string              rd = "base/";
    vector<Fg3dMesh>    meshes;
    meshes.push_back(fgLoadTri(dd+rd+"Mouth.tri"));
    meshes.back().texImages.push_back(fgLoadImgAnyFormat(dd+rd+"Mouth.tga"));
    meshes.push_back(fgLoadTri(dd+rd+"Glasses.tri"));
    meshes.back().texImages.push_back(fgLoadImgAnyFormat(dd+rd+"Glasses.tga"));
    fgSavePly("meshExportPly",meshes);
    fgRegressFile("meshExportPly.ply","base/test/");
    fgRegressFile("meshExportPly0.png","base/test/");
    fgRegressFile("meshExportPly1.png","base/test/");
}

// */
