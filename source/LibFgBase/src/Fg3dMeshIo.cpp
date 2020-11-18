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

String
meshFormatExtension(MeshFormat mf)
{
    static map<MeshFormat,String>   mfs = {
        {MeshFormat::tri,"tri"},
        {MeshFormat::fgmesh,"fgmesh"},
        {MeshFormat::obj,"obj"},
        {MeshFormat::dae,"dae"},
        {MeshFormat::fbx,"fbx"},
        {MeshFormat::ma,"ma"},
        {MeshFormat::lwo,"lwo"},
        {MeshFormat::wrl,"wrl"},
        {MeshFormat::stl,"stl"},
        {MeshFormat::a3ds,"3ds"},
        {MeshFormat::xsi,"xsi"},
    };
    auto            it = mfs.find(mf);
    FGASSERT(it != mfs.end());
    return it->second;
}

String
meshFormatSpecifier(MeshFormat mf)
{
    static map<MeshFormat,String>   mfs = {
        {MeshFormat::tri,"FaceGen TRI"},
        {MeshFormat::fgmesh,"FaceGen mesh"},
        {MeshFormat::obj,"Wavefront OBJ"},
        {MeshFormat::dae,"Collada"},
        {MeshFormat::fbx,"Filmbox ASCII"},
        {MeshFormat::ma,"Maya ASCII"},
        {MeshFormat::lwo,"Lightwave Object"},
        {MeshFormat::wrl,"VRML 97"},
        {MeshFormat::stl,"3D Systems STL Binary"},
        {MeshFormat::a3ds,"Autodesk 3DS"},
        {MeshFormat::xsi,"Softimage XSI"},
    };
    auto            it = mfs.find(mf);
    FGASSERT(it != mfs.end());
    return it->second;
}

MeshFormats
meshExportFormats()
{
    return {
        MeshFormat::dae,
        MeshFormat::fbx,
        MeshFormat::obj,
        MeshFormat::wrl,
        MeshFormat::stl,
        MeshFormat::a3ds,
        MeshFormat::ma,
        MeshFormat::lwo,
        MeshFormat::xsi,
    };
}


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
        mesh = loadWObj(path.str(),"usemtl");       // Split by material to remain 1-1 with any color map arguments
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
saveMesh(Meshes const & meshes,Ustring const & fname,string const & imgFormat)
{
    Ustring    ext = pathToExt(fname).toLower();
    if(ext == "tri")
        saveTri(fname,meshes);
    else if (ext == "dae")
        saveDae(fname,meshes,imgFormat);
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

const Svec<pair<String,String> > &
meshExportFormatExtDescs()
{
    static Svec<pair<String,String> > ret = {
        {"dae","Collada"},
        {"fbx","Filmbox ASCII"},
        {"obj","Wavefront OBJ"},
        {"wrl","VRML 97"},
        {"stl","3DSystems STL"},
        {"3ds","3D Studio"},
        {"ma","Maya ASCII"},
        {"lwo","Lightwave Object"},
        {"xsi","Softimage"},
    };
    return ret;
}

Strings const &
meshExportFormatExts()
{
    static Strings ret = sliceMember(meshExportFormatExtDescs(),&pair<String,String>::first);
    return ret;
}

Strings const &
meshExportFormatDescriptions()
{
    static Strings ret = sliceMember(meshExportFormatExtDescs(),&pair<String,String>::second);
    return ret;
}

std::string
meshSaveFormatsCLDescription()
{return string("(tri | [w]obj | dae | wrl | fbx | stl | lwo | ma | xsi | 3ds | ply)"); }

Strings const &
meshExportFormatsWithMorphs()
{
    static Strings ret = svec<string>("dae","fbx","ma","lwo","xsi");
    return ret;
}

string
inMetresStr(SpatialUnit u)
{
    if (u == SpatialUnit::millimetre)
        return "0.001";
    else if (u == SpatialUnit::centimetre)
        return "0.01";
    else if (u == SpatialUnit::metre)
        return "1.0";
    else
        FGASSERT_FALSE;
    return "";
}

string
toStr(SpatialUnit u)
{
    if (u == SpatialUnit::millimetre)
        return "millimetre";
    else if (u == SpatialUnit::centimetre)
        return "centimetre";
    else if (u == SpatialUnit::metre)
        return "metre";
    else
        FGASSERT_FALSE;
    return "";
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
                loadImage_(imgFile,mesh.surfaces[cnt++].albedoMapRef());
            else
                syntax.error("More albedo map images specified than surfaces in",mesh.name);
        }
        meshes.push_back(mesh);
    }
    if (meshes.empty())
        syntax.error("No meshes specified");
    saveMesh(meshes,outFile);
}

Cmd
getTriExportCmd()
{return Cmd(triexport,"triexport","Export meshes from FaceGen TRI format to other formats"); }

}
