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

using namespace std;

static
void
addSubdivisions(
    vector<Fg3dMesh> &  meshes,
    const Fg3dMesh &    mesh)
{
    meshes.push_back(mesh);
    for (uint ii=0; ii<5; ++ii)
        meshes.push_back(meshes.back().subdivideLoop());
}

static
void
test3dMeshSubdivision(const FgArgs &)
{
    Fg3dMesh        meanMesh = fgLoadTri(fgDataDir()+"base/Jane.tri");
    meanMesh.surfaces[0] = meanMesh.surfaces[0].convertToTris();
    fgout << meanMesh;
    // Test subdivision of surface points:
    uint                numPts = meanMesh.numSurfPoints();
    vector<FgVect3F>    surfPoints0(numPts);
    for (uint ii=0; ii<numPts; ++ii)
        surfPoints0[ii] = meanMesh.getSurfPoint(ii);
    meanMesh = meanMesh.subdivideFlat();
    vector<FgVect3F>    surfPoints1(numPts);
    for (uint ii=0; ii<numPts; ++ii) {
        surfPoints1[ii] = meanMesh.getSurfPoint(ii);
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
    Fg3dMesh    mesh;
    fgLoadWobj("square.obj",mesh);
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
    Fg3dMesh    mesh;
    fgLoadWobj("square.obj",mesh);
    string      textureFile("base/test/TextureMapOrdering.jpg");    
    FgImgRgbaUb texture;
    fgLoadImgAnyFormat(fgDataDir()+textureFile,texture);
    mesh.texImages.push_back(texture);
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

void    fgSave3dsTest(const FgArgs &);
void    fgSaveFbxTest(const FgArgs &);
void    fgSaveLwoTest(const FgArgs &);
void    fgSaveMaTest(const FgArgs &);
void    fgSavePlyTest(const FgArgs &);
void    fgSaveXsiTest(const FgArgs &);

void
fg3dTest(const FgArgs & args)
{
    vector<FgCmd>   cmds;
    cmds.push_back(FgCmd(test3dMeshSubdivision,"subdivision"));
    cmds.push_back(FgCmd(fgSave3dsTest,"3ds"));
    cmds.push_back(FgCmd(fgSaveFbxTest,"fbx"));
    cmds.push_back(FgCmd(fgSaveLwoTest,"lwo"));
    cmds.push_back(FgCmd(fgSaveMaTest,"ma"));
    cmds.push_back(FgCmd(fgSavePlyTest,"ply"));
    cmds.push_back(FgCmd(fgSaveXsiTest,"xsi"));
    fgMenu(args,cmds,true);
}

// */
