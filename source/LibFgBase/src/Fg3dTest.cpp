//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     December 11, 2009
//

#include "stdafx.h"

#include "FgTestUtils.hpp"
#include "Fg3dMeshOps.hpp"
#include "Fg3dMeshIo.hpp"
#include "FgApproxEqual.hpp"
#include "Fg3dDisplay.hpp"
#include "FgImgDisplay.hpp"
#include "FgCommand.hpp"
#include "Fg3dTopology.hpp"
#include "FgAffine1.hpp"

using namespace std;

static
void
addSubdivisions(
    vector<Fg3dMesh> &  meshes,
    const Fg3dMesh &    mesh)
{
    meshes.push_back(mesh);
    for (uint ii=0; ii<5; ++ii)
        meshes.push_back(fgSubdivide(meshes.back()));
}

static
void
test3dMeshSubdivision(const FgArgs &)
{
    Fg3dMesh        meanMesh = fgLoadTri(fgDataDir()+"base/Jane.tri");
    meanMesh.surfaces[0] = meanMesh.surfaces[0].convertToTris();
    fgout << meanMesh;
    // Test subdivision of surface points:
    size_t              numPts = meanMesh.surfPointNum();
    vector<FgVect3F>    surfPoints0(numPts);
    for (uint ii=0; ii<numPts; ++ii)
        surfPoints0[ii] = meanMesh.surfPointPos(ii);
    meanMesh = fgSubdivide(meanMesh,false);
    vector<FgVect3F>    surfPoints1(numPts);
    for (size_t ii=0; ii<numPts; ++ii) {
        surfPoints1[ii] = meanMesh.surfPointPos(ii);
        FGASSERT(
            fgApproxEqual(surfPoints0[ii][0],surfPoints1[ii][0]) &&
            fgApproxEqual(surfPoints0[ii][1],surfPoints1[ii][1]) &&
            fgApproxEqual(surfPoints0[ii][2],surfPoints1[ii][2]));
    }    
}

void
fg3dReadWobjTest(const FgArgs &)
{
    FgTestDir   tmp("readObj");
    ofstream    ofs("square.obj");
    ofs << 
        "v 0.0 0.0 0.0\n"
        "v 0.0 1.0 0.0\n"
        "v 1.0 1.0 0.0\n"
        "v 1.0 0.0 0.0\n"
        "vt 0.0 0.0\n"
        "vt 0.0 1.0\n"
        "vt 1.0 1.0\n"
        "vt 1.0 0.0\n"
        "g plane\n"
        "usemtl plane\n"
        "f 1/1 2/2 3/3 4/4\n"
        ;
    ofs.close();
    Fg3dMesh    mesh = fgLoadWobj("square.obj");
    fgDisplayMesh(mesh);
}

void
fgTextureImageMappingRenderTest(const FgArgs &)
{
    ofstream    ofs("square.obj");
    ofs << 
        "v 0.0 0.0 0.0\n"
        "v 0.0 1.0 0.0\n"
        "v 1.0 1.0 0.0\n"
        "v 1.0 0.0 0.0\n"
        "vt 0.0 0.0\n"
        "vt 0.0 1.0\n"
        "vt 1.0 1.0\n"
        "vt 1.0 0.0\n"
        "g plane\n"
        "usemtl plane\n"
        "f 1/1 2/2 3/3 4/4\n"
        ;
    ofs.close();
    Fg3dMesh    mesh = fgLoadWobj("square.obj");
    string      textureFile("base/test/TextureMapOrdering.jpg");
    fgLoadImgAnyFormat(fgDataDir()+textureFile,mesh.surfaces[0].albedoMapRef());
    fgDisplayMesh(mesh);
}

void
fgSubdivisionTest(const FgArgs &)
{
    vector<Fg3dMesh>    meshes;
    addSubdivisions(meshes,fgTetrahedron());
    addSubdivisions(meshes,fgTetrahedron(true));
    addSubdivisions(meshes,fgPyramid());
    addSubdivisions(meshes,fgPyramid(true));
    addSubdivisions(meshes,fgCube());
    addSubdivisions(meshes,fgCube(true));
    addSubdivisions(meshes,fgOctahedron());
    addSubdivisions(meshes,fgNTent(5));
    addSubdivisions(meshes,fgNTent(6));
    addSubdivisions(meshes,fgNTent(7));
    addSubdivisions(meshes,fgNTent(8));
    fgDisplayMeshes(meshes,true);
}

static
void
edgeDist(const FgArgs &)
{
    Fg3dMesh        mesh = fgLoadTri(fgDataDir()+"base/Jane.tri");
    Fg3dSurface     surf = fgMergeSurfaces(mesh.surfaces).convertToTris();
    Fg3dTopology    topo(mesh.verts,surf.tris.vertInds);
    size_t          vertIdx = 0;    // Randomly choose the first
    vector<float>   edgeDists = topo.edgeDistanceMap(mesh.verts,vertIdx);
    float           distMax = 0;
    for (size_t ii=0; ii<edgeDists.size(); ++ii)
        if (edgeDists[ii] < numeric_limits<float>::max())
            fgSetIfGreater(distMax,edgeDists[ii]);
    float           distToCol = 255.99f / distMax;
    vector<uchar>   colVal(edgeDists.size(),255);
    for (size_t ii=0; ii<colVal.size(); ++ii)
        if (edgeDists[ii] < numeric_limits<float>::max())
            colVal[ii] = uint(distToCol * edgeDists[ii]);
    mesh.surfaces[0].setAlbedoMap(FgImgRgbaUb(128,128,FgRgbaUB(255)));
    FgAffineCw2F    otcsToIpcs = fgOtcsToIpcs(FgVect2UI(128));
    for (size_t tt=0; tt<surf.tris.vertInds.size(); ++tt) {
        FgVect3UI   vertInds = surf.tris.vertInds[tt];
        FgVect3UI   uvInds = surf.tris.uvInds[tt];
        for (uint ii=0; ii<3; ++ii) {
            FgRgbaUB    col(255);
            col.red() = colVal[vertInds[ii]];
            col.green() = 255 - col.red();
            mesh.surfaces[0].albedoMap->paint(FgVect2UI(otcsToIpcs*mesh.uvs[uvInds[ii]]),col);
        }
    }
    fgDisplayMesh(mesh);
}

void    fgSave3dsTest(const FgArgs &);
void    fgSaveFbxTest(const FgArgs &);
void    fgSaveFgmeshTest(const FgArgs &);
void    fgSaveLwoTest(const FgArgs &);
void    fgSaveMaTest(const FgArgs &);
void    fgSavePlyTest(const FgArgs &);
void    fgSaveXsiTest(const FgArgs &);

void
fg3dTestMan(const FgArgs & args)
{
    vector<FgCmd>   cmds;
    cmds.push_back(FgCmd(edgeDist,"edgeDist"));
    cmds.push_back(FgCmd(test3dMeshSubdivision,"subdivision"));
    cmds.push_back(FgCmd(fgSave3dsTest,"3ds"));
    cmds.push_back(FgCmd(fgSaveFbxTest,"fbx"));
    cmds.push_back(FgCmd(fgSaveFgmeshTest,"fgmesh"));
    cmds.push_back(FgCmd(fgSaveLwoTest,"lwo"));
    cmds.push_back(FgCmd(fgSaveMaTest,"ma"));
    cmds.push_back(FgCmd(fgSavePlyTest,"ply"));
    cmds.push_back(FgCmd(fgSaveXsiTest,"xsi"));
    fgMenu(args,cmds,true,false,true);
}

// */
