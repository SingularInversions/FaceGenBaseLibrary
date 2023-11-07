//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "Fg3dMeshIo.hpp"
#include "FgFileSystem.hpp"
#include "FgSerial.hpp"
#include "FgCommand.hpp"
#include "FgImageIo.hpp"
#include "FgScopeGuard.hpp"
#include "FgTestUtils.hpp"

using namespace std;

namespace Fg {

// TODO: abstracting the load/save functions gets complicated:
// * some formats support multi mesh vs single mesh
// * some formats write map filesnames (eg. WOBJ)
// * some formats have ASCII / binary variants (eg. FBX)
//typedef Sfun<Mesh(String8 const &)>                 LoadMeshFn;         // for formats that support only 1 mesh without map names
//typedef Sfun<void(Mesh const &,String8 const &)>    SaveMeshFn;         // "
//typedef Sfun<Meshes(String8 const &)>               LoadMeshesFn;       // for formats that support multiple meshes without map names
//typedef Sfun<void(Meshes const &,String8 const &)>  SaveMeshesFn;       // "

struct      MeshFormatInfo
{
    MeshFormat          format;
    Strings             exts;           // lower case. If more than 1, first is the most common
    String              description;    // descriptive name of format
    bool                multi;          // supports single or multi meshes per file ?

    bool                operator==(MeshFormat f) const {return (f == format); }
};
typedef Svec<MeshFormatInfo>    MeshFormatsInfo;

MeshFormatsInfo const & getMeshFormatsInfo()
{
    static MeshFormatsInfo  ret {
        {MeshFormat::fgmesh,{"fgmesh"},"FaceGen fgmesh",false},
        {MeshFormat::tri,{"tri"},"FaceGen TRI",false},
        {MeshFormat::wobj,{"obj","wobj"},"Wavefront OBJ",false},
        {MeshFormat::dae,{"dae"},"Collada",true},
        {MeshFormat::fbxA,{"fbx"},"Filmbox ASCII",true},
        {MeshFormat::fbxB,{"fbx"},"Filmbox binary",true},
        {MeshFormat::ma,{"ma"},"Maya ASCII",true},              // not sure if JL merged meshes or if supported by format
        {MeshFormat::lwo,{"lwo"},"Lightwave Object",true},      // "
        {MeshFormat::wrl,{"wrl"},"VRML 97",true},
        {MeshFormat::stl,{"stl"},"3D Systems STL binary",false},
        {MeshFormat::a3ds,{"3ds"},"Autodesk 3DS",false},
        {MeshFormat::xsi,{"xsi"},"Softimage XSI",true},         // "
        {MeshFormat::ply,{"ply"},"Polygon File Format",false},
    };
    return ret;
}

MeshFormat          getMeshFormat(String const & ext)
{
    String              extl = toLower(ext);
    for (MeshFormatInfo const & mfi : getMeshFormatsInfo()) {
        if (contains(mfi.exts,extl))
            return mfi.format;
    }
    fgThrow("No 3D mesh file format known for extension",extl);
    return MeshFormat::tri;
}
bool                meshFormatSupportsMulti(MeshFormat mf)
{
    return findFirst(getMeshFormatsInfo(),mf).multi;
}
String              getMeshFormatExt(MeshFormat mf)
{
    return findFirst(getMeshFormatsInfo(),mf).exts.at(0);
}
String              getMeshFormatName(MeshFormat mf)
{
    return findFirst(getMeshFormatsInfo(),mf).description;
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
        MeshFormat::a3ds,
        MeshFormat::ma,
        MeshFormat::lwo,
        MeshFormat::xsi,
        MeshFormat::ply,
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

Mesh                loadMeshMaps(String8 const & dirBase)
{
    Mesh            ret = loadMesh(dirBase);
    if (!ret.surfaces.empty()) {
        {
            Strings         exts = findExts(dirBase,getImgExts());
            if (!exts.empty())
                ret.surfaces[0].material.albedoMap = make_shared<ImgRgba8>(loadImage(dirBase+"."+exts[0]));
        }
        {
            String8         dbs = dirBase+"_Specular";
            Strings         exts = findExts(dbs,getImgExts());
            if (!exts.empty())
                ret.surfaces[0].material.specularMap = make_shared<ImgRgba8>(loadImage(dbs+"."+exts[0]));
        }
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
    else if (fmt == MeshFormat::a3ds)
        save3ds(fname,{mesh},imgFmt);
    else if (fmt == MeshFormat::xsi)
        saveXsi(fname,{mesh},imgFmt);
    else if (fmt == MeshFormat::ply)
        savePly(fname,{mesh},imgFmt);
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
    else if (fmt == MeshFormat::a3ds)
        save3ds(fname,meshes,imgFmt);
    else if (fmt == MeshFormat::xsi)
        saveXsi(fname,meshes,imgFmt);
    else if (fmt == MeshFormat::ply)
        savePly(fname,meshes,imgFmt);
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

Mesh                loadFgmesh(String8 const & fname)
{
    ScopeGuard          sg {[](){g_useSize64=true; }};      // ensure reset to normal
    g_useSize64 = false;                                    // this formats stores size_t as 32bit
    Bytes               ser = loadRaw(fname);
    if (ser.size() < 16)
        fgThrow("Too short to be a valid .fgmesh file",fname);
    size_t              pos {0};
    String              header = dsrlzT_<String>(ser,pos);
    if (cHead(header,6) != "FgMesh")
        fgThrow("Invalid header for .fgmesh file",fname);
    Mesh                ret;
    if (cRest(header,6) == "01") {
        try {
            dsrlz_(ser,pos,ret.verts);
            dsrlz_(ser,pos,ret.uvs);
            dsrlz_(ser,pos,ret.surfaces);
            dsrlz_(ser,pos,ret.deltaMorphs);
            dsrlz_(ser,pos,ret.targetMorphs);
            dsrlz_(ser,pos,ret.markedVerts);
            if (pos<ser.size())                 // ver 1.1
                dsrlz_(ser,pos,ret.joints);
        }
        catch (FgException & e) {e.contexts.emplace_back("invalid .fgmesh V01 file",fname.m_str); }
        catch (exception const & e) {fgThrow("invalid .fgmesh V01 file",fname.m_str,e.what()); }
    }
    else
        fgThrow("Unrecognized version of .fgmesh file, update to the latest version of this software",fname);
    return ret;
}

void                saveFgmesh(String8 const & fname,Mesh const & mesh)
{
    ScopeGuard          sg {[](){g_useSize64=true; }};      // ensure reset to normal
    g_useSize64 = false;                                    // this formats stores size_t as 32bit
    Bytes               ser;
    srlz_(String{"FgMesh01"},ser);
    srlz_(mesh,ser);
    saveRaw(ser,fname,false);
}

Mesh                getTestMeshV1()
{
    // Makes use of all fields and non-trivial use of cardinalities. All floats are pow2 friendly so we can use exactly equality tests:
    Mesh                ret;
    ret.verts = {                   // cube corners
        {-0.5,-0.5,-0.5},
        {-0.5,-0.5, 0.5},
        {-0.5, 0.5,-0.5},
        {-0.5, 0.5, 0.5},
        { 0.5,-0.5,-0.5},
        { 0.5,-0.5, 0.5},
        { 0.5, 0.5,-0.5},
        { 0.5, 0.5, 0.5},
    };
    ret.uvs = {                     // cross unwrap with +z face at centre and -Z face at bottom (scanline order):
        {0.25,1   },    // 0
        {0.5 ,1   },
        {0   ,0.75},    // 2
        {0.25,0.75},
        {0.5 ,0.75},
        {0.75,0.75},
        {0   ,0.5 },    // 6
        {0.25,0.5 },
        {0.5 ,0.5 },
        {0.75,0.5 },
        {0.25,0.25},    // 10
        {0.5 ,0.25},
        {0.25,0   },    // 12
        {0.5 ,0   },
    };
    Surf                s0,s1;
    s0.name = "Surf0";
    s1.name = "Surf1";
    s0.tris.vertInds = {        // +ve facing faces (RHR)
        {1,3,7}, {7,5,1},       // +X direction face
        {2,6,7}, {7,3,2},       // +Y
        {4,5,7}, {7,6,4},       // +Z
    };
    s0.tris.uvInds = {          // cross unwrap with +Z at centre and -Z at bottom
        {9,5,4}, {4,8,9},       // +X
        {0,3,4}, {4,1,0},       // +Y
        {7,8,4}, {4,3,7},       // +Z
    };
    s1.quads.vertInds = {
        {0,4,6,2},              // -X
        {0,1,5,4},              // -Y
        {0,2,3,1},              // -Z
    };
    s1.quads.uvInds = {
        {6,7,3,2},              // -X
        {10,11,8,7},            // -Y
        {10,12,13,11},          // -Z
    };
    s0.surfPoints = {
        {{0,{0.5,0,0.5}},"SP0"},    // centre of +X face
        {{4,{0.5,0,0.5}},"SP1"},    // centre of +Z face
    };
    s1.surfPoints = {
        {{0,{0.5,0,0.5}},"SP0"},    // centre of -X face
        {{2,{0.5,0,0.5}},"SP1"},    // centre of -Z face
    };
    ret.surfaces = {s0,s1};
    ret.deltaMorphs.emplace_back("deltaMorph0",ret.verts*0.25f);
    ret.deltaMorphs.emplace_back("deltaMorph1",ret.verts*-0.25f);
    Vec3F               del0 {0.25,0.125,0.0625},
                        del1 {0.125,0.0625,0.25};
    ret.targetMorphs.emplace_back("targMorph0",IdxVec3Fs{{1,del0},{3,del1}});
    ret.targetMorphs.emplace_back("targMorph1",IdxVec3Fs{{6,del1},{6,del0}});
    ret.markedVerts = {
        {0,"vert0"},
        {7,"vert7"},
    };
    return ret;
}

Mesh                getTestMeshV11()
{
    Mesh                ret = getTestMeshV1();
    ret.joints = {
        {"Centre",0,{0,0,0},{}},
        {"XpFace",1,{0.5,0,0},{{1,1},{3,1},{5,1},{7,1},}},
    };
    return ret;
}

void                testFgmesh(CLArgs const &)
{
    {       // V1: (only need to test load)
        Mesh                ref = getTestMeshV1(),
                            tst = loadFgmesh(dataDir()+"base/test/meshio-fgmesh-v1.fgmesh");
        FGASSERT(srlz(ref) == srlz(tst));
    }
    {       // V1.1 (current):
        TestDir             td {"base/meshio"};
        Mesh                ref = getTestMeshV1();
        saveFgmesh("v11.fgmesh",ref);
        Mesh                tst = loadFgmesh("v11.fgmesh");
        FGASSERT(srlz(tst)==srlz(ref));
    }
}

void testSave3ds(CLArgs const &);
void testSaveLwo(CLArgs const &);
void testSaveMa(CLArgs const &);
void testSaveFbx(CLArgs const &);
void testSaveDae(CLArgs const &);
void testSaveObj(CLArgs const &);
void testSavePly(CLArgs const &);
void testSaveXsi(CLArgs const &);
void testSaveVrml(CLArgs const &);

void                test3dMeshIo(CLArgs const & args)
{
    Cmds            cmds {
        {testSave3ds,   "3ds",  ".3DS file format export"},
        {testSaveLwo,   "lwo",  "Lightwve object file format export"},
        {testSaveMa,    "ma",   "Maya ASCII file format export"},
        {testSaveFbx,   "fbx",  ".FBX file format export"},
        {testFgmesh,    "fgm",  "FaceGen .fgmesh format"},
        {testSaveObj,   "obj",  "Wavefront OBJ ASCII file format export"},
        {testSavePly,   "ply",  ".PLY file format export"},
        {testSaveVrml,  "vrml", ".WRL file format export"},
        {testSaveDae,   "dae",  "Collada DAE format export"},
        {testSaveXsi,   "xsi",  ".XSI file format export"},
    };
    doMenu(args,cmds,true,false);
}

}
