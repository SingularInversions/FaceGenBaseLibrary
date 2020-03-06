//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"
#include "Fg3dMeshIo.hpp"
#include "FgFileSystem.hpp"
#include "FgMetaFormat.hpp"
#include "FgException.hpp"
#include "FgSyntax.hpp"
#include "FgCommand.hpp"
#include "FgImageIo.hpp"

using namespace std;

namespace Fg {

bool
loadMesh(
    Ustring const &     fname,
    Mesh &              mesh)
{
    Path      path(fname);
    if (path.ext.empty()) {
        if (pathExists(fname+".tri"))
            path.ext = "tri";
        else if (pathExists(fname + ".wobj"))
            path.ext = "wobj";
        else if (pathExists(fname + ".obj"))
            path.ext = "obj";
        else if (pathExists(fname + ".fgmesh"))
            path.ext = "fgmesh";
        else
            return false;
    }
    Ustring    ext = path.ext.toLower();
    if(ext == "tri")
        mesh = loadTri(path.str());
    else if ((ext == "obj") || (ext == "wobj"))
        mesh = loadWObj(path.str());
    else if (ext == "fgmesh")
        mesh = loadFgmesh(path.str());
    else
        fgThrow("Not a readable 3D mesh format",fname);
    return true;
}

Mesh
loadMesh(Ustring const & fname)
{
    Mesh    ret;
    if (!loadMesh(fname,ret))
        fgThrow("No mesh format found for:",fname);
    return ret;
}

Mesh
loadMeshMaps(Ustring const & baseName)
{
    Mesh            ret = loadMesh(baseName);
    if (!ret.surfaces.empty()) {
        Strings         albExts = imgFindFiles(baseName);
        if (!albExts.empty())
            ret.surfaces[0].material.albedoMap = make_shared<ImgC4UC>(loadImage(baseName+"."+albExts[0]));
        Ustring         specBase = baseName+"_Specular";
        Strings         specExts = imgFindFiles(specBase);
        if (!specExts.empty())
            ret.surfaces[0].material.specularMap = make_shared<ImgC4UC>(loadImage(specBase+"."+specExts[0]));
    }
    return ret;
}

Strings
meshLoadFormats()
{return svec<string>("fgmesh","obj","wobj","tri"); }

string
meshLoadFormatsCLDescription()
{return string("(fgmesh | [w]obj | tri)"); }

void
meshSaveAnyFormat(
    Meshes const &    meshes,
    Ustring const &            fname,
    string const &              imgFormat)
{
    Ustring    ext = pathToExt(fname).toLower();
    if(ext == "tri")
        saveTri(fname,meshes);
    else if ((ext == "obj") || (ext == "wobj"))
        saveWObj(fname,meshes,imgFormat);
    else if (ext == "wrl")
        saveVrml(fname,meshes,imgFormat);
    else if (ext == "fbx")
        saveFbx(fname,meshes,imgFormat);
    else if (ext == "stl")
        saveStl(fname,meshes);
    else if (ext == "lwo")
        saveLwo(fname,meshes,imgFormat);
    else if (ext == "ma")
        saveMa(fname,meshes,imgFormat);
    else if (ext == "xsi")
        saveXsi(fname,meshes,imgFormat);
    else if (ext == "3ds")
        save3ds(fname,meshes,imgFormat);
    else if (ext == "ply")
        savePly(fname,meshes,imgFormat);
    else if (ext == "fgmesh")
        saveFgmesh(fname,meshes);
    else
        fgThrow("Not a writeable 3D mesh format",fname);
}

Strings const &
meshExportFormatExts()
{
    static Strings ret = svec<string>("obj","wrl","stl","3ds","fbx","ma","lwo","xsi");
    return ret;
}

Strings const &
meshExportFormatDescriptions()
{
    static Strings ret = svec<string>(
        "Wavefront OBJ","VRML 97","STL","Autodesk 3DS","Filmbox ASCII","Maya ASCII","Lightwave Object","Softimage XSI");
    return ret;
}

std::string
meshSaveFormatsCLDescription()
{return string("(tri | [w]obj | wrl | fbx | stl | lwo | ma | xsi | 3ds | ply)"); }

Strings const &
meshExportFormatsWithMorphs()
{
    static Strings ret = svec<string>("fbx","ma","lwo","xsi");
    return ret;
}

/**
   \ingroup Base_Commands
   Command to Export meshes from FaceGen TRI format to other formats
 */
static
void
triexport(CLArgs const & args)
{
    Syntax    syntax(args,
        "<out>.<meshExt> (<mesh>.tri [<texImage>.<imgExt>])+\n"
        "    <meshExt>      - " + meshSaveFormatsCLDescription() + "\n"
        "    <imgExt>       - " + imgFileExtensionsDescription()
        );
    string              outFile(syntax.next());
    Meshes    meshes;
    while (syntax.more()) {
        string          triFile(syntax.next());
        if (!checkExt(triFile,"tri"))
            syntax.error("Not a .TRI file",triFile);
        Mesh        mesh = loadTri(triFile);
        size_t          cnt = 0;
        while (syntax.more() && toLower(pathToExt(syntax.peekNext())) != "tri") {
            string              imgFile(syntax.next()),
                                ext = pathToExt(imgFile);
            Strings             exts = imgFileExtensions();
            auto                it = find(exts.begin(),exts.end(),ext);
            if (it == exts.end())
                syntax.error("Unknown image file type",imgFile);
            if (cnt < mesh.surfaces.size())
                loadImage(imgFile,mesh.surfaces[cnt++].albedoMapRef());
            else
                syntax.error("More albedo map images specified than surfaces in",mesh.name);
        }
        meshes.push_back(mesh);
    }
    if (meshes.empty())
        syntax.error("No meshes specified");
    meshSaveAnyFormat(meshes,outFile);
}

Cmd
getTriExportCmd()
{return Cmd(triexport,"triexport","Export meshes from FaceGen TRI format to other formats"); }

}
