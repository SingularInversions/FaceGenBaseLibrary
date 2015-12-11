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
#include "FgSyntax.hpp"
#include "FgCommand.hpp"

using namespace std;

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
fgSaveMeshesAnyFormat(
    const vector<Fg3dMesh> &    meshes,
    const FgString &            fname)
{
    FgString    ext = fgPathToExt(fname).toLower();
    if(ext == "tri")
        fgSaveTri(fname,meshes);
    else if ((ext == "obj") || (ext == "wobj"))
        fgSaveObj(fname,meshes);
    else if (ext == "wrl")
        fgSaveVrml(fname,meshes);
    else if (ext == "fbx")
        fgSaveFbx(fname,meshes);
    else if (ext == "stl")
        fgSaveStl(fname,meshes);
    else if (ext == "lwo")
        fgSaveLwo(fname,meshes);
    else if (ext == "ma")
        fgSaveMa(fname,meshes);
    else if (ext == "xsi")
        fgSaveXsi(fname,meshes);
    else if (ext == "3ds")
        fgSave3ds(fname,meshes);
    else if (ext == "ply")
        fgSavePly(fname,meshes);
    else
        fgThrow("Not a writeable 3D mesh format",fname);
}

std::string
fgSaveMeshFormatsDescription()
{return string("(tri | [w]obj | wrl | fbx | stl | lwo | ma | xsi | 3ds | ply)"); }

FgVerts
fgLoadVerts(const FgString & meshFilename)
{
    Fg3dMesh    mesh = fgLoadMeshAnyFormat(meshFilename);
    return mesh.allVerts();
}

/**
   \ingroup Base_Commands
   Command to Export meshes from FaceGen TRI format to other formats
 */
static
void
triexport(const FgArgs & args)
{
    FgSyntax    syntax(args,
        "<out>.<meshExt> (<mesh>.tri [<texImage>.<imgExt>])+\n"
        "    <meshExt>      - " + fgSaveMeshFormatsDescription() + "\n"
        "    <imgExt>       - " + fgImgCommonFormatsDescription()
        );
    string              outFile(syntax.next());
    vector<Fg3dMesh>    meshes;
    while (syntax.more()) {
        string          triFile(syntax.next());
        if (!fgCheckSetExtension(triFile,"tri"))
            syntax.error("Invalid .TRI file extension: ",triFile);
        Fg3dMesh        mesh = fgLoadTri(triFile);
        while (syntax.more() && fgToLower(fgPathToExt(syntax.peekNext())) != "tri") {
            string                      imgFile(syntax.next()),
                                        ext = fgPathToExt(imgFile);
            vector<string>              exts = fgImgCommonFormats();
            vector<string>::iterator    it = find(exts.begin(),exts.end(),ext);
            if (it == exts.end())
                syntax.error("Unknown image file type",imgFile);
            FgImgRgbaUb     img;
            fgLoadImgAnyFormat(imgFile,img);
            mesh.texImages.push_back(img);
        }
        meshes.push_back(mesh);
    }
    if (meshes.empty())
        syntax.error("No meshes specified");
    fgSaveMeshesAnyFormat(meshes,outFile);
}

FgCmd
fgCmdTriexportInfo()
{return FgCmd(triexport,"triexport","Export meshes from FaceGen TRI format to other formats"); }
