//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "Fg3dMesh.hpp"
#include "FgSerial.hpp"
#include "FgFile.hpp"
#include "FgBounds.hpp"
#include "FgFileSystem.hpp"
#include "FgImageIo.hpp"

using namespace std;

namespace Fg {

static string triIdent = "FRTRI003";

static
string
readString(istream & istr,bool wchar)
{
    uint        size;
    readBinRaw_(istr,size);
    string      str;
    if (size == 0)
        return str;
    str.resize(size);
    for (uint ii=0; ii<size; ++ii) {
        if (wchar) {
            wchar_t     wch;
            readBinRaw_(istr,wch);
            str[ii] = char(wch);
        }
        else
            istr.read(&str[ii],1);
    }
    // Get rid of NULL terminating character required by spec:
    str.resize(size-1);
    return str;
}

Mesh
loadTri(istream & istr)
{
    Mesh            mesh;
    // Check for file type identifier
    char            cdata[9];
    istr.read(cdata,8);
    if (strncmp(cdata,triIdent.data(),8) != 0)           // 0 indicates no difference
        fgThrow("File not in TRI format");
        // Read in the header
    uint32      numVerts,
                numTris,
                numQuads,
                numLabVerts,
                numSurfPts,
                numUvs,
                texExt,
                numDiffMorph,
                numStatMorph,
                numStatMorphVerts;
    char        buff[16];
    readBinRaw_(istr,numVerts);
    readBinRaw_(istr,numTris);
    readBinRaw_(istr,numQuads);
    readBinRaw_(istr,numLabVerts);
    readBinRaw_(istr,numSurfPts);
    readBinRaw_(istr,numUvs);
    readBinRaw_(istr,texExt);
    readBinRaw_(istr,numDiffMorph);
    readBinRaw_(istr,numStatMorph);
    readBinRaw_(istr,numStatMorphVerts);
    istr.read(buff,16);
    bool    texs = ((texExt & 0x01) != 0),
            wchar = ((texExt & 0x02) != 0);
    if (wchar)
        fgout << fgnl << "WARNING: Unicode labels being converted to ASCII.";

    // Read in the verts:
    if (numVerts==0)
        fgThrow("TRI file has no vertices");
    mesh.verts.resize(numVerts);
    Vec3Fs    targVerts(numStatMorphVerts);
    istr.read(reinterpret_cast<char*>(&mesh.verts[0]),int(12*numVerts));
    if (numStatMorphVerts > 0)
        istr.read(reinterpret_cast<char*>(&targVerts[0]),int(12*numStatMorphVerts));

    // Read in the surface if there is any surface data (a TRI has only one):
    bool            hasSurface =  ((numTris > 0) || (numQuads > 0));
    if (hasSurface)
        mesh.surfaces.resize(1);
    Surf            dummy;
    Surf &          surf = hasSurface ? mesh.surfaces[0] : dummy;
    surf.tris.vertInds.resize(numTris);
    if (numTris > 0)
        istr.read(reinterpret_cast<char*>(&surf.tris.vertInds[0]),int(12*numTris));
    surf.quads.vertInds.resize(numQuads);
    if (numQuads > 0)
        istr.read(reinterpret_cast<char*>(&surf.quads.vertInds[0]),int(16*numQuads));
    // Marked verts:
    mesh.markedVerts.resize(numLabVerts);
    for (uint jj=0; jj<numLabVerts; jj++) {
        istr.read(reinterpret_cast<char*>(&mesh.markedVerts[jj].idx),4);
        mesh.markedVerts[jj].label = readString(istr,wchar);
    }
    // Surface points:
    for (uint ii=0; ii<numSurfPts; ii++) {
        SurfPointName     sp;
        sp.point.triEquivIdx = readBinRaw<uint32>(istr);
        readBinRaw_(istr,sp.point.weights);
        sp.label = readString(istr,wchar);
        surf.surfPoints.push_back(sp);
    }
    // Texture coordinates:
    if (numUvs > 0) {
        surf.tris.uvInds.resize(surf.tris.size());
        surf.quads.uvInds.resize(surf.quads.size());
        mesh.uvs.resize(numUvs);
        istr.read(reinterpret_cast<char*>(&mesh.uvs[0]),int(8*numUvs));
        if (surf.tris.size() > 0)
            istr.read(reinterpret_cast<char*>(&surf.tris.uvInds[0]),int(12*numTris));
        if (surf.quads.size() > 0)
            istr.read(reinterpret_cast<char*>(&surf.quads.uvInds[0]),int(16*numQuads));
    }
    else if (texs) { // In the case of per vertex UVs we have to convert to indexed UVs
        surf.tris.uvInds.resize(surf.tris.size());
        surf.quads.uvInds.resize(surf.quads.size());
        mesh.uvs.resize(mesh.verts.size());
        istr.read(reinterpret_cast<char*>(&mesh.uvs[0]),int(8*numVerts));
        for (uint ii=0; ii<surf.tris.size(); ii++)
            for (uint jj=0; jj<3; jj++)
                surf.tris.uvInds[ii][jj] = surf.tris.vertInds[ii][jj];
        for (uint ii=0; ii<surf.quads.size(); ii++)
            for (uint jj=0; jj<4; jj++)
                surf.quads.uvInds[ii][jj] = surf.quads.vertInds[ii][jj];
    }
    // Delta morphs:
    mesh.deltaMorphs.resize(numDiffMorph);
    for (uint mm=0; mm<numDiffMorph; mm++) {
        mesh.deltaMorphs[mm].name = readString(istr,wchar);
        mesh.deltaMorphs[mm].verts.resize(numVerts);
        float       scale;
        readBinRaw_(istr,scale);
        for (uint vv=0; vv<numVerts; vv++) {
            Vec3S    sval;
            readBinRaw_(istr,sval);
            mesh.deltaMorphs[mm].verts[vv] = Vec3F(sval) * scale;
        }
    }
    // Target morphs:
    size_t                      targVertsStart = 0;
    mesh.targetMorphs.reserve(numStatMorph);
    for (uint ii=0; ii<numStatMorph; ++ii) {
        IndexedMorph       tm;
        tm.name = readString(istr,wchar);
        uint32                      numTargVerts;
        readBinRaw_(istr,numTargVerts);
        if (numTargVerts > 0) {         // For some reason this is not the case in v2.0 eyes
            tm.ivs.resize(numTargVerts);
            size_t          cnt {0};
            for (IdxVec3F & iv : tm.ivs) {
                istr.read((char*)&iv.idx,4);
                iv.vec = targVerts[targVertsStart + cnt++];
            }
            targVertsStart += numTargVerts;
            mesh.targetMorphs.push_back(tm);
        }
    }
    return mesh;
}

Mesh                loadTri(String8 const & fname)
{
    Ifstream        ff {fname};
    Mesh            ret = loadTri(ff);
    ret.name = pathToBase(fname);
    return ret;
}

Mesh
loadTri(String8 const & meshFile,String8 const & texImage)
{
    Mesh        mesh = loadTri(meshFile);
    loadImage_(texImage,mesh.surfaces[0].albedoMapRef());
    return mesh;
}

static
void
writeLabel(ostream & ostr,string const & str)
{
    // The spec requires writing a null terminator after the string:
    writeBinRaw_(ostr,uint32(str.size()+1));
    ostr.write(str.c_str(),str.size()+1);
}

void        saveTri(String8 const & fname,Mesh const & mesh)
{
    Surf const          surf = merge(mesh.surfaces);
    // Mesh must have both of these for valid UVs:
    bool                hasUvs = (surf.hasUvIndices() && !mesh.uvs.empty());
    SurfPointNames const & surfPoints = surf.surfPoints;
    size_t              numTargetMorphVerts = sumSizes(mesh.targetMorphs),
                        numBaseVerts = mesh.verts.size();
    Ofstream            ff(fname);
    ff.write(triIdent.data(),8);
    writeBinRaw_(ff,int32(numBaseVerts));               // V
    writeBinRaw_(ff,int32(surf.tris.size()));             // T
    writeBinRaw_(ff,int32(surf.quads.size()));            // Q
    writeBinRaw_(ff,int32(mesh.markedVerts.size()));    // numLabVerts (LV)
    writeBinRaw_(ff,int32(surfPoints.size()));          // numSurfPts (LS)
    int32               numUvs = hasUvs ? int32(mesh.uvs.size()) : 0;   // In case mesh is inconsistent
    writeBinRaw_(ff,int32(numUvs));                     // numUvs (X > 0 -> per-facet texture coordinates)
    if (hasUvs)
        writeBinRaw_(ff,int32(0x01));                   // <ext>: 0x01 -> texture coordinates
    else
        writeBinRaw_(ff,int32(0));
    writeBinRaw_(ff,int32(mesh.deltaMorphs.size()));    // numDiffMorph
    writeBinRaw_(ff,int32(mesh.targetMorphs.size()));   // numStatMorph
    writeBinRaw_(ff,int32(numTargetMorphVerts));        // numStatMorphVerts
    writeBinRaw_(ff,int32(0));
    writeBinRaw_(ff,int32(0));
    writeBinRaw_(ff,int32(0));
    writeBinRaw_(ff,int32(0));
    // Verts:
    for (uint ii=0; ii<mesh.verts.size(); ++ii)
        writeBinRaw_(ff,mesh.verts[ii]);
    for (size_t ii=0; ii<mesh.targetMorphs.size(); ++ii) {
        const IndexedMorph &   tm = mesh.targetMorphs[ii];
        for (IdxVec3F const & iv : tm.ivs)
            writeBinRaw_(ff,iv.vec);
    }
    // Facets:
    for (uint ii=0; ii<surf.tris.size(); ++ii)
        writeBinRaw_(ff,surf.tris.vertInds[ii]);
    for (uint ii=0; ii<surf.quads.size(); ++ii)
        writeBinRaw_(ff,surf.quads.vertInds[ii]);
    // Marked Verts:
    for (size_t ii=0; ii<mesh.markedVerts.size(); ++ii) {
        writeBinRaw_(ff,uint32(mesh.markedVerts[ii].idx));
        writeLabel(ff,mesh.markedVerts[ii].label);
    }
    // Surface Points:
    for (size_t ii=0; ii<surfPoints.size(); ii++) {
        SurfPointName const &   sp = surfPoints[ii];
        writeBinRaw_(ff,uint32(sp.point.triEquivIdx));
        writeBinRaw_(ff,sp.point.weights);
        writeLabel(ff,sp.label);
    }
    // UV list and per-facet UV indices if present:
    if (surf.hasUvIndices())
    {
        for(size_t ii=0; ii < mesh.uvs.size(); ++ii)
            writeBinRaw_(ff,mesh.uvs[ii]);
        for(size_t ii=0; ii < surf.tris.uvInds.size(); ++ii)
            writeBinRaw_(ff,surf.tris.uvInds[ii]);
        for(uint ii=0; ii < surf.quads.uvInds.size(); ++ii)
            writeBinRaw_(ff,surf.quads.uvInds[ii]);
    }
    // Delta morphs:
    for (size_t ii=0; ii<mesh.deltaMorphs.size(); ++ii) {   
        DirectMorph const &   morph = mesh.deltaMorphs[ii];
        FGASSERT(!morph.verts.empty());
        writeLabel(ff,morph.name.as_ascii());
        float           scale = float(numeric_limits<short>::max()-1) /
            cMaxElem(mapAbs(catH(cBounds(morph.verts))));
        writeBinRaw_(ff,1.0f/scale);
        for (size_t jj=0; jj<morph.verts.size(); ++jj)
            for (size_t kk=0; kk<3; ++kk)
                writeBinRaw_(ff,short(std::floor(morph.verts[jj][kk]*scale)+0.5f));
    }
    // Target morphs:
    for (size_t ii=0; ii<mesh.targetMorphs.size(); ++ii) {
        const IndexedMorph &   morph = mesh.targetMorphs[ii];
        writeLabel(ff,morph.name.as_ascii());
        writeBinRaw_(ff,uint32(morph.ivs.size()));
        for (size_t jj=0; jj<morph.ivs.size(); ++jj)
            writeBinRaw_(ff,uint32(morph.ivs[jj].idx));
    }
}

}
