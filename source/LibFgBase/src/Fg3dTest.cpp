//
// Copyright (c) 2022 Singular Inversions Inc. (facegen.com)
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

void                testm3d(CLArgs const & args)
{
    Cmds            cmds {
        {testmSubdShapes,"subd0","Loop subdivsion of simple shapes"},
        {testmSubdFace,"subd1","Loop subdivision of textured face"},
        {testmSphere4,"sphere4","Spheres created from tetrahedon"},
        {testmSphere,"sphere","Spheres created from icosahedron"},
        {testmSquarePrism,"prism","Square prism"},
    };
    doMenu(args,cmds,true,false);
}

}

// */
