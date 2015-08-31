//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: Sept 22, 2011
//

#include "stdafx.h"
#include "Fg3dMeshIo.hpp"
#include "FgFileSystem.hpp"
#include "FgMetaFormat.hpp"
#include "FgException.hpp"
#include "Fg3dNormals.hpp"

using namespace std;

static
void
saveStl(FgOfstream & ff,const Fg3dMesh & mesh)
{
    Fg3dNormals     norms = fgNormals(mesh);
    for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
        const Fg3dSurface &         surf = mesh.surfaces[ss];
        const Fg3dFacetNormals &    facetNorms = norms.facet[ss];
        for (uint ii=0; ii<surf.numTris(); ++ii) {
            FgVect3UI           tri = surf.getTri(ii);
            FgVect3F            norm = facetNorms.tri[ii];
            for (uint jj=0; jj<3; ++jj)
                ff.writeb(norm[jj]);
            for (uint jj=0; jj<3; ++jj) {
                FgVect3F        vert = mesh.verts[tri[jj]];
                for (uint kk=0; kk<3; ++kk)
                    ff.writeb(vert[kk]);
            }
            ff.writeb(uint16(0));
        }
        for (uint ii=0; ii<surf.numQuads(); ++ii) {
            FgVect4UI           quad = surf.getQuad(ii);
            FgVect3F            norm = facetNorms.quad[ii];
            for (uint jj=0; jj<3; ++jj)
                ff.writeb(norm[jj]);
            for (uint jj=0; jj<3; ++jj) {
                FgVect3F        vert = mesh.verts[quad[jj]];
                for (uint kk=0; kk<3; ++kk)
                    ff.writeb(vert[kk]);
            }
            ff.writeb(uint16(0));
            for (uint jj=0; jj<3; ++jj)
                ff.writeb(norm[jj]);
            for (uint jj=2; jj<5; ++jj) {
                FgVect3F        vert = mesh.verts[quad[jj%4]];
                for (uint kk=0; kk<3; ++kk)
                    ff.writeb(vert[kk]);
            }
            ff.writeb(uint16(0));
        }
    }
}

void
fgSaveStl(const FgString & fname,const vector<Fg3dMesh> & meshes)
{
    FGASSERT(!meshes.empty());
    FgOfstream      ff(fname);
    ff.write("STL Binary file exported by FaceGen                                            ",80);
    uint32          numTris = 0;
    for (size_t ii=0; ii<meshes.size(); ++ii)
        numTris += meshes[ii].numTriEquivs();
    ff.writeb(numTris);
    for (size_t ii=0; ii<meshes.size(); ++ii)
        saveStl(ff,meshes[ii]);
}

bool
fgLoadMeshAnyFormat(
    const FgString &    fname,
    Fg3dMesh &          mesh)
{
    FgPath      path(fname);
    if (path.ext.empty()) {
        if (fgExists(fname+".tri"))
            path.ext = "tri";
        else if (fgExists(fname + ".wobj"))
            path.ext = "wobj";
        else if (fgExists(fname + ".obj"))
            path.ext = "obj";
        else
            return false;
    }
    FgString    ext = path.ext.toLower();
    if(ext == "tri")
        mesh = fgLoadTri(path.str());
    else if ((ext == "obj") || (ext == "wobj"))
        fgLoadWobj(path.str(),mesh);
    else
        fgThrow("Not a readable 3D mesh format",fname);
    return true;
}

Fg3dMesh
fgLoadMeshAnyFormat(const FgString & fname)
{
    Fg3dMesh    ret;
    if (!fgLoadMeshAnyFormat(fname,ret))
        fgThrow("No mesh format found for:",fname);
    return ret;
}

vector<string>
fgLoadMeshFormats()
{return fgSvec<string>("obj","wobj","tri"); }

string
fgLoadMeshFormatsDescription()
{return string("([w]obj | tri)"); }

void
fgSaveMeshAnyFormat(
    const Fg3dMesh &    mesh,
    const FgString &    fname)
{
    FgString    ext = fgPathToExt(fname).toLower();
    if(ext == "tri")
        fgSaveTri(fname,mesh);
    else if ((ext == "obj") || (ext == "wobj"))
        fgSaveObj(fname,mesh);
    else if (ext == "wrl")
        fgSaveVrml(fname,fgSvec(mesh));
    else if (ext == "fbx")
        fgSaveFbx(fname,mesh);
    else if (ext == "stl")
        fgSaveStl(fname,mesh);
    else
        fgThrow("Not a writeable 3D mesh format",fname);
}

std::string
fgSaveMeshFormatsDescription()
{return string("([w]obj | tri | wrl | fbx | stl)"); }

FgVerts
fgLoadVerts(const FgString & meshFilename)
{
    Fg3dMesh    mesh = fgLoadMeshAnyFormat(meshFilename);
    return mesh.allVerts();
}

//void
//fgSavePly(const FgString & fname,const Fg3dMesh & mesh)
//{
//    FgOfstream  os(fname);
//    os <<
//        "ply\n"
//        "format ascii 1.0\n"
//        "element vertex " << mesh.verts.size() << "\n"
//        "property float x\n"
//        "property float y\n"
//        "property float z\n"
//        "element face " << mesh.numTriEquivs() << "\n"
//        "
//        
//
//}

