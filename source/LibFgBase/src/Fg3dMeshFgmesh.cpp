//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Mesh format using little-endian int32, uint32, IEEE 754 single-precision and UTF-8 data
//

#include "stdafx.h"

#include "Fg3dMeshIo.hpp"
#include "FgSerial.hpp"
#include "FgFile.hpp"
#include "FgBounds.hpp"
#include "FgFileSystem.hpp"
#include "FgCommand.hpp"
#include "Fg3dDisplay.hpp"
#include "FgFile.hpp"

using namespace std;

namespace Fg {

template<uint dim>
void                readBin_(std::istream & is,NPolys<dim> & fi)
{
    readBin_(is,fi.vertInds);
    readBin_(is,fi.uvInds);
}
template<uint dim>
void                writeBin_(std::ostream & os,const NPolys<dim> & fi)
{
    writeBin_(os,fi.vertInds);
    writeBin_(os,fi.uvInds);
}
void                readBin_(std::istream & is,SurfPointName & sp)
{
    readBin_(is,sp.point.triEquivIdx);
    readBin_(is,sp.point.weights);
    readBin_(is,sp.label);
}
void                writeBin_(std::ostream & os,SurfPointName const & sp)
{
    writeBin_(os,sp.point.triEquivIdx);
    writeBin_(os,sp.point.weights);
    writeBin_(os,sp.label);
}
void                readBin_(std::istream & is,Surf & s)
{
    readBin_(is,s.name);
    readBin_(is,s.tris);
    readBin_(is,s.quads);
    readBin_(is,s.surfPoints);
}
void                writeBin_(std::ostream & os,Surf const & s)
{
    writeBin_(os,s.name);
    writeBin_(os,s.tris);
    writeBin_(os,s.quads);
    writeBin_(os,s.surfPoints);
}
void                readBin_(std::istream & is,MarkedVert & m)
{
    readBin_(is,m.idx);
    readBin_(is,m.label);
}
void                writeBin_(std::ostream & os,const MarkedVert & m)
{
    writeBin_(os,m.idx);
    writeBin_(os,m.label);
}
void                readBin_(std::istream & is,DirectMorph & m)
{
    readBin_(is,m.name);
    readBin_(is,m.verts);
}
void                writeBin_(std::ostream & os,DirectMorph const & m)
{
    writeBin_(os,m.name);
    writeBin_(os,m.verts);
}
void                readBin_(istream & is,IdxVec3F & iv)
{
    readBin_(is,iv.idx);
    readBin_(is,iv.vec);
}
void                writeBin_(ostream & os,IdxVec3F const & iv)
{
    writeBin_(os,iv.idx);
    writeBin_(os,iv.vec);
}
void                readBin_(std::istream & is,IndexedMorph & m)
{
    readBin_(is,m.name);
    readBin_(is,m.ivs);
}
void                writeBin_(std::ostream & os,const IndexedMorph & m)
{
    writeBin_(os,m.name);
    writeBin_(os,m.ivs);
}
void                readBin_(std::istream & is,Mesh & mesh)
{
    readBin_(is,mesh.verts);
    readBin_(is,mesh.uvs);
    readBin_(is,mesh.surfaces);
    readBin_(is,mesh.deltaMorphs);
    readBin_(is,mesh.targetMorphs);
    readBin_(is,mesh.markedVerts);
}
void                writeBin_(std::ostream & os,Mesh const & mesh)
{
    writeBin_(os,mesh.verts);
    writeBin_(os,mesh.uvs);
    writeBin_(os,mesh.surfaces);
    writeBin_(os,mesh.deltaMorphs);
    writeBin_(os,mesh.targetMorphs);
    writeBin_(os,mesh.markedVerts);
}

Mesh                loadFgmesh(String8 const & fname)
{
    Ifstream            ifs {fname};
    size_t              hdrStrSz = dsrlz<uint32>(ifs.readBytes(4));
    if (hdrStrSz != 8)
        fgThrow("Not a valid .FgMesh file",fname);
    String              header = ifs.readChars(8);
    if (cHead(header,6) != "FgMesh")
        fgThrow("Not a valid .FgMesh file",fname);
    if (cRest(header,6) != "01")
        fgThrow("This is a newer version of .FgMesh file, please update this software",fname);
    Mesh                ret;
    readBin_(ifs,ret);
    return ret;
}
void                saveFgmesh(String8 const & fname,Mesh const & mesh)
{
    Ofstream          ofs(fname);
    writeBin_(ofs,string("FgMesh01"));
    writeBin_(ofs,mesh);
}

//Mesh                loadFgmeshNew(String8 const & fname)
//{
//    Bytes               bytes = loadRaw(fname);
//    size_t              pos {0};
//    uint32              hdrStrSz = dsrlzT_<uint32>(bytes,pos);
//    if (hdrStrSz != 8)
//        fgThrow("Not a valid .FgMesh file",fname);
//    pos = 0;
//    String              header;
//    dsrlz32_(bytes,pos,header);
//    if (header != "FgMesh01")
//        fgThrow("Not a valid .FgMesh file",fname);
//    Mesh                ret;
//    dsrlz32_(bytes,pos,ret.verts);
//    //dsrlz_(bytes,pos,ret.uvs);
//    //dsrlz_(bytes,pos,ret.surfaces);
//    return ret;
//}

}
