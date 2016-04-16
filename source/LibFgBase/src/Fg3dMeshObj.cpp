//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors: Andrew Beatty
//

#include "stdafx.h"
#include "FgStdStream.hpp"
#include "FgImage.hpp"
#include "FgFileSystem.hpp"
#include "Fg3dMeshOps.hpp"
#include "FgTokenizer.hpp"
#include "FgParse.hpp"
#include "Fg3dNormals.hpp"

#include <boost/algorithm/string.hpp>

using namespace std;

void
fgObjTest()
{
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
}

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
FgVect3F
parseVert(const string & str)
{
    vector<string>  nums = fgSplitChar(str,' ');
    // A fourth homogenous coord value can also be specified but is only used for rational
    // cureves so we ignore:
    FGASSERT((nums.size() > 2) && (nums.size() < 5));
    FgVect3F        ret;
    for (uint ii=0; ii<3; ++ii)
        ret[ii] = parseFloat(nums[ii]);
    return ret;
}

static
FgVect2F
parseUv(const string & str)
{
    vector<string>  nums = fgSplitChar(str,' ');
    // A third homogenous coord value can also be specified but is only used for rational
    // cureves so we ignore:
    FGASSERT((nums.size() > 1) && (nums.size() < 4));
    FgVect2F        ret;
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
    FgFacetInds<3> &    tris,
    FgFacetInds<4> &    quads)
{
    bool            ret = false;
    vector<string>  strs = fgSplitChar(str,' ');
    FGASSERT(strs.size() > 2);
    vector<vector<uint> >   nums;
    for (size_t ii=0; ii<strs.size(); ++ii) {
        vector<string>  ns = fgSplitChar(strs[ii],'/',true);
        size_t          sz = fgMin(ns.size(),size_t(2));    // Ignore normal indices
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
        tris.vertInds.push_back(FgVect3UI(nums[0][0],nums[1][0],nums[2][0]));
        if (nums[0].size() > 1)
            tris.uvInds.push_back(FgVect3UI(nums[0][1],nums[1][1],nums[2][1]));
    }
    if (nums.size() == 4) {
        quads.vertInds.push_back(FgVect4UI(nums[0][0],nums[1][0],nums[2][0],nums[3][0]));
        if (nums[0].size() > 1)
            quads.uvInds.push_back(FgVect4UI(nums[0][1],nums[1][1],nums[2][1],nums[3][1]));
    }
    if (nums.size() > 4) {          // N-gon
        for (size_t ii=0; ii<nums.size()-2; ++ii) {
            tris.vertInds.push_back(FgVect3UI(nums[0][0],nums[ii+1][0],nums[ii+2][0]));
            if (nums[ii].size() > 1)
                tris.uvInds.push_back(FgVect3UI(nums[0][1],nums[ii+1][1],nums[ii+2][1]));
        }
        ret = true;
    }
    return ret;
}

void
fgLoadWobj(
    const FgString &    fname,
    Fg3dMesh &          meshRef,
    string              surfSeparator)
{
    string                      currName;
    map<string,Fg3dSurface>     surfs;
    vector<string>      lines = fgSplitLines(fgSlurp(fname));
    Fg3dMesh            mesh;
    Fg3dSurface         surf;
    size_t              numNgons = 0;
    for (size_t ii=0; ii<lines.size(); ++ii) {
        try {
            const string &  line = lines[ii];
            if (line[0] == 'v') {
                if (line[1] == ' ')
                    mesh.verts.push_back(parseVert(line.substr(2)));
                if (line[1] == 't')
                    if (line[2] == ' ')
                        mesh.uvs.push_back(parseUv(line.substr(3)));
            }
            if (line[0] == 'f') {
                if (line[1] == ' ') {
                    if (parseFacet(line.substr(2),mesh.verts.size(),mesh.uvs.size(),surf.tris,surf.quads))
                        ++numNgons;
                }
            }
            if (!surfSeparator.empty() && fgStartsWith(line,surfSeparator)) {
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
                    surf = Fg3dSurface();
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
    if (!surf.empty()) {
        if (surfs.find(currName) == surfs.end())
            surfs[currName] = surf;
        else
            surfs[currName].merge(surf);
    }
    mesh.name = fgPathToBase(fname);
    for (map<string,Fg3dSurface>::iterator it = surfs.begin(); it != surfs.end(); ++it) {
        Fg3dSurface &   srf = it->second;
        if (!srf.tris.valid() || !srf.quads.valid()) {
            srf.tris.uvInds.clear();
            srf.quads.uvInds.clear();
            fgout << fgnl << "WARNING: Partial UV indices ignored in " << fname << " surface " << it->first;
        }
        srf.name = it->first;
        mesh.surfaces.push_back(srf);
    }
    meshRef = mesh;
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
    FgOfstream &    ofs,
    const vector<FgMatrixC<uint,dim,1> > &  vertInds,
    const vector<FgMatrixC<uint,dim,1> > &  uvInds,
    Offsets         offsets)
{
    if (uvInds.size() == 0) {
        for (uint jj=0; jj<vertInds.size(); ++jj) {
            FgMatrixC<uint,dim,1>   vert = vertInds[jj];
            ofs << "f ";
            for (uint kk=0; kk<dim; ++kk) {
                uint        vi = vert[kk]+offsets.vert;
                ofs << vi << "//" << vi << " ";
            }
            ofs << endl;
        }
    }
    else {
        FGASSERT(vertInds.size() == uvInds.size());
        for (uint jj=0; jj<vertInds.size(); ++jj) {
            FgMatrixC<uint,dim,1>   vert = vertInds[jj];
            FgMatrixC<uint,dim,1>   uv = uvInds[jj];
            ofs << "f ";
            for (uint kk=0; kk<dim; ++kk) {
                uint        vi = vert[kk]+offsets.vert;
                ofs << vi << "/" << uv[kk]+offsets.uv << "/" << vi << " ";
            }
            ofs << endl;
        }
    }
}

static void
writeMtlBase(
    FgOfstream &    ofs,
    uint            idx)
{
    ofs << "newmtl " << "Texture" << fgToString(idx) << endl
        << "    illum 0\n"
        << "    Kd 0.7 0.7 0.7\n"
        << "    Ks 0 0 0\n"
        << "    Ka 0 0 0\n";
}

static
Offsets
writeMesh(
    FgOfstream &        ofs,
    FgOfstream &        ofsMtl,
    const Fg3dMesh &    mesh,
    const FgPath &      fpath,
    Offsets             offsets,
    const string &      imgFormat)
{
    if (!mesh.deltaMorphs.empty())
        fgout << endl << "WARNING: OBJ format does not support morphs";

    for (uint ii=0; ii<mesh.verts.size(); ++ii) {
        FgVect3F    vert = mesh.verts[ii];
        ofs << "v " << vert[0] << " " << vert[1] << " " << vert[2] << endl;
    }
    for (size_t ii=0; ii<mesh.uvs.size(); ++ii) {
        FgVect2F    uv = mesh.uvs[ii];
        ofs << "vt " << uv[0] << " " << uv[1] << endl;
    }
    Fg3dNormals         norms = fgNormals(mesh.surfaces,mesh.verts);
    for (size_t ii=0; ii<norms.vert.size(); ++ii) {
        FgVect3F    n = norms.vert[ii];
        ofs << "vn " << n[0] << " " << n[1] << " " << n[2] << endl;
    }
    for (uint tt=0; tt<mesh.texImages.size(); ++tt) {
        string  idxString = fgToString(offsets.mat+tt);
        // Some OBJ parsers (Meshlab) can't handle spaces in filename:
        FgString        imgName = fpath.base.replace(' ','_')+idxString+"."+imgFormat;
        fgSaveImgAnyFormat(fpath.dir()+imgName,mesh.texImages[tt]);
        if (ofsMtl) {
            writeMtlBase(ofsMtl,offsets.mat+tt);
            ofsMtl << "    map_Kd " << imgName << endl;
        }
    }
    for (size_t ii=0; ii<mesh.surfaces.size(); ++ii) {
        const Fg3dSurface & surf = mesh.surfaces[ii];
        FgString    name = (surf.name.empty() ? FgString("Surf")+fgToString(ii) : surf.name);
        ofs << "g " << name << endl;
        ofs << "usemtl Texture" << fgToString(offsets.mat+ii) << endl;
        writeFacets(ofs,surf.tris.vertInds,surf.tris.uvInds,offsets);
        writeFacets(ofs,surf.quads.vertInds,surf.quads.uvInds,offsets);
    }
    offsets.vert += uint(mesh.verts.size());
    offsets.uv += uint(mesh.uvs.size());
    offsets.mat += uint(mesh.texImages.size());
    return offsets;
}

void
fgSaveObj(
    const FgString &            filename,
    const vector<Fg3dMesh> &    meshes,
    string                      imgFormat)
{
    FgPath      fpath(filename);
    bool        texImage = false;
    for (size_t ii=0; ii<meshes.size(); ++ii)
        if (!meshes[ii].texImages.empty())
            texImage = true;
    FgOfstream  ofs(fpath.dirBase()+".obj");
    FgOfstream  ofsMtl;
    ofs.precision(7);
    ofs <<
        "# Wavefront OBJ format.\n"
        "# Generated by FaceGen, for more information visit http://FaceGen.com.\n";
    // Some OBJ parsers (MeshLab) can't handle spaces in filenames:
    if (texImage) {
        FgString    mtlBaseExt = fpath.base.replace(' ','_') + ".mtl";
        ofs << "mtllib " << mtlBaseExt << endl;
        ofsMtl.open(fpath.dir() + mtlBaseExt);
        writeMtlBase(ofsMtl,0);
    }
    Offsets     offsets(1,1,1);
    for (size_t ii=0; ii<meshes.size(); ++ii) {
        const Fg3dMesh &    mesh = meshes[ii];
        FgString            name = (mesh.name.empty() ? fpath.base : mesh.name);
        // Replace spaces with underscores in case some OBJ parsers can't handle that:
        name = name.replace(' ','_');
        ofs << "o " << name << endl
            << "s 1" << endl;    // Enable smooth shading
        offsets = writeMesh(ofs,ofsMtl,meshes[ii],fpath,offsets,imgFormat);
    }
}

// */
