//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Wavefront OBJ import / export
// 
// * This impl only supports polygonal geometry, no smooth surfaces or lines.
// * WOBJ does not support morphs, hierarchies, animation or units.
//

#include "stdafx.h"
#include "FgStdStream.hpp"
#include "FgImage.hpp"
#include "FgFileSystem.hpp"
#include "Fg3dMeshIo.hpp"
#include "Fg3dMesh.hpp"
#include "FgParse.hpp"
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
parseFloat(string const & str)
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
    bool &          homogeneous, // Set to true if there is a homogeneous coord (which is ignored)
    bool &          vertColors) // Set to true if there is a vertex color specified (which is ignored)
{
    Strings         nums = splitChar(str,' ');
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
    Vec3F           ret;
    for (uint ii=0; ii<3; ++ii)
        ret[ii] = parseFloat(nums[ii]);
    return ret;
}

static
Vec2F
parseUv(string const & str)
{
    Strings  nums = splitChar(str,' ');
    // A third homogeneous coord value can also be specified but is only used for rational
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
    string const &      str,
    size_t              numVerts,
    size_t              numUvs,
    FacetInds<3> &    tris,
    FacetInds<4> &    quads)
{
    bool            ret = false;
    Strings  strs = splitChar(str,' ');
    FGASSERT(strs.size() > 2);
    vector<Uints >   nums;
    for (size_t ii=0; ii<strs.size(); ++ii) {
        Strings  ns = splitChar(strs[ii],'/',true);
        size_t          sz = cMin(ns.size(),size_t(2));    // Ignore normal indices
        Uints    nms;
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
        tris.posInds.push_back(Vec3UI(nums[0][0],nums[1][0],nums[2][0]));
        if (nums[0].size() > 1)
            tris.uvInds.push_back(Vec3UI(nums[0][1],nums[1][1],nums[2][1]));
    }
    if (nums.size() == 4) {
        quads.posInds.push_back(Vec4UI(nums[0][0],nums[1][0],nums[2][0],nums[3][0]));
        if (nums[0].size() > 1)
            quads.uvInds.push_back(Vec4UI(nums[0][1],nums[1][1],nums[2][1],nums[3][1]));
    }
    if (nums.size() > 4) {          // N-gon
        for (size_t ii=0; ii<nums.size()-2; ++ii) {
            tris.posInds.push_back(Vec3UI(nums[0][0],nums[ii+1][0],nums[ii+2][0]));
            if (nums[ii].size() > 1)
                tris.uvInds.push_back(Vec3UI(nums[0][1],nums[ii+1][1],nums[ii+2][1]));
        }
        ret = true;
    }
    return ret;
}

Surfs
splitByUvDomain(Surf const & surf,Vec2Fs const & uvs,String8 const & baseName)
{
    set<Vec2I>              domains;
    map<Vec2I,Uints>        domainToQuadInds,
                            domainToTriInds;
    bool                    mixed = false;
    for (uint tt=0; tt<surf.tris.uvInds.size(); ++tt) {
        Vec3UI                  uvInds = surf.tris.uvInds[tt];
        Vec2I                   domain(mapFloor(uvs[uvInds[0]]));
        domains.insert(domain);
        for (uint ii=1; ii<3; ++ii)
            if (Vec2I(mapFloor(uvs[uvInds[ii]])) != domain)
                mixed = true;
        domainToTriInds[domain].push_back(tt);
    }
    for (uint qq=0; qq<surf.quads.uvInds.size(); ++qq) {
        Vec4UI                  uvInds = surf.quads.uvInds[qq];
        Vec2I                   domain(mapFloor(uvs[uvInds[0]]));
        domains.insert(domain);
        for (uint ii=1; ii<4; ++ii)
            if (Vec2I(mapFloor(uvs[uvInds[ii]])) != domain)
                mixed = true;
        domainToQuadInds[domain].push_back(qq);
    }
    if (domains.size() > 1)
        fgout << "WARNING: OBJ UV domains detected and converted to " << domains.size() << " surfaces: ";
    if (mixed)
        fgout << "WARNING: some facet(s) span multiple UV domains";
    Surfs                   ret; ret.reserve(domains.size());
    for (Vec2I domain : domains) {
        fgout << domain << " ";
        Uints const &           triSels = domainToTriInds[domain];
        Tris                    tris {
            permute(surf.tris.posInds,triSels),
            permute(surf.tris.uvInds,triSels),
        };
        Uints const &           quadSels = domainToQuadInds[domain];
        Quads                   quads {
            permute(surf.quads.posInds,quadSels),
            permute(surf.quads.uvInds,quadSels),
        };
        String8                 name = baseName;
        if (domains.size() > 1)
            name += "-" + toStr(ret.size());
        ret.emplace_back(name,tris,quads);
    }
    return ret;
}

Mesh
loadWObj(String8 const & fname)
{
    Mesh                mesh;
    map<string,Surf>    surfMap;
    Strings             lines = splitLines(loadRaw(fname));   // Removes empty lines
    Surf                currSurf;
    string              currName;
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
                surfMap[currName].merge(currSurf);
            currSurf = Surf{};
        }
    };
    for (size_t ii=0; ii<lines.size(); ++ii) {
        try {
            string const &  line = lines[ii];
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
                if (parseFacet(line.substr(2),mesh.verts.size(),mesh.uvs.size(),currSurf.tris,currSurf.quads))
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
            fgout << fgnl << "WARNING: Error in line " << ii+1 << " of " << fname << ": " << e.tr_message() << fgpush
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
            surfMap[currName].merge(currSurf);
    }
    mesh.name = pathToBase(fname);
    Surfs               labelledSurfs;
    for (map<string,Surf>::iterator it = surfMap.begin(); it != surfMap.end(); ++it) {
        Surf &   srf = it->second;
        if (!srf.tris.valid() || !srf.quads.valid()) {
            srf.tris.uvInds.clear();
            srf.quads.uvInds.clear();
            fgout << fgnl << "WARNING: Partial UV indices ignored in " << fname << " surface " << it->first;
        }
        srf.name = it->first;
        labelledSurfs.push_back(srf);
    }
    // Important to remove domain info if present since this is WOBJ-specific:
    Mat22F          uvBounds = cBounds(mesh.uvs);
    Vec2F           uvBnds = {cMin(uvBounds.m),cMax(uvBounds.m)};
    if ((uvBnds[0] < 0.0f) || (uvBnds[1] > 1.0f)) {
        // WOBJs can actually use UV domains in combination with 'o', 'g', 's' or 'usemtl' elements (eg. Reallusion):
        for (Surf const & surf : labelledSurfs)
            cat_(mesh.surfaces,splitByUvDomain(surf,mesh.uvs,surf.name));
        fgout << fgnl << "WARNING: OBJ UVs folded into [0,1) from " << uvBnds;
        for (Vec2F & uv : mesh.uvs)
            uv -= mapFloor(uv);
    }
    else
        mesh.surfaces = labelledSurfs;
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
    ofs << "newmtl " << "Texture" << toStr(idx) << "\n"
        "    illum 0\n"
        "    Kd 0.7 0.7 0.7\n"
        "    Ks 0 0 0\n"
        "    Ka 0 0 0\n";
}

static
Offsets
writeMesh(
    Ofstream &          ofs,
    Ofstream &          ofsMtl,
    Mesh const &        mesh,
    Path const &        fpath,
    Offsets             offsets,
    string const &      imgFormat,
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
    MeshNormals         norms = cNormals(mesh.surfaces,mesh.verts);
    for (size_t ii=0; ii<norms.vert.size(); ++ii) {
        Vec3F    n = norms.vert[ii];
        ofs << "vn " << n[0] << " " << n[1] << " " << n[2] << "\n";
    }
    for (uint tt=0; tt<mesh.surfaces.size(); ++tt) {
        if (mesh.surfaces[tt].material.albedoMap) {
            string  idxString = toStr(offsets.mat+tt);
            // Some OBJ parsers (Meshlab) can't handle spaces in filename:
            String8        imgName = fpath.base.replace(' ','_')+idxString+"."+imgFormat;
            saveImage(fpath.dir()+imgName,*mesh.surfaces[tt].material.albedoMap);
            if (ofsMtl) {
                writeMtlBase(ofsMtl,offsets.mat+tt);
                ofsMtl << "    map_Kd " << imgName << "\n";
            }
        }
    }
    for (size_t ii=0; ii<mesh.surfaces.size(); ++ii) {
        Surf const & surf = mesh.surfaces[ii];
        String8    name = (surf.name.empty() ? String8("Surf")+toStr(ii) : surf.name);
        ofs << "g " << name << "\n";
        if (mtlFile)    // Meshlab can't handle 'usemtl' if there is no MTL file:
            ofs << "usemtl Texture" << toStr(offsets.mat+ii) << "\n";
        writeFacets(ofs,surf.tris.posInds,surf.tris.uvInds,offsets);
        writeFacets(ofs,surf.quads.posInds,surf.quads.uvInds,offsets);
    }
    offsets.vert += uint(mesh.verts.size());
    offsets.uv += uint(mesh.uvs.size());
    offsets.mat += uint(mesh.surfaces.size());
    return offsets;
}

void
saveWObj(
    String8 const &         filename,
    Meshes const &          meshes,
    string                  imgFormat)
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

void
fgSaveObjTest(CLArgs const & args)
{
    FGTESTDIR
    String8         dd = dataDir();
    string          rd = "base/";
    Mesh            mouth = loadTri(dd+rd+"Mouth.tri");
    mouth.deltaMorphs.clear();       // Remove morphs to avoid warning
    mouth.targetMorphs.clear();
    mouth.surfaces[0].setAlbedoMap(loadImage(dd+rd+"MouthSmall.png"));
    Mesh            glasses = loadTri(dd+rd+"Glasses.tri");
    glasses.surfaces[0].setAlbedoMap(loadImage(dd+rd+"Glasses.tga"));
    saveWObj("meshExportObj.obj",svec(mouth,glasses));
    regressFileRel("meshExportObj.obj","base/test/");
    regressFileRel("meshExportObj.mtl","base/test/");
    regressFileRel("meshExportObj1.png","base/test/");
    regressFileRel("meshExportObj2.png","base/test/");
}

}

// */
