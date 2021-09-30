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
#include "Fg3dTopology.hpp"
#include "FgAffine1.hpp"
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

static
void
edgeDist(CLArgs const &)
{
    Mesh                mesh = loadTri(dataDir()+"base/Jane.tri");
    Surf                surf = mergeSurfaces(mesh.surfaces).convertToTris();
    SurfTopo        topo(mesh.verts.size(),surf.tris.posInds);
    size_t              vertIdx = 0;    // Randomly choose the first
    Floats       edgeDists = topo.edgeDistanceMap(mesh.verts,vertIdx);
    float               distMax = 0;
    for (size_t ii=0; ii<edgeDists.size(); ++ii)
        if (edgeDists[ii] < floatMax())
            updateMax_(distMax,edgeDists[ii]);
    float               distToCol = 255.99f / distMax;
    Uchars              colVal(edgeDists.size(),255);
    for (size_t ii=0; ii<colVal.size(); ++ii)
        if (edgeDists[ii] < floatMax())
            colVal[ii] = uint(distToCol * edgeDists[ii]);
    mesh.surfaces[0].setAlbedoMap(ImgRgba8(128,128,RgbaUC(255)));
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
#ifdef _MSC_VER     // Precision differences with gcc/clang:
        {fgSaveLwoTest,"lwo","Lightwve object file format export"},
        {fgSaveMaTest,"ma","Maya ASCII file format export"},
        {fgSaveFbxTest, "fbx", ".FBX file format export"},
        {fgSaveObjTest, "obj", "Wavefront OBJ ASCII file format export"},
        {fgSavePlyTest, "ply", ".PLY file format export"},
        {testVrmlSave,  "vrml", ".WRL file format export"},
        {testSaveDae, "dae", "Collada DAE format export"},
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
testmSurfTopo(CLArgs const &)
{
    // Test boundary vert normals by adding marked verts along normals and viewing:
    Mesh                mesh = loadTri(dataDir()+"base/JaneLoresFace.tri");
    TriSurf             triSurf = mesh.asTriSurf();
    Vec3Ds              verts = mapCast<Vec3D>(triSurf.verts);
    double              scale = cMax(cDims(verts).m) * 0.01;        // Extend norms 1% of max dim
    SurfTopo            topo {triSurf.verts.size(),triSurf.tris};
    Svec<SurfTopo::BoundEdges> boundaries = topo.boundaries();
    for (auto const & boundary : boundaries) {
        Vec3Ds              vertNorms = topo.boundaryVertNormals(boundary,verts);
        for (size_t bb=0; bb<boundary.size(); ++bb) {
            auto const &        be = boundary[bb];
            Vec3D               vert = verts[be.vertIdx] + vertNorms[bb] * scale;
            mesh.addMarkedVert(Vec3F(vert),"");
        }
    }
    viewMesh(mesh);
}

void
testm3d(CLArgs const & args)
{
    Cmds            cmds {
        {edgeDist,"edgeDist"},
        {fgSaveFgmeshTest,"fgmesh","FaceGen mesh file format export"},  // Uses GUI
        {testmSubdShapes,"subd0","Loop subdivsion of simple shapes"},
        {testmSubdFace,"subd1","Loop subdivision of textured face"},
        {testmSphere4,"sphere4","Spheres created from tetrahedon"},
        {testmSphere,"sphere","Spheres created from icosahedron"},
        {testmSurfTopo,"topo","Surface topology functions"},
    };
    doMenu(args,cmds,true,false,true);
}

}

// */
