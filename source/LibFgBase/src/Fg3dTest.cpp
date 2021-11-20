//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
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

void
fg3dReadWobjTest(CLArgs const &)
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

void
fgTextureImageMappingRenderTest(CLArgs const &)
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

void fgSave3dsTest(CLArgs const &);
void fgSaveLwoTest(CLArgs const &);
void fgSaveMaTest(CLArgs const &);
void testSaveFbx(CLArgs const &);
void testSaveDae(CLArgs const &);
void fgSaveObjTest(CLArgs const &);
void fgSavePlyTest(CLArgs const &);
void fgSaveXsiTest(CLArgs const &);
void testVrmlSave(CLArgs const &);

void
test3d(CLArgs const & args)
{
    Cmds            cmds {
        {fgSave3dsTest,"3ds",".3DS file format export"},
#ifdef _MSC_VER     // Precision differences with gcc/clang:
        {fgSaveLwoTest,"lwo","Lightwve object file format export"},
        {fgSaveMaTest,"ma","Maya ASCII file format export"},
        {testSaveFbx, "fbx", ".FBX file format export"},
        {fgSaveObjTest, "obj", "Wavefront OBJ ASCII file format export"},
        {fgSavePlyTest, "ply", ".PLY file format export"},
        {testVrmlSave,  "vrml", ".WRL file format export"},
        {testSaveDae, "dae", "Collada DAE format export"},
        {fgSaveXsiTest, "xsi", ".XSI file format export"},
#endif
    };
    doMenu(args,cmds,true,false,true);
}

void
testmSubdFace(CLArgs const &)
{
    Mesh            base = loadMeshMaps(dataDir()+"base/Jane");
    base.surfaces[0] = base.surfaces[0].convertToTris();
    Mesh            subd = subdivide(base,true);
    viewMesh({base,subd},true);
}

void
testmSubdShapes(CLArgs const &)
{
    Meshes          meshes;
    auto            addSubdivisions = [&](Mesh mesh,String const & name)
    {
        mesh.name = name;
        meshes.push_back(mesh);
        for (uint ii=0; ii<5; ++ii) {
            mesh = subdivide(mesh);
            mesh.name = name+toStr(ii+1);
            meshes.push_back(mesh);
        }
    };
    addSubdivisions(Mesh{cTetrahedron()},"Tetrahedron");
    addSubdivisions(Mesh{cTetrahedron(true)},"TetrahedronOpen");
    addSubdivisions(cPyramid(),"Pyramid");
    addSubdivisions(cPyramid(true),"PyramidOpen");
    addSubdivisions(c3dCube(),"Cube");
    addSubdivisions(c3dCube(true),"CubeOpen");
    addSubdivisions(Mesh{cOctahedron()},"Octahedron");
    addSubdivisions(Mesh{cIcosahedron()},"Icosahedron");
    addSubdivisions(cNTent(5),"5-Tent");
    addSubdivisions(cNTent(6),"6-Tent");
    addSubdivisions(cNTent(7),"7-Tent");
    addSubdivisions(cNTent(8),"8-Tent");
    viewMesh(meshes,true);
}

void
testmSphere4(CLArgs const &)
{
    Meshes          meshes;
    for (size_t ii=0; ii<5; ++ii)
        meshes.emplace_back(cSphere4(ii));
    viewMesh(meshes,true);
}

void
testmSphere(CLArgs const &)
{
    Meshes          meshes;
    for (size_t ii=0; ii<4; ++ii)
        meshes.emplace_back(cSphere(ii));
    viewMesh(meshes,true);
}

void
testm3d(CLArgs const & args)
{
    Cmds            cmds {
        {testmSubdShapes,"subd0","Loop subdivsion of simple shapes"},
        {testmSubdFace,"subd1","Loop subdivision of textured face"},
        {testmSphere4,"sphere4","Spheres created from tetrahedon"},
        {testmSphere,"sphere","Spheres created from icosahedron"},
    };
    doMenu(args,cmds,true,false,true);
}

}

// */
