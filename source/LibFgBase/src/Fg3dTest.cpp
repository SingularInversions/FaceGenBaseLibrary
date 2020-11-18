//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
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
#include "FgBuild.hpp"
#include "FgCoordSystem.hpp"

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

static
void
edgeDist(CLArgs const &)
{
    Mesh                mesh = loadTri(dataDir()+"base/Jane.tri");
    Surf                surf = mergeSurfaces(mesh.surfaces).convertToTris();
    MeshTopology        topo(mesh.verts.size(),surf.tris.posInds);
    size_t              vertIdx = 0;    // Randomly choose the first
    vector<float>       edgeDists = topo.edgeDistanceMap(mesh.verts,vertIdx);
    float               distMax = 0;
    for (size_t ii=0; ii<edgeDists.size(); ++ii)
        if (edgeDists[ii] < maxFloat())
            setIfGreater(distMax,edgeDists[ii]);
    float               distToCol = 255.99f / distMax;
    Uchars              colVal(edgeDists.size(),255);
    for (size_t ii=0; ii<colVal.size(); ++ii)
        if (edgeDists[ii] < maxFloat())
            colVal[ii] = uint(distToCol * edgeDists[ii]);
    mesh.surfaces[0].setAlbedoMap(ImgC4UC(128,128,RgbaUC(255)));
    AffineEw2F          otcsToIpcs = cOtcsToIpcs(Vec2UI(128));
    for (size_t tt=0; tt<surf.tris.size(); ++tt) {
        Vec3UI              vertInds = surf.tris.posInds[tt];
        Vec3UI              uvInds = surf.tris.uvInds[tt];
        for (uint ii=0; ii<3; ++ii) {
            RgbaUC          col(255);
            col.red() = colVal[vertInds[ii]];
            col.green() = 255 - col.red();
            mesh.surfaces[0].material.albedoMap->paint(Vec2UI(otcsToIpcs*mesh.uvs[uvInds[ii]]),col);
        }
    }
    viewMesh(mesh);
}

void fgSave3dsTest(CLArgs const &);
void fgSaveLwoTest(CLArgs const &);
void fgSaveMaTest(CLArgs const &);
void fgSaveFbxTest(CLArgs const &);
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
        {fgSaveLwoTest,"lwo","Lightwve object file format export"},
        {fgSaveMaTest,"ma","Maya ASCII file format export"},
        {testSaveDae, "dae", "Collada DAE format export"},
        {fgSaveFbxTest, "fbx", ".FBX file format export"},
        {fgSaveObjTest, "obj", "Wavefront OBJ ASCII file format export"},
        {fgSavePlyTest, "ply", ".PLY file format export"},
        {testVrmlSave,  "vrml", ".WRL file format export"},
#ifdef _MSC_VER     // Precision differences with gcc/clang:
        {fgSaveXsiTest, "xsi", ".XSI file format export"},
#endif
    };
    doMenu(args,cmds,true,false,true);
}

void fgSaveFgmeshTest(CLArgs const &);

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
    addSubdivisions(cTetrahedron(),"Tetrahedron");
    addSubdivisions(cTetrahedron(true),"TetrahedronOpen");
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
testm3d(CLArgs const & args)
{
    Cmds            cmds {
        {edgeDist,"edgeDist"},
        {fgSaveFgmeshTest,"fgmesh","FaceGen mesh file format export"},  // Uses GUI
        {testmSubdShapes,"subd0","Loop subdivsion of simple shapes"},
        {testmSubdFace,"subd1","Loop subdivision of textured face"},
    };
    doMenu(args,cmds,true,false,true);
}

}

// */
