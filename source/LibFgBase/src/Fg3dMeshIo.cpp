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
        else if (fgExists(fname + ".fgmesh"))
            path.ext = "fgmesh";
        else
            return false;
    }
    FgString    ext = path.ext.toLower();
    if(ext == "tri")
        mesh = fgLoadTri(path.str());
    else if ((ext == "obj") || (ext == "wobj"))
        mesh = fgLoadWobj(path.str());
    else if (ext == "fgmesh")
        mesh = fgLoadFgmesh(path.str());
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
{return fgSvec<string>("fgmesh","obj","wobj","tri"); }

string
fgLoadMeshFormatsDescription()
{return string("(fgmesh | [w]obj | tri)"); }

void
fgSaveMeshesAnyFormat(
    const vector<Fg3dMesh> &    meshes,
    const FgString &            fname,
    const string &              imgFormat)
{
    FgString    ext = fgPathToExt(fname).toLower();
    if(ext == "tri")
        fgSaveTri(fname,meshes);
    else if ((ext == "obj") || (ext == "wobj"))
        fgSaveObj(fname,meshes,imgFormat);
    else if (ext == "wrl")
        fgSaveVrml(fname,meshes,imgFormat);
    else if (ext == "fbx")
        fgSaveFbx(fname,meshes,imgFormat);
    else if (ext == "stl")
        fgSaveStl(fname,meshes);
    else if (ext == "lwo")
        fgSaveLwo(fname,meshes,imgFormat);
    else if (ext == "ma")
        fgSaveMa(fname,meshes,imgFormat);
    else if (ext == "xsi")
        fgSaveXsi(fname,meshes,imgFormat);
    else if (ext == "3ds")
        fgSave3ds(fname,meshes,imgFormat);
    else if (ext == "ply")
        fgSavePly(fname,meshes,imgFormat);
    else if (ext == "fgmesh")
        fgSaveFgmesh(fname,meshes);
    else
        fgThrow("Not a writeable 3D mesh format",fname);
}

const vector<string> &
fgMeshExportFormatsExts()
{
    static vector<string> ret = fgSvec<string>("obj","wrl","stl","3ds","fbx","ma","lwo","xsi");
    return ret;
}

const vector<string> &
fgMeshExportFormatsDescriptions()
{
    static vector<string> ret = fgSvec<string>(
        "Wavefront OBJ","VRML 97","STL","Autodesk 3DS","Filmbox ASCII","Maya ASCII","Lightwave Object","Softimage XSI");
    return ret;
}

std::string
fgMeshSaveFormatsString()
{return string("(tri | [w]obj | wrl | fbx | stl | lwo | ma | xsi | 3ds | ply)"); }

const vector<string> &
fgMeshExportFormatsWithMorphs()
{
    static vector<string> ret = fgSvec<string>("fbx","ma","lwo","xsi");
    return ret;
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
        "    <meshExt>      - " + fgMeshSaveFormatsString() + "\n"
        "    <imgExt>       - " + fgImgCommonFormatsDescription()
        );
    string              outFile(syntax.next());
    vector<Fg3dMesh>    meshes;
    while (syntax.more()) {
        string          triFile(syntax.next());
        if (!fgCheckExt(triFile,"tri"))
            syntax.error("Not a .TRI file",triFile);
        Fg3dMesh        mesh = fgLoadTri(triFile);
        size_t          cnt = 0;
        while (syntax.more() && fgToLower(fgPathToExt(syntax.peekNext())) != "tri") {
            string                      imgFile(syntax.next()),
                                        ext = fgPathToExt(imgFile);
            vector<string>              exts = fgImgCommonFormats();
            vector<string>::iterator    it = find(exts.begin(),exts.end(),ext);
            if (it == exts.end())
                syntax.error("Unknown image file type",imgFile);
            if (cnt < mesh.surfaces.size())
                fgLoadImgAnyFormat(imgFile,mesh.surfaces[cnt++].albedoMapRef());
            else
                syntax.error("More albedo map images specified than surfaces in",mesh.name);
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
