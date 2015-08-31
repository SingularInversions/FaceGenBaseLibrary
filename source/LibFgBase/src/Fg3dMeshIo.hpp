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
#include "Fg3dMesh.hpp"

// Load / save FaceGen TRI format:

Fg3dMesh
fgLoadTri(std::istream & is);

Fg3dMesh
fgLoadTri(const FgString & fname);

Fg3dMesh
fgLoadTri(const FgString & meshFile,const FgString & texFile);

void
fgSaveTri(
    const FgString &    fname,
    const Fg3dMesh &    mesh);      // Merges all surfaces

// Load / save Wavefront OBJ format:

void
fgLoadWobj(
    const FgString &    filename,
    Fg3dMesh &          mesh,
    // Break up the surfaces by the given WOBJ separator. Valid values are 'usemtl', 'o' and 'g':
    string              surfSeparator=string());

// Ignores morphs:
void
fgSaveObj(
    const FgString &    filename,
    const std::vector<Fg3dMesh> &   meshes);

inline void
fgSaveObj(const FgString & filename,const Fg3dMesh & mesh)
{fgSaveObj(filename,fgSvec(mesh)); }

void
fgSaveVrml(
    const FgString &                filename,
    const std::vector<Fg3dMesh> &   meshes);

// Texture image not currently supported as Unity will not automatically load anyway:
void
fgSaveFbx(
    const FgString &            filename,
    const vector<Fg3dMesh> &    meshes);

inline void
fgSaveFbx(const FgString & filename,const Fg3dMesh & mesh)
{fgSaveFbx(filename,fgSvec(mesh)); }

// All meshes merged, ignores UVs, textures, morphs, etc:
void
fgSaveStl(const FgString & fname,const vector<Fg3dMesh> & meshes);

inline void
fgSaveStl(const FgString & fname,const Fg3dMesh & mesh)
{fgSaveStl(fname,fgSvec(mesh)); }

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

void
fgSaveMeshAnyFormat(
    const Fg3dMesh &    mesh,
    const FgString &    fname);

std::string
fgSaveMeshFormatsDescription();

FgVerts
fgLoadVerts(const FgString & meshFilename);

//void
//fgSavePly(const FgString & fname,const Fg3dMesh & mesh);

#endif
