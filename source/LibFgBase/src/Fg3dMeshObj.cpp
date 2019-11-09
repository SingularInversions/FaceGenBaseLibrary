//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//


#include "stdafx.h"
#include "FgStdStream.hpp"
#include "FgImage.hpp"
#include "FgFileSystem.hpp"
#include "Fg3dMeshIo.hpp"
#include "Fg3dMeshOps.hpp"
#include "FgParse.hpp"
#include "Fg3dNormals.hpp"
#include "FgTestUtils.hpp"

using namespace std;

namespace Fg {

//void
//fgObjTest()
//{
//    ofstream    ofs("square.obj");
//    ofs << 
//        "v 0.0 0.0 0.0\n"
//        "v 0.0 1.0 0.0\n"
//        "v 1.0 1.0 0.0\n"
//        "v 1.0 0.0 0.0\n"
//        "vt 0.0 0.0\n"
//        "vt 0.0 1.0\n"
//        "vt 1.0 1.0\n"
//        "vt 1.0 0.0\n"
//        "g plane\n"
//        "usemtl plane\n"
//        "f 1/1 2/2 3/3 4/4\n"
//        ;
//    ofs.close();
//}

static
float
parseFloat(const string & str)
{
    istringstream   iss(str);
    float           ret;
    iss >> ret;
    return ret;
}

static
Vec3F
parseVert(
    string const &  str,
    bool &          homogenous, // Set to true if there is a homogenous coord (which is ignored)
    bool &          vertColors) // Set to true if there is a vertex color specified (which is ignored)
{
    Strings         nums = splitChar(str,' ');
    if (nums.size() < 3)
        fgThrow("Too few values specifying vertex");
    else if (nums.size() == 4)
        // A fourth homogenous coord value can also be specified but is only used for rational
        // cureves so we ignore:
        homogenous = true;
    else if (nums.size() == 6)
        vertColors = true;
    else if (nums.size() != 3)
        fgThrow("Invalid number of arguments for vertex");
    Vec3F           ret;
    for (uint ii=0; ii<3; ++ii)
        ret[ii] = parseFloat(nums[ii]);
    return ret;
}

static
Vec2F
parseUv(const string & str)
{
    vector<string>  nums = splitChar(str,' ');
    // A third homogenous coord value can also be specified but is only used for rational
    // cureves so we ignore:
    FGASSERT((nums.size() > 1) && (nums.size() < 4));
    Vec2F        ret;
    for (uint ii=0; ii<2; ++ii)
        ret[ii] = parseFloat(nums[ii]);
    return ret;
}

static
bool
parseFacet(
    const string &      str,
    size_t              numVerts,
    size_t              numUvs,
    FacetInds<3> &    tris,
    FacetInds<4> &    quads)
{
    bool            ret = false;
    vector<string>  strs = splitChar(str,' ');
    FGASSERT(strs.size() > 2);
    vector<vector<uint> >   nums;
    for (size_t ii=0; ii<strs.size(); ++ii) {
        vector<string>  ns = splitChar(strs[ii],'/',true);
        size_t          sz = minEl(ns.size(),size_t(2));    // Ignore normal indices
        vector<uint>    nms;
        for (size_t jj=0; jj<sz; ++jj) {
            size_t      numLim = ((jj == 0) ? numVerts : numUvs);
            if (!ns[jj].empty()) {
                istringstream   iss(ns[jj]);
                int             num;
                iss >> num;
                --num;                              // WOBJ indexing starts at 1
                FGASSERT(num < int(numLim));
                // Indices can be negative in which case -1 refers to last index and so on backward:
                if (num < 0) {
                    FGASSERT(size_t(-num) <= numLim);
                    nms.push_back(uint(int(numLim)+num));
                }
                else
                    nms.push_back(uint(num));
            }
        }
        nums.push_back(nms);
    }
    for (size_t ii=1; ii<nums.size(); ++ii)
        FGASSERT(nums[ii].size() == nums[ii-1].size());
    if (nums.size() == 3) {
        tris.vertInds.push_back(Vec3UI(nums[0][0],nums[1][0],nums[2][0]));
        if (nums[0].size() > 1)
            tris.uvInds.push_back(Vec3UI(nums[0][1],nums[1][1],nums[2][1]));
    }
    if (nums.size() == 4) {
        quads.vertInds.push_back(Vec4UI(nums[0][0],nums[1][0],nums[2][0],nums[3][0]));
        if (nums[0].size() > 1)
            quads.uvInds.push_back(Vec4UI(nums[0][1],nums[1][1],nums[2][1],nums[3][1]));
    }
    if (nums.size() > 4) {          // N-gon
        for (size_t ii=0; ii<nums.size()-2; ++ii) {
            tris.vertInds.push_back(Vec3UI(nums[0][0],nums[ii+1][0],nums[ii+2][0]));
            if (nums[ii].size() > 1)
                tris.uvInds.push_back(Vec3UI(nums[0][1],nums[ii+1][1],nums[ii+2][1]));
        }
        ret = true;
    }
    return ret;
}

Mesh
loadWobj(
    Ustring const &     fname,
    string              surfSeparator)
{
    Mesh                mesh;
    string              currName;
    map<string,Surf>    surfs;
    Strings             lines = splitLines(fgSlurp(fname));   // Removes empty lines
    Surf                surf;
    size_t              numNgons = 0;
    bool                vertexColors = false,
                        vertexHomogenous = false;
    for (size_t ii=0; ii<lines.size(); ++ii) {
        try {
            string const &  line = lines[ii];
            if (line[0] == 'v') {
                if (line.at(1) == ' ')
                    mesh.verts.push_back(parseVert(line.substr(2),vertexHomogenous,vertexColors));
                if (line.at(1) == 't')
                    if (line.at(2) == ' ')
                        mesh.uvs.push_back(parseUv(line.substr(3)));
            }
            if (line[0] == 'f') {
                if (line.at(1) == ' ') {
                    if (parseFacet(line.substr(2),mesh.verts.size(),mesh.uvs.size(),surf.tris,surf.quads))
                        ++numNgons;
                }
            }
            if (!surfSeparator.empty() && fgBeginsWith(line,surfSeparator)) {
                vector<string>  words = fgSplitAtSeparators(line,' ');
                if (words.size() != 2) {
                    fgout << "WARNING: Invalid " << surfSeparator << " name on line " << ii << " of " << fname;
                    break;
                }
                string          name = words[1];
                if (currName != name) {
                    if (!surf.empty()) {
                        if (surfs.find(currName) == surfs.end())
                            surfs[currName] = surf;
                        else
                            surfs[currName].merge(surf);
                    }
                    currName = name;
                    surf = Surf();
                }
            }
            if (line[0] == '#')
                continue;
        }
        catch(const FgException & e) {
            fgout << fgnl << "WARNING: Error in line " << ii+1 << " of " << fname << ": " << e.tr_message() << fgpush
                << fgnl << lines[ii] << fgpop;
        }
    }
    if (numNgons > 0)
        fgout << fgnl << "WARNING: " << numNgons << " N-gons broken into tris in " << fname;
    if (vertexHomogenous)
        fgout << fgnl << "WARNING: Vertex homogenous coordinates ignored.";
    if (vertexColors)
        fgout << fgnl << "WARNING: Vertex color values ignored.";
    if (!surf.empty()) {
        if (surfs.find(currName) == surfs.end())
            surfs[currName] = surf;
        else
            surfs[currName].merge(surf);
    }
    mesh.name = fgPathToBase(fname);
    for (map<string,Surf>::iterator it = surfs.begin(); it != surfs.end(); ++it) {
        Surf &   srf = it->second;
        if (!srf.tris.valid() || !srf.quads.valid()) {
            srf.tris.uvInds.clear();
            srf.quads.uvInds.clear();
            fgout << fgnl << "WARNING: Partial UV indices ignored in " << fname << " surface " << it->first;
        }
        srf.name = it->first;
        mesh.surfaces.push_back(srf);
    }
    // Some OBJ meshes make use of wrap aliasing in their UVs (eg. Daz Gen 3):
    bool        uvsWrapped = false;
    for (size_t ii=0; ii<mesh.uvs.size(); ++ii) {
        Vec2F &  uv = mesh.uvs[ii];
        for (uint xx=0; xx<2; ++xx) {
            if ((uv[xx] < 0.0f) || (uv[xx] > 1.0f)) {
                uvsWrapped = true;
                uv[xx] = uv[xx] - floor(uv[xx]);
            }
        }
    }
    if (uvsWrapped)
        fgout << fgnl << "WARNING: UV indices unwrapped.";
    return mesh;
}

struct  Offsets
{
    uint    vert;
    uint    uv;
    uint    mat;

    Offsets(uint v,uint u,uint m)
    : vert(v), uv(u), mat(m)
    {}
};

template<uint dim>
void
writeFacets(
    Ofstream &    ofs,
    const vector<Mat<uint,dim,1> > &  vertInds,
    const vector<Mat<uint,dim,1> > &  uvInds,
    Offsets         offsets)
{
    if (uvInds.size() == 0) {
        for (uint jj=0; jj<vertInds.size(); ++jj) {
            Mat<uint,dim,1>   vert = vertInds[jj];
            ofs << "f ";
            for (uint kk=0; kk<dim; ++kk) {
                uint        vi = vert[kk]+offsets.vert;
                ofs << vi << "//" << vi << " ";
            }
            ofs << "\n";
        }
    }
    else {
        FGASSERT(vertInds.size() == uvInds.size());
        for (uint jj=0; jj<vertInds.size(); ++jj) {
            Mat<uint,dim,1>   vert = vertInds[jj];
            Mat<uint,dim,1>   uv = uvInds[jj];
            ofs << "f ";
            for (uint kk=0; kk<dim; ++kk) {
                uint        vi = vert[kk]+offsets.vert;
                ofs << vi << "/" << uv[kk]+offsets.uv << "/" << vi << " ";
            }
            ofs << "\n";
        }
    }
}

static void
writeMtlBase(
    Ofstream &    ofs,
    uint            idx)
{
    ofs << "newmtl " << "Texture" << toString(idx) << "\n"
        "    illum 0\n"
        "    Kd 0.7 0.7 0.7\n"
        "    Ks 0 0 0\n"
        "    Ka 0 0 0\n";
}

static
Offsets
writeMesh(
    Ofstream &        ofs,
    Ofstream &        ofsMtl,
    const Mesh &    mesh,
    const Path &      fpath,
    Offsets             offsets,
    const string &      imgFormat,
    bool                mtlFile)        // Is there an associated MTL file
{
    if (!mesh.deltaMorphs.empty())
        fgout << "\n" << "WARNING: OBJ format does not support morphs";

    for (uint ii=0; ii<mesh.verts.size(); ++ii) {
        Vec3F    vert = mesh.verts[ii];
        ofs << "v " << vert[0] << " " << vert[1] << " " << vert[2] << "\n";
    }
    for (size_t ii=0; ii<mesh.uvs.size(); ++ii) {
        Vec2F    uv = mesh.uvs[ii];
        ofs << "vt " << uv[0] << " " << uv[1] << "\n";
    }
    Normals         norms = calcNormals(mesh.surfaces,mesh.verts);
    for (size_t ii=0; ii<norms.vert.size(); ++ii) {
        Vec3F    n = norms.vert[ii];
        ofs << "vn " << n[0] << " " << n[1] << " " << n[2] << "\n";
    }
    for (uint tt=0; tt<mesh.surfaces.size(); ++tt) {
        if (mesh.surfaces[tt].material.albedoMap) {
            string  idxString = toString(offsets.mat+tt);
            // Some OBJ parsers (Meshlab) can't handle spaces in filename:
            Ustring        imgName = fpath.base.replace(' ','_')+idxString+"."+imgFormat;
            imgSaveAnyFormat(fpath.dir()+imgName,*mesh.surfaces[tt].material.albedoMap);
            if (ofsMtl) {
                writeMtlBase(ofsMtl,offsets.mat+tt);
                ofsMtl << "    map_Kd " << imgName << "\n";
            }
        }
    }
    for (size_t ii=0; ii<mesh.surfaces.size(); ++ii) {
        const Surf & surf = mesh.surfaces[ii];
        Ustring    name = (surf.name.empty() ? Ustring("Surf")+toString(ii) : surf.name);
        ofs << "g " << name << "\n";
        if (mtlFile)    // Meshlab can't handle 'usemtl' if there is no MTL file:
            ofs << "usemtl Texture" << toString(offsets.mat+ii) << "\n";
        writeFacets(ofs,surf.tris.vertInds,surf.tris.uvInds,offsets);
        writeFacets(ofs,surf.quads.vertInds,surf.quads.uvInds,offsets);
    }
    offsets.vert += uint(mesh.verts.size());
    offsets.uv += uint(mesh.uvs.size());
    offsets.mat += uint(mesh.surfaces.size());
    return offsets;
}

void
saveObj(
    const Ustring &            filename,
    const vector<Mesh> &    meshes,
    string                      imgFormat)
{
    Path      fpath(filename);
    bool        texImage = false;
    for (size_t ii=0; ii<meshes.size(); ++ii)
        if (meshes[ii].numValidAlbedoMaps() > 0)
            texImage = true;
    Ofstream  ofs(fpath.dirBase()+".obj");
    Ofstream  ofsMtl;
    ofs.precision(7);
    ofs <<
        "# Wavefront OBJ format.\n"
        "# Generated by FaceGen, for more information visit https://facegen.com\n";
    // Some OBJ parsers (MeshLab) can't handle spaces in filenames:
    if (texImage) {
        Ustring    mtlBaseExt = fpath.base.replace(' ','_') + ".mtl";
        ofs << "mtllib " << mtlBaseExt << "\n";
        ofsMtl.open(fpath.dir() + mtlBaseExt);
        writeMtlBase(ofsMtl,0);
    }
    Offsets     offsets(1,1,1);
    for (size_t ii=0; ii<meshes.size(); ++ii) {
        const Mesh &    mesh = meshes[ii];
        Ustring            name = (mesh.name.empty() ? fpath.base : mesh.name);
        // Replace spaces with underscores in case some OBJ parsers can't handle that:
        name = name.replace(' ','_');
        ofs << "o " << name << "\n"
            << "s 1" << "\n";    // Enable smooth shading
        offsets = writeMesh(ofs,ofsMtl,meshes[ii],fpath,offsets,imgFormat,texImage);
    }
}

void
fgSaveObjTest(const CLArgs & args)
{
    FGTESTDIR
    Ustring        dd = dataDir();
    string          rd = "base/";
    Mesh        mouth = loadTri(dd+rd+"Mouth.tri");
    mouth.deltaMorphs.clear();       // Remove morphs to avoid warning
    mouth.targetMorphs.clear();
    mouth.surfaces[0].setAlbedoMap(imgLoadAnyFormat(dd+rd+"MouthSmall.png"));
    Mesh        glasses = loadTri(dd+rd+"Glasses.tri");
    glasses.surfaces[0].setAlbedoMap(imgLoadAnyFormat(dd+rd+"Glasses.tga"));
    saveObj("meshExportObj",fgSvec(mouth,glasses));
    regressFileRel("meshExportObj.obj","base/test/");
    regressFileRel("meshExportObj.mtl","base/test/");
    regressFileRel("meshExportObj1.png","base/test/");
    regressFileRel("meshExportObj2.png","base/test/");
}

}

// */
