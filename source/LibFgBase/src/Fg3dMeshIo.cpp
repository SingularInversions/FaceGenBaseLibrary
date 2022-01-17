//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
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

Svec<pair<MeshFormat,String>> getMeshFormatExtMap()
{
    return Svec<pair<MeshFormat,String>> {
        {MeshFormat::tri,"tri"},
        {MeshFormat::fgmesh,"fgmesh"},
        {MeshFormat::wobj,"obj"},       // both 'obj' and 'wobj' exts map to same format
        {MeshFormat::wobj,"wobj"},
        {MeshFormat::dae,"dae"},
        {MeshFormat::fbxA,"fbx"},       // first format with same ext is default for save
        {MeshFormat::fbxB,"fbx"},
        {MeshFormat::ma,"ma"},
        {MeshFormat::lwo,"lwo"},
        {MeshFormat::wrl,"wrl"},
        {MeshFormat::stl,"stl"},
        {MeshFormat::_3ds,"3ds"},
        {MeshFormat::xsi,"xsi"},
    };
}
MeshFormat          getMeshFormat(String const & ext)
{
    String          extl = toLower(ext);
    MeshFormats     formats = lookupRs(getMeshFormatExtMap(),extl);
    if (formats.empty())
        fgThrow("No mesh file format found for extension",extl);
    return formats[0];                  // preferred format listed first
}
bool                meshFormatSupportsMulti(MeshFormat mf)
{
    static map<MeshFormat,bool>   mfs = {
        {MeshFormat::tri,false},
        {MeshFormat::fgmesh,false},
        {MeshFormat::wobj,false},
        {MeshFormat::dae,true},
        {MeshFormat::fbxA,true},
        {MeshFormat::fbxB,true},
        {MeshFormat::ma,true},          // but who knows how JL implemented
        {MeshFormat::lwo,true},         // "
        {MeshFormat::wrl,true},
        {MeshFormat::stl,false},
        {MeshFormat::_3ds,false},
        {MeshFormat::xsi,true},         // "
    };
    auto            it = mfs.find(mf);
    FGASSERT(it != mfs.end());
    return it->second;
}
String              getMeshFormatExt(MeshFormat mf)
{
    Svec<pair<MeshFormat,String>> mfs = getMeshFormatExtMap();
    return lookupFirstL(mfs,mf);
}
String              getMeshFormatName(MeshFormat mf)
{
    static map<MeshFormat,String>   mfs = {
        {MeshFormat::tri,"FaceGen TRI"},
        {MeshFormat::fgmesh,"FaceGen mesh"},
        {MeshFormat::wobj,"Wavefront OBJ"},
        {MeshFormat::dae,"Collada"},
        {MeshFormat::fbxA,"Filmbox ASCII"},
        {MeshFormat::fbxB,"Filmbox binary"},
        {MeshFormat::ma,"Maya ASCII"},
        {MeshFormat::lwo,"Lightwave Object"},
        {MeshFormat::wrl,"VRML 97"},
        {MeshFormat::stl,"3D Systems STL Binary"},
        {MeshFormat::_3ds,"Autodesk 3DS"},
        {MeshFormat::xsi,"Softimage XSI"},
    };
    auto            it = mfs.find(mf);
    FGASSERT(it != mfs.end());
    return it->second;
}
MeshFormats         getMeshNativeFormats()
{
    return {
        MeshFormat::fgmesh,
        MeshFormat::tri,
    };
}
MeshFormats         getMeshExportFormats()
{
    return {
        MeshFormat::dae,
        MeshFormat::fbxA,
        MeshFormat::wobj,
        MeshFormat::wrl,
        MeshFormat::stl,
        MeshFormat::_3ds,
        MeshFormat::ma,
        MeshFormat::lwo,
        MeshFormat::xsi,
    };
}
String              getClOptionsString(MeshFormats const & formats)
{
    FGASSERT(!formats.empty());
    String              ret = "(" + getMeshFormatExt(formats[0]);
    for (size_t ii=1; ii<formats.size(); ++ii)
        ret += "," + getMeshFormatExt(formats[ii]);
    ret += ")";
    return ret;
}
String              getMeshLoadExtsCLDescription()
{
    return String("(fgmesh | [w]obj | tri)");
}
Svec<pair<String,String> > const & meshExportFormatExtDescs()
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
String              getMeshSaveExtsCLDescription()
{
    return "(fgmesh | tri | [w]obj | dae | wrl | fbx | stl | lwo | ma | xsi | 3ds | ply)";
}
Strings             meshExportFormatsWithMorphs()
{
    return {"dae","fbx","ma","lwo","xsi"};
}
Mesh                loadMesh(String8 const & fname)
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
Meshes              loadMeshes(String8 const & fname)
{
    String8         ext = pathToExt(fname).toLower();
    if (ext == "fgmesh")
        return {loadFgmesh(fname)};
    else if ((ext == "obj") || (ext == "wobj"))
        return {loadWObj(fname)};
    else if (ext == "fbx")
        return loadFbx(fname);
    if(ext != "tri")                // this structure avoids no-return warnings after 'fgThrow'
        fgThrow("Not a loadable 3D mesh format",fname);
    return {loadTri(fname)};
}
bool                loadMeshAnyFormat_(String8 const & fname,Mesh & mesh)
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
Mesh                loadMeshAnyFormat(String8 const & fname)
{
    Mesh                ret;
    if (!loadMeshAnyFormat_(fname,ret))
        fgThrow("No mesh format found for base name",fname);
    return ret;
}
Mesh                loadMeshMaps(String8 const & baseName)
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
void                saveMesh(Mesh const & mesh,String8 const & fname,String const & imgFmt)
{
    MeshFormat          fmt = getMeshFormat(pathToExt(fname).m_str);
    if (fmt == MeshFormat::tri)
        saveTri(fname,mesh);
    else if (fmt == MeshFormat::fgmesh)
        saveFgmesh(fname,mesh);
    else if (fmt == MeshFormat::wobj)
        saveWObj(fname,{mesh},imgFmt);
    else if (fmt == MeshFormat::dae)
        saveDae(fname,{mesh},imgFmt);
    else if (fmt == MeshFormat::fbxA)
        saveFbxAscii(fname,{mesh},imgFmt);
    else if (fmt == MeshFormat::fbxB)
        fgThrow("saveMesh: FBX binary not yet implemented");
    else if (fmt == MeshFormat::ma)
        saveMa(fname,{mesh},imgFmt);
    else if (fmt == MeshFormat::lwo)
        saveLwo(fname,{mesh},imgFmt);
    else if (fmt == MeshFormat::wrl)
        saveVrml(fname,{mesh},imgFmt);
    else if (fmt == MeshFormat::stl)
        saveStl(fname,{mesh});
    else if (fmt == MeshFormat::_3ds)
        save3ds(fname,{mesh},imgFmt);
    else if (fmt == MeshFormat::xsi)
        saveXsi(fname,{mesh},imgFmt);
    else
        fgThrow("saveMesh: unimplemented mesh format");
}
void                saveMergeMesh(Meshes const & meshes,String8 const & fname,String const & imgFmt)
{
    MeshFormat          fmt = getMeshFormat(pathToExt(fname).m_str);
    if (fmt == MeshFormat::tri)
        saveTri(fname,mergeMeshes(meshes));
    else if (fmt == MeshFormat::fgmesh)
        saveFgmesh(fname,mergeMeshes(meshes));
    else if (fmt == MeshFormat::wobj)
        saveWObj(fname,meshes,imgFmt);
    else if (fmt == MeshFormat::dae)
        saveDae(fname,meshes,imgFmt);
    else if (fmt == MeshFormat::fbxA)
        saveFbxAscii(fname,meshes,imgFmt);
    else if (fmt == MeshFormat::fbxB)
        fgThrow("saveMesh: FBX binary not yet implemented");
    else if (fmt == MeshFormat::ma)
        saveMa(fname,meshes,imgFmt);
    else if (fmt == MeshFormat::lwo)
        saveLwo(fname,meshes,imgFmt);
    else if (fmt == MeshFormat::wrl)
        saveVrml(fname,meshes,imgFmt);
    else if (fmt == MeshFormat::stl)
        saveStl(fname,meshes);
    else if (fmt == MeshFormat::_3ds)
        save3ds(fname,meshes,imgFmt);
    else if (fmt == MeshFormat::xsi)
        saveXsi(fname,meshes,imgFmt);
    else
        fgThrow("saveMergeMesh: unimplemented mesh format");
}
String              inMetresStr(SpatialUnit u)
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
String              toStr(SpatialUnit u)
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

}
