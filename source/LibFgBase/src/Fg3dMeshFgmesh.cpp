//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
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
loadFgmesh(Ustring const & fname)
{
    Mesh        ret;
    Ifstream      ifs(fname);
    if (fgReadpT<string>(ifs) != "FgMesh01")
        fgThrow("Not a valid FGMESH file",fname);
    fgReadp(ifs,ret);
    return ret;
}

void
saveFgmesh(Ustring const & fname,Mesh const & mesh)
{
    Ofstream          ofs(fname);
    fgWritep(ofs,string("FgMesh01"));
    fgWritep(ofs,mesh);
}

void
saveFgmesh(Ustring const & fname,Meshes const & meshes)
{saveFgmesh(fname,mergeMeshes(meshes)); }

void
fgSaveFgmeshTest(CLArgs const & args)
{
    FGTESTDIR;
    saveFgmesh("Mouth.tri",loadTri(dataDir()+"base/Mouth.tri"));
    viewMesh(loadFgmesh("Mouth.tri"));
}

}
