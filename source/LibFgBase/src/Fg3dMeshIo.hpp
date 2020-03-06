//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FG3DMESHIO_HPP
#define FG3DMESHIO_HPP

#include "FgString.hpp"
#include "Fg3dMeshOps.hpp"

namespace Fg {

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
meshSaveAnyFormat(Meshes const & meshes,Ustring const & fname,String const & imgFormat="png");

inline
void
meshSaveAnyFormat(Mesh const & mesh,Ustring const & fname)
{meshSaveAnyFormat(svec(mesh),fname); }

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

// Third party mesh formats:

// Load from Wavefront OBJ file
Mesh
loadWObj(
    Ustring const &     filename,
    // Break up the surfaces by the given WOBJ separator. Valid values are 'usemtl', 'o' and 'g':
    String              surfSeparator=String());

// Save to Wavefront OBJ file (ignores morphs)
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
saveDae(Ustring const & fname,Meshes const & meshes,String imgFormat = "png");

}

#endif
