//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
// Created: Sept 22, 2011
//

#ifndef FG3DMESHIO_HPP
#define FG3DMESHIO_HPP

#include "FgString.hpp"
#include "Fg3dMeshOps.hpp"

// Returns false if 'fname' has no extension and no mesh file was found with a valid extension.
// Returns true otherwise. Throws an exception if the specified extension cannot be read as a mesh.
bool
fgLoadMeshAnyFormat(
    const FgString &    fname,  // If no extension specified will search readable mesh types.
    Fg3dMesh &          mesh);  // RETURNED

Fg3dMesh
fgLoadMeshAnyFormat(const FgString & fname);

// Returns lower case list of supported extensions:
vector<string>
fgLoadMeshFormats();

string
fgLoadMeshFormatsDescription();

// Note that meshes and/or surfaces may be merged and other data may be lost
// depending on the format (see comments below per-format):
void
fgSaveMeshesAnyFormat(const vector<Fg3dMesh> & meshes,const FgString & fname);

inline
void
fgSaveMeshAnyFormat(const Fg3dMesh & mesh,const FgString & fname)
{fgSaveMeshesAnyFormat(fgSvec(mesh),fname); }

std::string
fgSaveMeshFormatsDescription();

FgVerts
fgLoadVerts(const FgString & meshFilename);

Fg3dMesh
fgLoadTri(std::istream & is);

Fg3dMesh
fgLoadTri(const FgString & fname);

Fg3dMesh
fgLoadTri(const FgString & meshFile,const FgString & texFile);

// Merges all surfaces:
void
fgSaveTri(
    const FgString &    fname,
    const Fg3dMesh &    mesh);

// Merges all meshes and surfaces:
inline
void
fgSaveTri(const FgString & fname,const vector<Fg3dMesh> & meshes)
{return fgSaveTri(fname,fgMergeMeshes(meshes)); }

void
fgLoadWobj(
    const FgString &    filename,
    Fg3dMesh &          mesh,
    // Break up the surfaces by the given WOBJ separator. Valid values are 'usemtl', 'o' and 'g':
    string              surfSeparator=string());

// Ignores morphs:
void
fgSaveObj(
    const FgString &            filename,
    const vector<Fg3dMesh> &    meshes,
    string                      imgFormat = "png");

void
fgSaveVrml(
    const FgString &            filename,
    const vector<Fg3dMesh> &    meshes,
    string                      imgFormat = "png");

// Texture image not currently supported as Unity will not automatically load anyway:
void
fgSaveFbx(
    const FgString &            filename,
    const vector<Fg3dMesh> &    meshes,
    string                      imgFormat = "png");

// All meshes merged, ignores UVs, textures, morphs, etc:
void
fgSaveStl(
    const FgString &            fname,
    const vector<Fg3dMesh> &    meshes);

// Morph targets are also saved:
void
fgSaveLwo(
    const FgString &        fname,
    const vector<Fg3dMesh> & meshes,
    string                  imgFormat = "png");

// Morph targets are also saved:
void
fgSaveMa(
    const FgString &        fname,
    const vector<Fg3dMesh> & meshes,
    string                  imgFormat = "png");

// Morph targets are also saved:
void
fgSaveXsi(
    const FgString &        fname,
    const vector<Fg3dMesh> & meshes,
    string                  imgFormat = "png");

// 3DS:
// * No morph targets
// * No quads
// * Splits UV seams
// * 8.3 tex names only
// * 2^16 max verts & tris
void
fgSave3ds(
    const FgString &        fname,
    vector<Fg3dMesh>        meshes,
    string                  imgFormat = "png");

// Vertices & surfaces must be merged to a single list but tex images are specified per facet.
// Currently saves all facets as tris but can easily be changed to preverve quads:
void
fgSavePly(
    const FgString &        fname,
    const vector<Fg3dMesh> & meshes,
    string                  imgFormat = "png");

#endif
