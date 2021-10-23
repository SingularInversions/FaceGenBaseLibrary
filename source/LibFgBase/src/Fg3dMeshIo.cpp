//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
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

Mesh
loadMesh(String8 const & fname)
{
    String8         ext = pathToExt(fname).toLower();
    if (ext == "fgmesh")
        return loadFgmesh(fname);
    else if ((ext == "obj") || (ext == "wobj"))
        return loadWObj(fname);
    if(ext != "tri")                // this structure avoids no-return warnings after 'fgThrow'
        fgThrow("Not a loadable 3D mesh format",fname);
    return loadTri(fname);
}

bool
loadMeshAnyFormat_(String8 const & fname,Mesh & mesh)
{
    Path            path(fname);
    if (path.ext.empty()) {
        if (pathExists(fname+".fgmesh"))
            mesh = loadFgmesh(fname+".fgmesh");
        else if (pathExists(fname+".tri"))
            mesh = loadTri(fname+".tri");
        else if (pathExists(fname+".wobj"))
            mesh = loadWObj(fname+".wobj");
        else if (pathExists(fname+".obj"))
            mesh = loadWObj(fname+".obj");
        else
            return false;
    }
    return true;
}

Mesh
loadMeshAnyFormat(String8 const & fname)
{
    Mesh                ret;
    if (!loadMeshAnyFormat_(fname,ret))
        fgThrow("No mesh format found for base name",fname);
    return ret;
}

Mesh
loadMeshMaps(String8 const & baseName)
{
    Mesh            ret = loadMesh(baseName);
    if (!ret.surfaces.empty()) {
        Strings         albExts = getImageFiles(baseName);
        if (!albExts.empty())
            ret.surfaces[0].material.albedoMap = make_shared<ImgRgba8>(loadImage(baseName+"."+albExts[0]));
        String8         specBase = baseName+"_Specular";
        Strings         specExts = getImageFiles(specBase);
        if (!specExts.empty())
            ret.surfaces[0].material.specularMap = make_shared<ImgRgba8>(loadImage(specBase+"."+specExts[0]));
    }
    return ret;
}

Strings
meshLoadFormats()
{return svec<string>("fgmesh","tri","obj","wobj"); }

bool
hasMeshExtension(String8 const & filename)
{
    return contains(meshLoadFormats(),toLower(pathToExt(filename).m_str));
}

string
getMeshLoadExtsCLDescription()
{return string("(fgmesh | [w]obj | tri)"); }

void
saveMesh(Meshes const & meshes,String8 const & fname,string const & imgFormat)
{
    String8    ext = pathToExt(fname).toLower();
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
getMeshSaveExtsCLDescription()
{return string("(fgmesh | tri | [w]obj | dae | wrl | fbx | stl | lwo | ma | xsi | 3ds | ply)"); }

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
cmdExport(CLArgs const & args)
{
    Syntax    syntax(args,
        "<out>.<meshExt> (<mesh>.tri [<texImage>.<imgExt>])+\n"
        "    <meshExt>      - " + getMeshSaveExtsCLDescription() + "\n"
        "    <imgExt>       - " + getImageFileExtCLDescriptions()
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
            Strings             exts = getImageFileExts();
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
cmdExportInfo()
{return Cmd(cmdExport,"export","Export FaceGen meshes and related color maps to other formats"); }

}
