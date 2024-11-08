//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Wavefront OBJ import / export
// 
// * This impl only supports polygonal geometry, no smooth surfaces or lines.
// * WOBJ does not support morphs, hierarchies, animation or units.
//

#include "stdafx.h"
#include "FgFile.hpp"
#include "FgImage.hpp"
#include "FgFileSystem.hpp"
#include "Fg3dMeshIo.hpp"
#include "Fg3dDisplay.hpp"
#include "FgParse.hpp"
#include "FgTestUtils.hpp"

using namespace std;

namespace Fg {

namespace {

float               parseFloat(String const & str)
{
    istringstream       iss {str};
    float               ret;
    iss >> ret;
    if (iss.fail())
        fgThrow("Not a valid floating point number",str);
    return ret;
}

int                 parseInt(String const & str)
{
    istringstream       iss {str};
    int                 ret;
    iss >> ret;
    if (iss.fail())
        fgThrow("Not a valid integer",str);
    return ret;
}

Vec3F               parseVert(
    String const &      str,
    bool &              homogeneous, // Set to true if there is a homogeneous coord (which is ignored)
    bool &              vertColors) // Set to true if there is a vertex color specified (which is ignored)
{
    Strings             nums = splitChar(str,' ');
    if (nums.size() < 3)
        fgThrow("Too few values specifying vertex");
    else if (nums.size() == 4)
        // A fourth homogeneous coord value can also be specified but is only used for rational
        // cureves so we ignore:
        homogeneous = true;
    else if (nums.size() == 6)
        vertColors = true;
    else if (nums.size() != 3)
        fgThrow("Invalid number of arguments for vertex");
    Vec3F               ret;
    for (uint ii=0; ii<3; ++ii)
        ret[ii] = parseFloat(nums[ii]);
    return ret;
}

Vec2F               parseUv(String const & str)
{
    Strings             nums = splitChar(str,' ');
    // A third homogeneous coord value can also be specified but is only used for rational
    // cureves so we ignore:
    FGASSERT((nums.size() > 1) && (nums.size() < 4));
    Vec2F               ret;
    for (uint ii=0; ii<2; ++ii)
        ret[ii] = parseFloat(nums[ii]);
    return ret;
}

bool                parseFacet_(
    String const &      line,
    size_t              numVerts,
    size_t              numUvs,
    NPolys<3> &         tris,
    NPolys<4> &         quads)
{
    bool                ret = false;
    Strings             chunks = splitChar(line,' ');
    // each facet must have 3 or more chunks, corresponding to a tri, quad, or N-gon:
    if (chunks.size() < 3)
        fgThrow("facet definition must contain at least 3 vertices");
    Uintss                  facet;
    for (auto const & chunk : chunks) {
        // all chunks must be the same form which can be one of:
        //      vertIdx
        //      vertIdx / uvIdx
        //      vertIdx / uvIdx / normIdx
        Strings             numStrs = splitChar(chunk,'/',true);
        size_t              sz = cMin(numStrs.size(),size_t(2));    // Ignore normal indices
        Uints               indices;
        for (size_t jj=0; jj<sz; ++jj) {
            int                 numLim = scast<int>((jj == 0) ? numVerts : numUvs);
            if (!numStrs[jj].empty()) {
                int                 num = parseInt(numStrs[jj]) - 1;    // WOBJ indexing starts at 1
                FGASSERT(num < int(numLim));
                // Indices can be negative in which case -1 refers to last index and so on backward:
                if (num < 0)
                    num += numLim;
                if ((num<0) || (num>=numLim))
                    fgThrow("index out of bounds",toStr(numStrs[jj]));
                indices.push_back(scast<uint>(num));
            }
        }
        facet.push_back(indices);
    }
    for (size_t ii=1; ii<facet.size(); ++ii)
        FGASSERT(facet[ii].size() == facet[ii-1].size());
    if (facet.size() == 3) {        // TRI
        tris.vertInds.emplace_back(facet[0][0],facet[1][0],facet[2][0]);
        if (facet[0].size() > 1)
            tris.uvInds.emplace_back(facet[0][1],facet[1][1],facet[2][1]);
    }
    if (facet.size() == 4) {        // QUAD
        quads.vertInds.emplace_back(facet[0][0],facet[1][0],facet[2][0],facet[3][0]);
        if (facet[0].size() > 1)
            quads.uvInds.emplace_back(facet[0][1],facet[1][1],facet[2][1],facet[3][1]);
    }
    if (facet.size() > 4) {          // N-GON: break into tris in simplest possible way:
        for (size_t ii=0; ii<facet.size()-2; ++ii) {
            tris.vertInds.emplace_back(facet[0][0],facet[ii+1][0],facet[ii+2][0]);
            if (facet[ii].size() > 1)
                tris.uvInds.emplace_back(facet[0][1],facet[ii+1][1],facet[ii+2][1]);
        }
        ret = true;
    }
    return ret;
}

}

Mesh                loadWObj(String8 const & fname)
{
    Mesh                mesh;
    map<String,Surf>    surfMap;
    Strings             lines = splitLines(loadRawString(fname));   // Removes empty lines
    Surf                currSurf;
    String              currName;
    size_t              numNgons = 0;
    bool                vertexColors = false,
                        vertexHomog = false,
                        hasLines = false;
    size_t              unnamedSurfCnt {0};
    auto                pushSurfFn = [&]()
    {
        if (!currSurf.empty()) {
            // If no name we must assume not part of any existing surface:
            if (currName.empty())
                currName = "Unnamed" + toStr(unnamedSurfCnt++);
            auto            it = surfMap.find(currName);
            if (it == surfMap.end())
                surfMap[currName] = currSurf;
            else
                merge_(surfMap[currName],currSurf);
            currSurf = Surf{};
        }
    };
    for (size_t ii=0; ii<lines.size(); ++ii) {
        try {
            String const &  line = lines[ii];
            if (line[0] == '#')
                continue;
            else if (beginsWith(line,"l ")) {
                hasLines = true;
                continue;
            }
            else if (beginsWith(line,"v "))
                mesh.verts.push_back(parseVert(line.substr(2),vertexHomog,vertexColors));
            else if (beginsWith(line,"vt "))
                mesh.uvs.push_back(parseUv(line.substr(3)));
            else if (beginsWith(line,"f ")) {
                if (parseFacet_(line.substr(2),mesh.verts.size(),mesh.uvs.size(),currSurf.tris,currSurf.quads))
                    ++numNgons;
            }
            else if (beginsWith(line,"g ")) {
                pushSurfFn();
                String              groupName = noLeadingWhitespace(cRest(line,2));
                if (!groupName.empty())
                    currName = noLeadingWhitespace(cRest(line,2));      // Group name takes precedence over others
            }
            else if (beginsWith(line,"usemtl ")) {
                pushSurfFn();
                if (currName.empty())                                   // Only use 'usemtl' name if no group name
                    currName = noLeadingWhitespace(cRest(line,7));
            }
            else if (beginsWith(line,"s ")) {
                pushSurfFn();
                if (currName.empty())                                   // Only use 's' name if no group name
                    currName = noLeadingWhitespace(cRest(line,2));
            }
            else if (beginsWith(line,"o ")) {
                pushSurfFn();
                if (currName.empty())                                   // Only use 'o' name if no group name
                    currName = noLeadingWhitespace(cRest(line,2));
            }
        }
        catch(const FgException & e) {
            fgout << fgnl << "WARNING: Error in line " << ii+1 << " of " << fname << ": " << e.englishMessage() << fgpush
                << fgnl << lines[ii] << fgpop;
        }
    }
    if (numNgons > 0)
        fgout << fgnl << "WARNING: OBJ " << numNgons << " N-gons broken into tris in " << fname;
    if (vertexHomog)
        fgout << fgnl << "WARNING: OBJ vertex homogeneous coordinates ignored.";
    if (vertexColors)
        fgout << fgnl << "WARNING: OBJ vertex color values ignored.";
    if (hasLines)
        fgout << fgnl << "WARNING: OBJ line elements ignored.";
    if (!currSurf.empty()) {
        if (surfMap.find(currName) == surfMap.end())
            surfMap[currName] = currSurf;
        else
            merge_(surfMap[currName],currSurf);
    }
    mesh.name = pathToBase(fname);
    Surfs               labelledSurfs;
    for (map<String,Surf>::iterator it = surfMap.begin(); it != surfMap.end(); ++it) {
        Surf &              srf = it->second;
        if (!srf.tris.validSize() || !srf.quads.validSize()) {
            srf.tris.uvInds.clear();
            srf.quads.uvInds.clear();
            fgout << fgnl << "WARNING: Partial UV indices ignored in " << fname << " surface " << it->first;
        }
        srf.name = it->first;
        labelledSurfs.push_back(srf);
    }
    // Important to split tiles (if presenet) into separate surfaces since this convention is not used in FaceGen code:
    // WOBJs can actually use UV domains in combination with 'o', 'g', 's' or 'usemtl' elements (eg. Reallusion):
    for (Surf const & surf : labelledSurfs)
        cat_(mesh.surfaces,splitByUvTile_(surf,mesh.uvs));
    return mesh;
}

namespace {

struct      Offsets
{
    uint        vert;
    uint        uv;
    uint        mat;

    Offsets(uint v,uint u,uint m) : vert(v), uv(u), mat(m) {}
};

template<size_t dim>
void                writeFacets(
    Ofstream &          ofs,
    const vector<Arr<uint,dim> > &  vertInds,
    const vector<Arr<uint,dim> > &  uvInds,
    Offsets             offsets)
{
    if (uvInds.size() == 0) {
        for (uint jj=0; jj<vertInds.size(); ++jj) {
            Arr<uint,dim>     vert = vertInds[jj];
            ofs << "f ";
            for (uint kk=0; kk<dim; ++kk) {
                uint                vi = vert[kk]+offsets.vert;
                ofs << vi << "//" << vi << " ";
            }
            ofs << "\n";
        }
    }
    else {
        FGASSERT(vertInds.size() == uvInds.size());
        for (uint jj=0; jj<vertInds.size(); ++jj) {
            Arr<uint,dim>     vert = vertInds[jj];
            Arr<uint,dim>     uv = uvInds[jj];
            ofs << "f ";
            for (uint kk=0; kk<dim; ++kk) {
                uint        vi = vert[kk]+offsets.vert;
                ofs << vi << "/" << uv[kk]+offsets.uv << "/" << vi << " ";
            }
            ofs << "\n";
        }
    }
}

void                writeMtlBase(Ofstream & ofs,uint idx)
{
    ofs << "newmtl " << "Texture" << toStr(idx) << "\n"
        "    illum 0\n"
        "    Kd 0.7 0.7 0.7\n"
        "    Ks 0 0 0\n"
        "    Ka 0 0 0\n";
}

Offsets             writeMesh(
    Ofstream &          ofs,
    Ofstream &          ofsMtl,
    Mesh const &        mesh,
    Path const &        fpath,
    Offsets             offsets,
    String const &      imgFormat,
    bool                mtlFile)        // Is there an associated MTL file
{
    if (!mesh.deltaMorphs.empty())
        fgout << "\n" << "WARNING: OBJ format does not support morphs";

    for (uint ii=0; ii<mesh.verts.size(); ++ii) {
        Vec3F               vert = mesh.verts[ii];
        ofs << "v " << vert[0] << " " << vert[1] << " " << vert[2] << "\n";
    }
    for (size_t ii=0; ii<mesh.uvs.size(); ++ii) {
        Vec2F               uv = mesh.uvs[ii];
        ofs << "vt " << uv[0] << " " << uv[1] << "\n";
    }
    MeshNormals         norms = cNormals(mesh.surfaces,mesh.verts);
    for (size_t ii=0; ii<norms.vert.size(); ++ii) {
        Vec3F               n = norms.vert[ii];
        ofs << "vn " << n[0] << " " << n[1] << " " << n[2] << "\n";
    }
    for (uint tt=0; tt<mesh.surfaces.size(); ++tt) {
        if (mesh.surfaces[tt].material.albedoMap) {
            String              idxString = toStr(offsets.mat+tt);
            // Some OBJ parsers (Meshlab) can't handle spaces in filename:
            String8             imgName = fpath.base.replace(' ','_')+idxString+"."+imgFormat;
            saveImage(*mesh.surfaces[tt].material.albedoMap,fpath.dir()+imgName);
            if (ofsMtl) {
                writeMtlBase(ofsMtl,offsets.mat+tt);
                ofsMtl << "    map_Kd " << imgName << "\n";
            }
        }
    }
    for (size_t ii=0; ii<mesh.surfaces.size(); ++ii) {
        Surf const &        surf = mesh.surfaces[ii];
        String8             name = (surf.name.empty() ? String8("Surf")+toStr(ii) : surf.name);
        ofs << "g " << name << "\n";
        if (mtlFile)    // Meshlab can't handle 'usemtl' if there is no MTL file:
            ofs << "usemtl Texture" << toStr(offsets.mat+ii) << "\n";
        writeFacets(ofs,surf.tris.vertInds,surf.tris.uvInds,offsets);
        writeFacets(ofs,surf.quads.vertInds,surf.quads.uvInds,offsets);
    }
    offsets.vert += uint(mesh.verts.size());
    offsets.uv += uint(mesh.uvs.size());
    offsets.mat += uint(mesh.surfaces.size());
    return offsets;
}

}

void    saveWObj(String8 const & filename,Meshes const & meshes,String imgFormat)
{
    Path            fpath(filename);
    bool            texImage = false;
    for (size_t ii=0; ii<meshes.size(); ++ii)
        if (meshes[ii].numValidAlbedoMaps() > 0)
            texImage = true;
    Ofstream        ofs(filename);
    Ofstream        ofsMtl;
    ofs.precision(7);
    ofs <<
        "# Wavefront OBJ format.\n"
        "# Generated by FaceGen, for more information visit https://facegen.com\n";
    // Some OBJ parsers (MeshLab) can't handle spaces in filenames:
    if (texImage) {
        String8         mtlBaseExt = fpath.base.replace(' ','_') + ".mtl";
        ofs << "mtllib " << mtlBaseExt << "\n";
        ofsMtl.open(fpath.dir() + mtlBaseExt);
        writeMtlBase(ofsMtl,0);
    }
    Offsets         offsets(1,1,1);
    for (size_t ii=0; ii<meshes.size(); ++ii) {
        Mesh const &    mesh = meshes[ii];
        String8         name = (mesh.name.empty() ? fpath.base : mesh.name);
        // Replace spaces with underscores in case some OBJ parsers can't handle that:
        name = name.replace(' ','_');
        ofs << "o " << name << "\n"
            << "s 1" << "\n";    // Enable smooth shading
        offsets = writeMesh(ofs,ofsMtl,meshes[ii],fpath,offsets,imgFormat,texImage);
    }
}

void    injectVertsWObj(String8 const & inName,Vec3Fs const & verts,String8 const & outName)
{
    Strings             lines = splitLines(loadRawString(inName));
    size_t              cnt {0};
    for (String & line : lines) {
        if (beginsWith(line,"v ")) {
            if (cnt >= verts.size())
                fgThrow("Inject verts to OBJ: Too few vertices",toStr(verts.size()));
            Vec3F           vert = verts[cnt++];
            line = "v " + toStr(vert[0]) + " " + toStr(vert[1]) + " " + toStr(vert[2]);
        }
    }
    if (cnt < verts.size())
        fgout << fgnl << "WARNING: Inject verts to OBJ: Too many vertices: " << verts.size() << " > " << cnt;
    saveRaw(cat(lines,"\n"),outName);
}

void                testSaveObj(CLArgs const & args)
{
    FGTESTDIR
    String8         dd = dataDir();
    String          rd = "base/";
    Mesh            mouth = loadTri(dd+rd+"Mouth.tri");
    mouth.deltaMorphs.clear();       // Remove morphs to avoid warning
    mouth.targetMorphs.clear();
    mouth.surfaces[0].setAlbedoMap(loadImage(dd+rd+"MouthSmall.png"));
    Mesh            glasses = loadTri(dd+rd+"Glasses.tri");
    glasses.surfaces[0].setAlbedoMap(loadImage(dd+rd+"Glasses.tga"));
    saveWObj("meshExportObj.obj",{mouth,glasses});
    if (is64Bit() && (getCurrentCompiler()==Compiler::vs22))      // precision differences otherwise
        regressFileRel("meshExportObj.obj","base/test/");
    regressFileRel("meshExportObj.mtl","base/test/");
    regressFileRel("meshExportObj1.png","base/test/");
    regressFileRel("meshExportObj2.png","base/test/");
}

void                testLoadObj(CLArgs const & args)
{
    TestDir   tmp("readObj");
    ofstream    ofs("square.obj");
    ofs << 
        "v 0.0 0.0 0.0\n"
        "v 0.0 1.0 0.0\n"
        "v 1.0 1.0 0.0\n"
        "v 1.0 0.0 0.0\n"
        "vt 0.0 0.0\n"
        "vt 0.0 1.0\n"
        "vt 1.0 1.0\n"
        "vt 1.0 0.0\n"
        "g plane\n"
        "usemtl plane\n"
        "f 1/1 2/2 3/3 4/4\n"
        ;
    ofs.close();
    Mesh    mesh = loadWObj("square.obj");
    if (!isAutomated(args))
        viewMesh(mesh);
}


}

// */
