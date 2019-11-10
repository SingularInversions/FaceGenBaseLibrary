//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
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

namespace Fg {

Mesh
loadFgmesh(const Ustring & fname)
{
    Mesh        ret;
    Ifstream      ifs(fname);
    if (fgReadpT<string>(ifs) != "FgMesh01")
        fgThrow("Not a valid FGMESH file",fname);
    fgReadp(ifs,ret);
    return ret;
}

void
saveFgmesh(const Ustring & fname,const Mesh & mesh)
{
    Ofstream          ofs(fname);
    fgWritep(ofs,string("FgMesh01"));
    fgWritep(ofs,mesh);
}

void
saveFgmesh(const Ustring & fname,const Meshes & meshes)
{saveFgmesh(fname,fgMergeMeshes(meshes)); }

void
fgSaveFgmeshTest(const CLArgs & args)
{
    FGTESTDIR;
    saveFgmesh("Mouth.tri",loadTri(dataDir()+"base/Mouth.tri"));
    meshView(loadFgmesh("Mouth.tri"));
}

}
