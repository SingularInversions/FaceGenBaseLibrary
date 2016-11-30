//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Oct. 6, 2016
//
// Mesh format using little-endian int32, uint32, IEEE 754 single-precision and UTF-8 data
//

#include "stdafx.h"

#include "Fg3dMeshIo.hpp"
#include "FgException.hpp"
#include "FgStdStream.hpp"
#include "FgBounds.hpp"
#include "FgFileSystem.hpp"
#include "FgCommand.hpp"
#include "Fg3dDisplay.hpp"

using namespace std;

Fg3dMesh
fgLoadFgmesh(const FgString & fname)
{
    Fg3dMesh        ret;
    FgIfstream      ifs(fname);
    if (fgReadpT<string>(ifs) != "FgMesh01")
        fgThrow("Not a valid FGMESH file",fname);
    fgReadp(ifs,ret);
    return ret;
}

void
fgSaveFgmesh(const FgString & fname,const Fg3dMesh & mesh)
{
    FgOfstream          ofs(fname);
    fgWritep(ofs,string("FgMesh01"));
    fgWritep(ofs,mesh);
}

void
fgSaveFgmesh(const FgString & fname,const Fg3dMeshes & meshes)
{fgSaveFgmesh(fname,fgMergeMeshes(meshes)); }

void
fgSaveFgmeshTest(const FgArgs & args)
{
    FGTESTDIR;
    fgSaveFgmesh("Mouth.tri",fgLoadTri(fgDataDir()+"base/Mouth.tri"));
    fgDisplayMesh(fgLoadFgmesh("Mouth.tri"));
}
