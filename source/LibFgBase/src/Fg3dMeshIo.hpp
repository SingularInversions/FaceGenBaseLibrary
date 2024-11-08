//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FG3DMESHIO_HPP
#define FG3DMESHIO_HPP

#include "FgSerial.hpp"
#include "Fg3dMesh.hpp"

namespace Fg {

enum struct     MeshFormat
{
    tri,        // FaceGen
    fgmesh,     // FaceGen
    wobj,       // Wavefront OBJ
    dae,        // Collada
    fbxA,       // Filmbox ASCII version
    fbxB,       // Filmbox binary version
    ma,         // Maya ASCII
    lwo,        // Lightwave object
    wrl,        // VRML
    stl,        // 3D Systems stereolithography
    a3ds,       // Autodesk 3D Studio
    xsi,        // Softimage ASCII
    ply,        // Polygon File Format
};
typedef Svec<MeshFormat>    MeshFormats;

MeshFormat          getMeshFormat(String8 const & file);    // throws a descriptive error if not a known mesh format
bool                meshFormatSupportsMulti(MeshFormat);    // true if format supports multiple vertex lists
String              getMeshFormatExt(MeshFormat);
String              getMeshFormatName(MeshFormat);
MeshFormats         getMeshNativeFormats();                 // FaceGen formats (fgmesh,tri) are fully supported
MeshFormats         getMeshExportFormats();                 // save formats not including FaceGen formats
String              getClOptionsString(MeshFormats const &);
String              getMeshLoadExtsCLDescription();
Strings             meshExportFormatsWithMorphs();
// Includes 'tri':
std::string         getMeshSaveExtsCLDescription();

Mesh                loadMesh(String8 const & fname);
// use to load meshes properly from multi-mesh formats (single mesh formats will return a list of only 1 mesh):
Meshes              loadMeshes(String8 const & fname);              // includes multi-mesh formats
// If a loadable mesh extension for the given base name can be found the mesh will be loaded and
// the function will return true. False otherwise:
bool                loadMeshAnyFormat_(String8 const & baseName,Mesh & returned);
Mesh                loadMeshAnyFormat(String8 const & fname);       // As above but throws if no mesh found
// Loads both mesh and albedo map (if present) and specular map (if present):
Mesh                loadMeshMaps(String8 const & baseName);
// Saves mesh and loaded images to format given by extension:
void                saveMesh(Mesh const & mesh,String8 const & fname,String const & imgFormat="png");
// saves mesh AND images:
// Note that meshes and/or surfaces may be merged and other data may be lost
// depending on the format (see comments below per-format).
void                saveMergeMesh(Meshes const & meshes,String8 const & fname,String const & imgFormat="png");

// FaceGen mesh format load / save:
Mesh                loadFgmesh(String8 const & fname);
void                saveFgmesh(String8 const & fname,Mesh const & mesh);
// FaceGen legacy mesh format load / save:
Mesh                loadTri(std::istream & is);
Mesh                loadTri(String8 const & fname);
Mesh                loadTri(String8 const & meshFile,String8 const & texFile);
void                saveTri(String8 const & fname,Mesh const & mesh);       // merges all surfaces

enum struct     SpatialUnit { millimetre, centimetre, metre };
String              inMetresStr(SpatialUnit);
String              toStr(SpatialUnit);

// Third party mesh formats:

// Load from Wavefront OBJ file. Only supports polygonal meshes (no smooth curves):
Mesh                loadWObj(String8 const & filename);
// Save to Wavefront OBJ file. OBJ does not support morphs or units and multiple vertex lists are not
// properly supported, even though multiple meshes can be passed as an argument:
void                saveWObj(String8 const & filename,Meshes const & meshes,String imgFormat = "png");
void                injectVertsWObj(String8 const & inName,Vec3Fs const & verts,String8 const & outName);
// Ignores morphs:
void                saveVrml(String8 const & filename,Meshes const & meshes,String imgFormat = "png");
Meshes              loadFbx(String8 const & filename);      // FBX is a multi-mesh format
// Meshes with shared morphs must be merged as different surfaces into a single Mesh for the
// morphs to be unified in FBX (as imported into Unity). Unity will ignore albedo map file 
// references so they must be manually added:
void                saveFbxAscii(String8 const & filename,Meshes const & meshes,String imgFormat = "png");
// Inject updated vertex positions into an existing FBX binary with an identical vertex lists:
void                injectVertsFbxBin(String8 const & inFile,Vec3Fs const & verts,String8 const & outFile);
// All surfaces are merged during write, ignores UVs, textures, morphs, etc.:
void                saveStl(String8 const & fname,Meshes const & meshes);
// morph targets are also saved:
void                saveLwo(String8 const & fname,Meshes const & meshes,String imgFormat = "png");
// morph targets are also saved:
void                saveMa(String8 const & fname,Meshes const & meshes,String imgFormat = "png");
// morph targets are also saved:
void                saveXsi(String8 const & fname,Meshes const & meshes,String imgFormat = "png");
// 3DS:
// * No morph targets
// * No quads
// * Splits UV seams
// * 8.3 tex names only
// * 2^16 max verts & tris
void                save3ds(String8 const & fname,Meshes meshes,String imgFormat = "png");
// Vertices & surfaces must be merged to a single list but tex images are specified per facet.
// Currently saves all facets as tris but can easily be changed to preverve quads:
void                savePly(String8 const & fname,Meshes const & meshes,String imgFormat = "png");
// Collada. Does not yet support morphs.
void                saveDae(
    String8 const &     fname,
    Meshes const &      meshes,
    String              imgFormat = "png",
    SpatialUnit         unit=SpatialUnit::millimetre);

}

#endif
