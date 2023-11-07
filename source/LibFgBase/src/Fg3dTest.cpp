//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgTestUtils.hpp"
#include "Fg3dMesh.hpp"
#include "Fg3dMeshIo.hpp"
#include "FgApproxEqual.hpp"
#include "Fg3dDisplay.hpp"
#include "FgImgDisplay.hpp"
#include "FgCommand.hpp"
#include "FgAffine.hpp"
#include "FgBuild.hpp"

using namespace std;

namespace Fg {

void                fg3dReadWobjTest(CLArgs const &)
{
    TestDir   tmp("readObj");
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
    Mesh    mesh = loadWObj("square.obj");
    viewMesh(mesh);
}

void                fgTextureImageMappingRenderTest(CLArgs const &)
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
    Mesh        mesh = loadWObj("square.obj");
    string      textureFile("base/test/TextureMapOrdering.jpg");
    loadImage_(dataDir()+textureFile,mesh.surfaces[0].albedoMapRef());
    viewMesh(mesh);
}

void                test3dMeshIo(CLArgs const &);
void                test3dMesh(CLArgs const &);

void                test3d(CLArgs const & args)
{
    Cmds            cmds {
        {test3dMeshIo, "io", "3D Mesh I/O"},
        {test3dMesh, "mesh", "mesh operations"},
    };
    doMenu(args,cmds,true,false);
}

void                testmSubdFace(CLArgs const &)
{
    Mesh            base = loadMeshMaps(dataDir()+"base/Jane");
    base.surfaces[0] = base.surfaces[0].convertToTris();
    Mesh            subd = subdivide(base,true);
    viewMesh({base,subd},true);
}

void                testmSubdShapes(CLArgs const &)
{
    Meshes          meshes;
    auto            addSubdivisions = [&](TriSurf const & ts,String const & name)
    {
        Mesh            mesh {name,ts};
        meshes.push_back(mesh);
        for (uint ii=0; ii<5; ++ii) {
            mesh = subdivide(mesh);
            mesh.name = name+toStr(ii+1);
            meshes.push_back(mesh);
        }
    };
    addSubdivisions(cTetrahedron(),"Tetrahedron");
    addSubdivisions(cTetrahedron(true),"TetrahedronOpen");
    addSubdivisions(cPyramid(),"Pyramid");
    addSubdivisions(cPyramid(true),"PyramidOpen");
    addSubdivisions(cCubeTris(),"Cube");
    addSubdivisions(cCubeTris(true),"CubeOpen");
    addSubdivisions(cOctahedron(),"Octahedron");
    addSubdivisions(cIcosahedron(),"Icosahedron");
    addSubdivisions(cNTent(5),"5-Tent");
    addSubdivisions(cNTent(6),"6-Tent");
    addSubdivisions(cNTent(7),"7-Tent");
    addSubdivisions(cNTent(8),"8-Tent");
    viewMesh(meshes,true);
}

void                testmSphere4(CLArgs const &)
{
    Meshes          meshes;
    for (size_t ii=0; ii<5; ++ii)
        meshes.emplace_back("TetrahedronSphere",cSphere4(ii));
    viewMesh(meshes,true);
}

void                testmSphere(CLArgs const &)
{
    Meshes          meshes;
    for (size_t ii=0; ii<4; ++ii)
        meshes.emplace_back("Sphere",cSphere(ii));
    viewMesh(meshes,true);
}

void                testmSquarePrism(CLArgs const &)
{
    viewMesh(Mesh{"SquarePrism",cSquarePrism(1,4)});
}

void                testmInterp(CLArgs const &)
{
    // interpolate colors at every discrete step between two pixel centres
    ImgRgba8            clrMap {2,1,{{0,255,0,255},{255,0,0,255}}};
    Vec3Fs              verts {{0,0,0},{0,32,0}};
    Vec2Fs              uvs;
    Vec3UIs             vinds;
    Vec3UIs             tinds;
    for (uint ii=0; ii<256; ++ii) {
        verts.emplace_back(ii+1,0,0);
        verts.emplace_back(ii+1,32,0);
        uint                i2 = 2*ii;
        vinds.emplace_back(i2,i2+2,i2+1);
        vinds.emplace_back(i2+1,i2+2,i2+3);
        uvs.emplace_back(ii/255.0f,0.5f);
        tinds.emplace_back(ii);
        tinds.emplace_back(ii);
    }
    Surf                surf {TriInds{vinds,tinds},{}};
    surf.setAlbedoMap(clrMap);
    Mesh                mesh {"Interpolated color gradient",verts,uvs,{surf}};
    viewMesh(mesh);
}



void                testm3d(CLArgs const & args)
{
    Cmds            cmds {
        {testmInterp,"interp","interpolation of color maps"},
        {testmSquarePrism,"prism","Square prism"},
        {testmSubdShapes,"subd0","Loop subdivsion of simple shapes"},
        {testmSubdFace,"subd1","Loop subdivision of textured face"},
        {testmSphere4,"sphere4","Spheres created from tetrahedon"},
        {testmSphere,"sphere","Spheres created from icosahedron"},
    };
    doMenu(args,cmds,true,false);
}

}

// */
