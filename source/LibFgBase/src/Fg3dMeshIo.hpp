//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// NOTES:
//  * Most animation software works in metres while most 3D print software in mm.
//
// BLENDER:
//  * Imports and exports common formats with decent support but FBX only binary, not ASCII.
//  * Default units are metric/metres but large objects still import invisibly small.
//  * Imported objects are selected, press '.' on numeric keyboard to view selected object.
//  * Rotate around the object by holding down the middle mouse button.
//  * To view wireframe hit 'z' key then select view option on pop-up menu.
//  * When exporting you have to go through all the options tabs and deselect 'triangulate'
//  * Calls morphs 'Shape Keys' ('Relative' and 'Absolute')
//  * In the scene hierarchy (top right) click on a mesh node (upside down triangle)
//      * Then in 'Properties Editor' panel (bottom right)
//          * Ensure 'Object data' (upside-down triangle icon) tab selected
//          * Expand 'Shape Keys' to view defined morphs
//  * Even after deleting imported objects there is residual state that can cause errors on import
//    with shared IDs so restart Blender for proper import test.
//
// UNITY:
//  * Imports common formats but only exports FBX.
//  * Default units are metres.
//  * Right-click on 'Assets' in left tree view, select 'Import New Asset'
//  * In Assets->Textures (must create) 'Import New Asset' and select all texture maps
//  * Drag from 'Assets' window into 'Scene'
//  * Select an object by clicking on it or in 'Hierarchy' window
//  * Hit 'F' to zoom view of selected object
//  * Alt-Left-drag to rotate around object
//  * Drop-down menu on top left of Scene view can select Wireframe
//  * See object properties in 'Inspector' window on right & expand 'BlendShapes'
//
// ASSIMP:
//  * This is the clear FOSS favorite 3D formats lib
//  * Couldn't get it to build, nor could I get its CL tool to build, nor did I like the source code
//  * It has sample models of every format in in ~/test/models/<format>/
//
// DON'T BOTHER WITH:
//  * Meshlab: too feature-limited / buggy.
//  * MS 3D Viewer: Only exports to glb.
//

#ifndef FG3DMESHIO_HPP
#define FG3DMESHIO_HPP

#include "FgString.hpp"
#include "Fg3dMeshOps.hpp"

namespace Fg {

enum struct     MeshFormat
{
    tri,
    fgmesh,
    obj,
    dae,
    fbx,
    ma,
    lwo,
    wrl,
    stl,
    a3ds,
    xsi,
};
typedef Svec<MeshFormat>    MeshFormats;

String
meshFormatExtension(MeshFormat);

String
meshFormatSpecifier(MeshFormat);

MeshFormats
meshExportFormats();

// Returns false if 'fname' has no extension and no mesh file was found with a valid extension.
// Returns true otherwise. Throws an exception if the specified extension cannot be read as a mesh.
bool
loadMesh(
    Ustring const &     fname,  // If no extension specified will search readable mesh types.
    Mesh &              mesh);  // RETURNED

// As above but throws if no mesh found:
Mesh    loadMesh(Ustring const & fname);

// Loads both mesh and albedo map (if present) and specular map (if present):
Mesh    loadMeshMaps(Ustring const & baseName);

// Returns lower case list of supported extensions:
Strings
meshLoadFormats();

String
meshLoadFormatsCLDescription();

// Note that meshes and/or surfaces may be merged and other data may be lost
// depending on the format (see comments below per-format).
void
saveMesh(Meshes const & meshes,Ustring const & fname,String const & imgFormat="png");

inline
void
saveMesh(Mesh const & mesh,Ustring const & fname)
{saveMesh(svec(mesh),fname); }

// Does not include FaceGen formats:
Strings const &
meshExportFormatExts();

// 1-1 with above
Strings const &
meshExportFormatDescriptions();

Strings const &
meshExportFormatsWithMorphs();

// Includes 'tri':
std::string
meshSaveFormatsCLDescription();

// FaceGen mesh format load / save:

Mesh
loadFgmesh(Ustring const & fname);
void
saveFgmesh(Ustring const & fname,Mesh const & mesh);
void
saveFgmesh(Ustring const & fname,Meshes const & meshes);

// FaceGen legacy mesh format load / save:

Mesh        loadTri(std::istream & is);
Mesh        loadTri(Ustring const & fname);
Mesh        loadTri(Ustring const & meshFile,Ustring const & texFile);

// Merges all surfaces:
void
saveTri(Ustring const & fname,Mesh const & mesh);
// Merges all meshes and surfaces. Texture images not saved.
inline
void
saveTri(Ustring const & fname,Meshes const & meshes)
{return saveTri(fname,mergeMeshes(meshes)); }


enum struct     SpatialUnit { millimetre, centimetre, metre };
String          inMetresStr(SpatialUnit);
String          toStr(SpatialUnit);

// Third party mesh formats:

// Load from Wavefront OBJ file
Mesh
loadWObj(
    Ustring const &     filename,
    // Break up the surfaces by the given WOBJ separator. Valid values are 'usemtl', 'o' and 'g':
    String              surfSeparator=String());

// Save to Wavefront OBJ file. OBJ does not support morphs or units.
void
saveWObj(Ustring const & filename,Meshes const & meshes,String imgFormat = "png");

// Ignores morphs:
void
saveVrml(Ustring const & filename,Meshes const & meshes,String imgFormat = "png");

// Meshes with shared morphs must be merged as different surfaces into a single Mesh for the
// morphs to be unified in FBX (as imported into Unity). Unity will ignore albedo map file 
// references so they must be manually added:
void
saveFbx(Ustring const & filename,Meshes const & meshes,String imgFormat = "png");

// All meshes merged, ignores UVs, textures, morphs, etc:
void
saveStl(Ustring const & fname,Meshes const & meshes);

// Morph targets are also saved:
void
saveLwo(Ustring const & fname,Meshes const & meshes,String imgFormat = "png");

// Morph targets are also saved:
void
saveMa(Ustring const & fname,Meshes const & meshes,String imgFormat = "png");

// Morph targets are also saved:
void
saveXsi(Ustring const & fname,Meshes const & meshes,String imgFormat = "png");

// 3DS:
// * No morph targets
// * No quads
// * Splits UV seams
// * 8.3 tex names only
// * 2^16 max verts & tris
void
save3ds(Ustring const & fname,Meshes meshes,String imgFormat = "png");

// Vertices & surfaces must be merged to a single list but tex images are specified per facet.
// Currently saves all facets as tris but can easily be changed to preverve quads:
void
savePly(Ustring const & fname,Meshes const & meshes,String imgFormat = "png");

// Collada. Does not yet support morphs.
void
saveDae(Ustring const & fname,Meshes const & meshes,String imgFormat = "png",SpatialUnit unit=SpatialUnit::millimetre);

}

#endif
