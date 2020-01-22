//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "Fg3dMesh.hpp"
#include "FgException.hpp"
#include "FgStdStream.hpp"
#include "FgBounds.hpp"
#include "FgFileSystem.hpp"

using namespace std;

namespace Fg {

static string triIdent = "FRTRI003";

static
string
readString(istream & istr,bool wchar)
{
    uint        size;
    readb(istr,size);
    string      str;
    if (size == 0)
        return str;
    str.resize(size);
    for (uint ii=0; ii<size; ++ii) {
        if (wchar) {
            wchar_t     wch;
            readb(istr,wch);
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
    if (strncmp(cdata,"FRTRI103",8) == 0)
        fgThrow("File is encrypted, use 'fileconvert' utility to decrypt");
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
    readb(istr,numVerts);
    readb(istr,numTris);
    readb(istr,numQuads);
    readb(istr,numLabVerts);
    readb(istr,numSurfPts);
    readb(istr,numUvs);
    readb(istr,texExt);
    readb(istr,numDiffMorph);
    readb(istr,numStatMorph);
    readb(istr,numStatMorphVerts);
    istr.read(buff,16);
    bool    texs = ((texExt & 0x01) != 0),
            wchar = ((texExt & 0x02) != 0);
    if (wchar)
        fgout << fgnl << "WARNING: Unicode labels being converted to ASCII.";

    // Read in the verts:
    if (numVerts==0)
        fgThrow("TRI file has no vertices");
    mesh.verts.resize(numVerts);
    vector<Vec3F>    targVerts(numStatMorphVerts);
    istr.read(reinterpret_cast<char*>(&mesh.verts[0]),int(12*numVerts));
    if (numStatMorphVerts > 0)
        istr.read(reinterpret_cast<char*>(&targVerts[0]),int(12*numStatMorphVerts));

    // Read in the surface if there is any surface data (a TRI has only one):
    bool            hasSurface =  ((numTris > 0) || (numQuads > 0));
    if (hasSurface)
        mesh.surfaces.resize(1);
    Surf            dummy;
    Surf &          surf = hasSurface ? mesh.surfaces[0] : dummy;
    surf.tris.posInds.resize(numTris);
    if (numTris > 0)
        istr.read(reinterpret_cast<char*>(&surf.tris.posInds[0]),int(12*numTris));
    surf.quads.posInds.resize(numQuads);
    if (numQuads > 0)
        istr.read(reinterpret_cast<char*>(&surf.quads.posInds[0]),int(16*numQuads));
    // Marked verts:
    mesh.markedVerts.resize(numLabVerts);
    for (uint jj=0; jj<numLabVerts; jj++) {
        istr.read(reinterpret_cast<char*>(&mesh.markedVerts[jj].idx),4);
        mesh.markedVerts[jj].label = readString(istr,wchar);
    }
    // Surface points:
    for (uint ii=0; ii<numSurfPts; ii++) {
        SurfPoint     sp;
        readb(istr,sp.triEquivIdx);
        readb(istr,sp.weights);
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
                surf.tris.uvInds[ii][jj] = surf.tris.posInds[ii][jj];
        for (uint ii=0; ii<surf.quads.size(); ii++)
            for (uint jj=0; jj<4; jj++)
                surf.quads.uvInds[ii][jj] = surf.quads.posInds[ii][jj];
    }
    // Delta morphs:
    mesh.deltaMorphs.resize(numDiffMorph);
    for (uint mm=0; mm<numDiffMorph; mm++) {
        mesh.deltaMorphs[mm].name = readString(istr,wchar);
        mesh.deltaMorphs[mm].verts.resize(numVerts);
        float       scale;
        readb(istr,scale);
        for (uint vv=0; vv<numVerts; vv++) {
            Vec3S    sval;
            readb(istr,sval);
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
        readb(istr,numTargVerts);
        if (numTargVerts > 0) {         // For some reason this is not the case in v2.0 eyes
            tm.baseInds.resize(numTargVerts);
            istr.read((char*)&tm.baseInds[0],4*numTargVerts);
            tm.verts = cutSubvec(targVerts,targVertsStart,numTargVerts);
            targVertsStart += numTargVerts;
            mesh.targetMorphs.push_back(tm);
        }
    }
    return mesh;
}

Mesh
loadTri(Ustring const & fname)
{
    Mesh        ret;
    try {
        Ifstream      ff(fname);
        ret = loadTri(ff);
    }
    catch (FgException & e) {
        e.m_ct.back().dataUtf8 = fname.m_str;
        throw;
    }
    ret.name = pathToBase(fname);
    return ret;
}

Mesh
loadTri(Ustring const & meshFile,Ustring const & texImage)
{
    Mesh        mesh = loadTri(meshFile);
    imgLoadAnyFormat(texImage,mesh.surfaces[0].albedoMapRef());
    return mesh;
}

static
void
writeLabel(ostream & ostr,string const & str)
{
    // The spec requires writing a null terminator after the string:
    fgWriteb(ostr,uint32(str.size()+1));
    ostr.write(str.c_str(),str.size()+1);
}

void
saveTri(
    Ustring const &     fname,
    Mesh const &        mesh)
{
    Surf const          surf = mergeSurfaces(mesh.surfaces);
    SurfPoints const &  surfPoints = surf.surfPoints;
    size_t              numTargetMorphVerts = fgSumVerts(mesh.targetMorphs),
                        numBaseVerts = mesh.verts.size();
    Ofstream            ff(fname);
    ff.write(triIdent.data(),8);
    fgWriteb(ff,int32(numBaseVerts));               // V
    fgWriteb(ff,int32(surf.numTris()));             // T
    fgWriteb(ff,int32(surf.numQuads()));            // Q
    fgWriteb(ff,int32(mesh.markedVerts.size()));    // numLabVerts (LV)
    fgWriteb(ff,int32(surfPoints.size()));          // numSurfPts (LS)
    int32               numUvs = int32(mesh.uvs.size());
    fgWriteb(ff,int32(numUvs));                     // numUvs (X > 0 -> per-facet texture coordinates)
    if (surf.hasUvIndices())
        fgWriteb(ff,int32(0x01));                   // <ext>: 0x01 -> texture coordinates
    else
        fgWriteb(ff,int32(0));
    fgWriteb(ff,int32(mesh.deltaMorphs.size()));    // numDiffMorph
    fgWriteb(ff,int32(mesh.targetMorphs.size()));   // numStatMorph
    fgWriteb(ff,int32(numTargetMorphVerts));        // numStatMorphVerts
    fgWriteb(ff,int32(0));
    fgWriteb(ff,int32(0));
    fgWriteb(ff,int32(0));
    fgWriteb(ff,int32(0));

    // Verts:
    for (uint ii=0; ii<mesh.verts.size(); ++ii)
        fgWriteb(ff,mesh.verts[ii]);
    for (size_t ii=0; ii<mesh.targetMorphs.size(); ++ii) {
        const IndexedMorph &   tm = mesh.targetMorphs[ii];
        for (size_t jj=0; jj<tm.verts.size(); ++jj)
            fgWriteb(ff,tm.verts[jj]);
    }

    // Facets:
    for (uint ii=0; ii<surf.numTris(); ++ii)
        fgWriteb(ff,surf.getTriPosInds(ii));
    for (uint ii=0; ii<surf.numQuads(); ++ii)
        fgWriteb(ff,surf.getQuadPosInds(ii));

    // Marked Verts:
    for (size_t ii=0; ii<mesh.markedVerts.size(); ++ii) {
        fgWriteb(ff,mesh.markedVerts[ii].idx);
        writeLabel(ff,mesh.markedVerts[ii].label);
    }

    // Surface Points:
    for (size_t ii=0; ii<surfPoints.size(); ii++) {
        const SurfPoint &   sp = surfPoints[ii];
        fgWriteb(ff,sp.triEquivIdx);
        fgWriteb(ff,sp.weights);
        writeLabel(ff,sp.label);
    }
    // UV list and per-facet UV indices if present:
    if (surf.hasUvIndices())
    {
        for(size_t ii=0; ii < mesh.uvs.size(); ++ii)
            fgWriteb(ff,mesh.uvs[ii]);
        for(size_t ii=0; ii < surf.tris.uvInds.size(); ++ii)
            fgWriteb(ff,surf.tris.uvInds[ii]);
        for(uint ii=0; ii < surf.quads.uvInds.size(); ++ii)
            fgWriteb(ff,surf.quads.uvInds[ii]);
    }

    // Delta morphs:
    for (size_t ii=0; ii<mesh.deltaMorphs.size(); ++ii) {   
        Morph const &   morph = mesh.deltaMorphs[ii];
        FGASSERT(!morph.verts.empty());
        writeLabel(ff,morph.name.as_ascii());
        float           scale = float(numeric_limits<short>::max()-1) / cMaxElem(mapAbs(cBounds(morph.verts)));
        fgWriteb(ff,1.0f/scale);
        for (size_t jj=0; jj<morph.verts.size(); ++jj)
            for (size_t kk=0; kk<3; ++kk)
                fgWriteb(ff,short(std::floor(morph.verts[jj][kk]*scale)+0.5f));
    }

    // Target morphs:
    for (size_t ii=0; ii<mesh.targetMorphs.size(); ++ii) {
        const IndexedMorph &   morph = mesh.targetMorphs[ii];
        writeLabel(ff,morph.name.as_ascii());
        fgWriteb(ff,uint32(morph.baseInds.size()));
        for (size_t jj=0; jj<morph.baseInds.size(); ++jj)
            fgWriteb(ff,uint32(morph.baseInds[jj]));
    }
}

}
