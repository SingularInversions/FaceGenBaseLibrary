//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "Fg3dMesh.hpp"
#include "FgMath.hpp"
#include "FgAffine.hpp"
#include "FgBounds.hpp"
#include "FgImageDraw.hpp"
#include "FgBestN.hpp"
#include "FgGridTriangles.hpp"
#include "FgGeometry.hpp"
#include "Fg3dCamera.hpp"

using namespace std;

namespace Fg {

Mesh
meshFromImage(const ImgD & img)
{
    Mesh                ret;
    FGASSERT((img.height() > 1) && (img.width() > 1));
    Mat22D                imgIdxBounds(0,img.width()-1,0,img.height()-1);
    AffineEw2D            imgIdxToSpace(imgIdxBounds,Mat22D(0,1,0,1)),
                            imgIdxToOtcs(imgIdxBounds,Mat22D(0,1,1,0));
    Affine1D              imgValToSpace(cBounds(img.dataVec()),VecD2(0,1));
    for (Iter2UI it(img.dims()); it.valid(); it.next()) {
        Vec2D            imgCrd = Vec2D(it()),
                            xy = imgIdxToSpace * imgCrd;
        ret.verts.push_back(Vec3F(xy[0],xy[1],imgValToSpace * img[it()]));
        ret.uvs.push_back(Vec2F(imgIdxToOtcs * imgCrd));
    }
    std::vector<Vec4UI>  quads,
                            texInds;
    uint                    w = img.width();
    for (Iter2UI it(img.dims()-Vec2UI(1)); it.valid(); it.next()) {
        uint                x = it()[0],
                            x1 = x + 1,
                            y = it()[1] * w,
                            y1 = y + w;
        Vec4UI           inds(x1+y,x+y,x+y1,x1+y1);      // CC winding
        quads.push_back(inds);
        texInds.push_back(inds);
    }
    ret.surfaces.push_back(Surf(quads,texInds));
    return ret;
}

QuadSurf
cGrid(size_t szll)
{
    FGASSERT(szll > 0);
    uint                sz = scast<uint>(szll);
    QuadSurf            ret;
    AffineEw2D          itToCoord {Mat22D(0,sz,0,sz),Mat22D{-1,1,-1,1}};
    for (Iter2UI it(sz+1); it.valid(); it.next()) {
        Vec2D           p = itToCoord * Vec2D(it());
        ret.verts.push_back(Vec3F(p[0],p[1],0));
    }
    for (Iter2UI it(sz); it.valid(); it.next()) {
        uint            i0 = it()[0],
                        i1 = it()[1]*(sz+1),
                        v0 = i0+i1,
                        v1 = i0+i1+1,
                        v2 = i0+i1+1+(sz+1),
                        v3 = i0+i1+(sz+1);
        ret.quads.push_back(Vec4UI(v0,v1,v2,v3));
    }
    return ret;
}

TriSurf
cSphere4(size_t subdivisions)
{
    FGASSERT(subdivisions < 10);    // sanity check
    TriSurf             tet = cTetrahedron();
    for (uint ss=0; ss<subdivisions; ss++) {
        tet = subdivide(tet,true);
        for (Vec3F & v : tet.verts)
            normalize_(v);
    }
    return tet;
}

TriSurf
cSphere(size_t subdivisions)
{
    FGASSERT(subdivisions < 8);    // sanity check
    TriSurf             ico = cIcosahedron();
    for (size_t ii=0; ii<subdivisions; ++ii) {
        ico = subdivide(ico,true);
        for (Vec3F & v : ico.verts)
            normalize_(v);
    }
    return ico;
}

Mesh
removeDuplicateFacets(Mesh const & mesh)
{
    Mesh    ret = mesh;
    for (size_t ss=0; ss<ret.surfaces.size(); ++ss) {
        ret.surfaces[ss] = removeDuplicateFacets(ret.surfaces[ss]);
    }
    return ret;
}

Mesh
removeUnusedVerts(Mesh const & mesh)
{
    Mesh            ret;
    ret.name = mesh.name;
    ret.surfaces = mesh.surfaces;
    // Which vertices & uvs are referenced by a surface or marked vertex:
    vector<bool>        vertUsed(mesh.verts.size(),false);
    vector<bool>        uvsUsed(mesh.uvs.size(),false);
    for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
        Surf const & surf = mesh.surfaces[ss];
        for (size_t tt=0; tt<surf.tris.size(); ++tt) {
            Vec3UI   v = surf.tris.posInds[tt];
            for (uint ii=0; ii<3; ++ii)
                vertUsed[v[ii]] = true;
        }
        for (size_t tt=0; tt<surf.quads.size(); ++tt) {
            Vec4UI   v = surf.quads.posInds[tt];
            for (uint ii=0; ii<4; ++ii)
                vertUsed[v[ii]] = true;
        }
        for (size_t tt=0; tt<surf.tris.uvInds.size(); ++tt) {
            Vec3UI   u = surf.tris.uvInds[tt];
            for (uint ii=0; ii<3; ++ii)
                uvsUsed[u[ii]] = true;
        }
        for (size_t tt=0; tt<surf.quads.uvInds.size(); ++tt) {
            Vec4UI   u = surf.quads.uvInds[tt];
            for (uint ii=0; ii<4; ++ii)
                uvsUsed[u[ii]] = true;
        }
    }
    for (size_t ii=0; ii<mesh.markedVerts.size(); ++ii)
        vertUsed[mesh.markedVerts[ii].idx] = true;
    // Create the new vertex list:
    Uints    mapVerts(mesh.verts.size(),numeric_limits<uint>::max());
    uint            cnt = 0;
    for (size_t ii=0; ii<vertUsed.size(); ++ii) {
        if (vertUsed[ii]) {
            ret.verts.push_back(mesh.verts[ii]);
            mapVerts[ii] = cnt++;
        }
    }
    // Create the new uv list:
    Uints    mapUvs(mesh.uvs.size(),numeric_limits<uint>::max());
    cnt = 0;
    for (size_t ii=0; ii<uvsUsed.size(); ++ii) {
        if (uvsUsed[ii]) {
            ret.uvs.push_back(mesh.uvs[ii]);
            mapUvs[ii] = cnt++;
        }
    }
    // Remap the surfaces and marked verts, which we know all map to valid indices in the new list:
    for (size_t ss=0; ss<ret.surfaces.size(); ++ss) {
        Surf &           surf = ret.surfaces[ss];
        for (size_t ii=0; ii<surf.tris.size(); ++ii)
            for (uint jj=0; jj<3; ++jj)
                surf.tris.posInds[ii][jj] = mapVerts[surf.tris.posInds[ii][jj]];
        for (size_t ii=0; ii<surf.quads.size(); ++ii)
            for (uint jj=0; jj<4; ++jj)
                surf.quads.posInds[ii][jj] = mapVerts[surf.quads.posInds[ii][jj]];
        for (size_t ii=0; ii<surf.tris.uvInds.size(); ++ii)
            for (uint jj=0; jj<3; ++jj)
                surf.tris.uvInds[ii][jj] = mapUvs[surf.tris.uvInds[ii][jj]];
        for (size_t ii=0; ii<surf.quads.uvInds.size(); ++ii)
            for (uint jj=0; jj<4; ++jj)
                surf.quads.uvInds[ii][jj] = mapUvs[surf.quads.uvInds[ii][jj]];
    }
    ret.markedVerts = mesh.markedVerts;
    for (size_t ii=0; ii<ret.markedVerts.size(); ++ii)
        ret.markedVerts[ii].idx = mapVerts[ret.markedVerts[ii].idx];
    // Remap only those delta morphs which contain non-zero deltas:
    Vec3F            zero(0);
    for (size_t ii=0; ii<mesh.deltaMorphs.size(); ++ii) {
        Morph const & src = mesh.deltaMorphs[ii];
        Morph         dst;
        dst.name = src.name;
        bool            keep = false;
        for (size_t jj=0; jj<src.verts.size(); ++jj) {
            if (mapVerts[jj] != numeric_limits<uint>::max()) {
                dst.verts.push_back(src.verts[jj]);
                if (src.verts[jj] != zero)
                    keep = true;
            }
        }
        if (keep)
            ret.deltaMorphs.push_back(dst);
    }
    // Remap only those target morphs that reference retained vertices:
    for (size_t ii=0; ii<mesh.targetMorphs.size(); ++ii) {
        const IndexedMorph &      src = mesh.targetMorphs[ii];
        IndexedMorph              dst;
        dst.name = src.name;
        for (size_t jj=0; jj<src.ivs.size(); ++jj) {
            uint        idx = mapVerts[src.ivs[jj].idx];
            if (idx != numeric_limits<uint>::max())
                dst.ivs.emplace_back(idx,src.ivs[jj].vec);
        }
        if (!dst.ivs.empty())
            ret.targetMorphs.push_back(dst);
    }
    return  ret;
}

TriSurf
cTetrahedron(bool open)
{
    // Coordinates of a regular tetrahedron with edges of length 2*sqrt(2):
    Vec3Fs             verts {
        { 1.0f, 1.0f, 1.0f},
        {-1.0f,-1.0f, 1.0f},
        {-1.0f, 1.0f,-1.0f},
        { 1.0f,-1.0f,-1.0f},
    };
    Vec3UIs             tris {
        {0,1,3},
        {0,2,1},
        {2,0,3},
    };
    if (!open)
        tris.emplace_back(1,2,3);
    return TriSurf {verts,tris};
}

Mesh
cPyramid(bool open)
{
    Vec3Fs         verts {
        {-1, 0,-1},
        { 1, 0,-1},
        {-1, 0, 1},
        { 1, 0, 1},
        { 0, 1, 0},
    };
    Vec3UIs         tris {
        {0, 4, 1},
        {0, 2, 4},
        {2, 3, 4},
        {1, 4, 3},
    };
    if (!open) {
        tris.emplace_back(0,1,3);
        tris.emplace_back(3,2,0);
    }
    return Mesh{verts,{Surf{tris}}};
}

Mesh
c3dCube(bool open)
{
    Vec3Fs             verts;
    for (uint vv=0; vv<8; ++vv)
        verts.push_back(
            Vec3F(
                float(vv & 0x01),
                float((vv >> 1) & 0x01),
                float((vv >> 2) & 0x01)) * 2.0f -
            Vec3F(1.0f));
    Vec3UIs   tris;
    // X planes:
    tris.push_back(Vec3UI(0,4,6));
    tris.push_back(Vec3UI(6,2,0));
    tris.push_back(Vec3UI(5,1,3));
    tris.push_back(Vec3UI(3,7,5));
    // Z planes:
    tris.push_back(Vec3UI(1,0,2));
    tris.push_back(Vec3UI(2,3,1));
    tris.push_back(Vec3UI(4,5,7));
    tris.push_back(Vec3UI(7,6,4));
    // Y planes:
    tris.push_back(Vec3UI(0,1,5));
    tris.push_back(Vec3UI(5,4,0));
    if (!open) {
        tris.push_back(Vec3UI(3,2,6));
        tris.push_back(Vec3UI(6,7,3));
    }
    return Mesh{verts,{Surf{tris}}};
}

TriSurf
cOctahedron()
{
    // Visualize as a diamond with opposite vertices axially aligned:
    return TriSurf {
    {
        {-1, 0, 0},
        { 1, 0, 0},
        { 0,-1, 0},
        { 0, 1, 0},
        { 0, 0,-1},
        { 0, 0, 1},
    },
    {
        {0,2,5},
        {0,3,4},
        {0,4,2},
        {0,5,3},
        {1,2,4},
        {1,3,5},
        {1,4,3},
        {1,5,2},
    },
    };
}

TriSurf
cIcosahedron()
{
    // Data copied from github cginternals:
    float const         t = 0.5f * (1.0f + sqrt(5.0f)),
                        i = 1.0f / sqrt(sqr(t) + 1.0f),
                        a = t * i;
    return TriSurf
    {
        {
            // sqr(i) + sqr(a) = 1
            {-i, a, 0},
            { i, a, 0},
            {-i,-a, 0},
            { i,-a, 0},
            { 0,-i, a},
            { 0, i, a},
            { 0,-i,-a},
            { 0, i,-a},
            { a, 0,-i},
            { a, 0, i},
            {-a, 0,-i},
            {-a, 0, i},
        },
        {
            { 0,11, 5},
            { 0, 5, 1},
            { 0, 1, 7},
            { 0, 7,10},
            { 0,10,11},
            { 1, 5, 9},
            { 5,11, 4},
            {11,10, 2},
            {10, 7, 6},
            { 7, 1, 8},
            { 3, 9, 4},
            { 3, 4, 2},
            { 3, 2, 6},
            { 3, 6, 8},
            { 3, 8, 9},
            { 4, 9, 5},
            { 2, 4,11},
            { 6, 2,10},
            { 8, 6, 7},
            { 9, 8, 1},
        },
    };
}

Mesh
cNTent(uint nn)
{
    FGASSERT(nn > 2);
    Vec3Fs             verts;
    verts.push_back(Vec3F(0.0f,1.0f,0.0f));
    float   step = 2.0f * float(pi()) / float(nn);
    for (uint ii=0; ii<nn; ++ii) {
        float   angle = step * float(ii);
        verts.push_back(Vec3F(cos(angle),0.0f,sin(angle)));
    }
    Vec3UIs   tris;
    for (uint ii=0; ii<nn; ++ii)
        tris.push_back(Vec3UI(0,ii+1,((ii+1)%nn)+1));
    return Mesh{verts,{Surf{tris}}};
}

//Mesh
//fgFddCage(float size,float thick)
//{
//    Mesh            ret;
//    Vec3Fs    sqrVerts;
//    sqrVerts.push_back(Vec3F(-thick,-thick,0));
//    sqrVerts.push_back(Vec3F(-thick,thick,0));
//    sqrVerts.push_back(Vec3F(thick,thick,0));
//    sqrVerts.push_back(Vec3F(thick,-thick,0));
//    Vec4UI           sqrInds(0,1,2,3);
//    //cat_(ret.verts,mapCall(sqrVerts
//    for (uint axis=0; axis<3; ++axis) {
//        Vec3F    l(0);
//        for (int aa=-1; aa<2; aa+=2)
//            for (int bb=
//    }
//}

Mesh
mergeSameNameSurfaces(Mesh const & in)
{
    Mesh            ret = in;
    Surfs surfs;
    for (size_t ss=0; ss<in.surfaces.size(); ++ss) {
        Surf const & surf = in.surfaces[ss];
        bool    found = false;
        for (size_t ii=0; ii<surfs.size(); ++ii) {
            if (surfs[ii].name == surf.name) {
                surfs[ii].merge(surf);
                found = true;
                continue;
            }
        }
        if (!found)
            surfs.push_back(surf);
    }
    ret.surfaces = surfs;
    fgout << fgnl << "Merged " << in.surfaces.size() << " into " << surfs.size() << ".";
    return ret;
}

Mesh
fuseIdenticalVerts(Mesh const & mesh)
{
    Mesh            ret;
    ret.name = mesh.name;
    ret.uvs = mesh.uvs;
    ret.joints = mesh.joints;
    ret.jointDofs = mesh.jointDofs;
    Sizes           map; map.reserve(mesh.verts.size());
    for (Vec3F const & vert : mesh.verts) {
        size_t          idx = findFirstIdx(ret.verts,vert);
        if (idx < ret.verts.size())         // duplicate found
            map.push_back(idx);
        else {
            map.push_back(ret.verts.size());
            ret.verts.push_back(vert);
        }
    }
    for (Surf surf : mesh.surfaces) {
        for (Vec3UI & tri : surf.tris.posInds)
            for (uint & idx : tri.m)
                idx = map[idx];
        for (Vec4UI & quad : surf.quads.posInds)
            for (uint & idx : quad.m)
                idx = map[idx];
        ret.surfaces.push_back(surf);
    }
    return ret;
}

Mesh
fuseIdenticalUvs(Mesh const & in)
{
    Mesh            ret = in;
    ret.uvs.clear();
    Sizes           map; map.reserve(in.uvs.size());
    for (Vec2F const & uv : in.uvs) {
        size_t          idx = findFirstIdx(ret.uvs,uv);
        if (idx < ret.uvs.size())         // duplicate found
            map.push_back(idx);
        else {
            map.push_back(ret.uvs.size());
            ret.uvs.push_back(uv);
        }
    }
    for (Surf & surf : ret.surfaces) {
        for (Vec3UI & tri : surf.tris.uvInds)
            for (uint & idx : tri.m)
                idx = map[idx];
        for (Vec4UI & quad : surf.quads.uvInds)
            for (uint & idx : quad.m)
                idx = map[idx];
    }
    return ret;
}

// Warning: This function will yield stack overflow for large meshes unless you
// increase your stack size:
static
void
traverse(
    const vector<Uints > &   uvToQuadsIndex,
    Uints &              colourMap,
    const vector<Vec4UI> &   uvInds,
    uint                        colour,
    uint                        quadIdx)
{
    FGASSERT(colour > 0);
    colourMap[quadIdx] = colour;
    for (uint ii=0; ii<4; ++ii) {
        uint                    uvIdx = uvInds[quadIdx][ii];
        const Uints &    quadInds = uvToQuadsIndex[uvIdx];
        for (size_t jj=0; jj<quadInds.size(); ++jj)
            if (colourMap[quadInds[jj]] == 0)
                traverse(uvToQuadsIndex,colourMap,uvInds,colour,quadInds[jj]);
    }
}

Mesh
splitSurfsByUvContiguous(Mesh const & in)
{
    Mesh                ret,
                        mesh = in;
    mesh.surfaces = {mergeSurfaces(mesh.surfaces)};
    Surf const &        surf = mesh.surfaces[0];
    if (!surf.tris.uvInds.empty())
        fgThrow("Tris not currently supported for this operation (quads only)");
    Vec4UIs const     & uvInds = surf.quads.uvInds,
                      & quadInds = surf.quads.posInds;
    Uintss              uvToQuadsIndex(mesh.uvs.size());
    for (size_t ii=0; ii<uvInds.size(); ++ii)
        for (uint jj=0; jj<4; ++jj)
            uvToQuadsIndex[uvInds[ii][jj]].push_back(uint(ii));
    Uints               colourMap(uvInds.size(),0);
    uint                numColours = 0;
    for (size_t ii=0; ii<colourMap.size(); ++ii)
        if (colourMap[ii] == 0)
            traverse(uvToQuadsIndex,colourMap,uvInds,++numColours,uint(ii));
    Vec4UIss            newVertInds(numColours),
                        newUvInds(numColours);
    for (size_t ii=0; ii<colourMap.size(); ++ii) {
        newVertInds[colourMap[ii]-1].push_back(quadInds[ii]);
        newUvInds[colourMap[ii]-1].push_back(uvInds[ii]);
    }
    ret.verts = mesh.verts;
    ret.uvs = mesh.uvs;
    for (size_t ii=0; ii<numColours; ++ii)
        ret.surfaces.push_back(Surf(newVertInds[ii],newUvInds[ii]));
    fgout << fgnl << numColours << " separate UV-contiguous surfaces created";
    return ret;
}

Mesh            mergeMeshes(Ptrs<Mesh> meshPtrs)
{
    Mesh                ret;
    Ptrs<String8>       namePtrs = sliceMemberPP(meshPtrs,&Mesh::name);
    Ptrs<String8>       neNamePtrs = cFilter(namePtrs,[](String8 const * n){return !n->empty(); });
    ret.name = catDeref(neNamePtrs,"+");
    Ptrs<Vec3Fs>        vertsPtrs = sliceMemberPP(meshPtrs,&Mesh::verts);
    Sizes               voffsets = integrate(cSizes(vertsPtrs));
    ret.verts = catDeref(vertsPtrs);
    Ptrs<Vec2Fs>        uvsPtrs = sliceMemberPP(meshPtrs,&Mesh::uvs);
    Sizes               uoffsets = integrate(cSizes(uvsPtrs));
    ret.uvs = catDeref(uvsPtrs);
    Ptrs<Surfs>         surfsPtrs = sliceMemberPP(meshPtrs,&Mesh::surfaces);
    Sizes               soffsets = integrate(cSizes(surfsPtrs));
    ret.surfaces.reserve(soffsets.back());
    map<String8,IdxVec3Fs>  deltaMorphs,
                            targMorphs;
    for (size_t mm=0; mm<meshPtrs.size(); ++mm) {
        Mesh const &        mesh = *meshPtrs[mm];
        uint                voffset = voffsets[mm],
                            uoffset = uoffsets[mm];
        String8 const &     meshName = mesh.name;
        for (Surf const & surf : mesh.surfaces)
            ret.surfaces.emplace_back(
                meshName + surf.name,
                offsetIndices(surf.tris,voffset,uoffset),
                offsetIndices(surf.quads,voffset,uoffset),
                surf.surfPoints,
                surf.material
            );
        for (Morph const & m : mesh.deltaMorphs) {
            IndexedMorph        im = deltaToIndexedMorph(m);
            auto                it = deltaMorphs.find(im.name);
            if (it == deltaMorphs.end())
                deltaMorphs[im.name] = im.ivs;
            else
                cat_(it->second,offsetIndices(im.ivs,voffsets[mm]));
        }
        for (IndexedMorph const & m : mesh.targetMorphs) {
            auto                it = targMorphs.find(m.name);
            if (it == targMorphs.end())
                targMorphs[m.name] = m.ivs;
            else
                cat_(it->second,offsetIndices(m.ivs,voffsets[mm]));
        }
        for (MarkedVert const & mv : mesh.markedVerts)
            ret.markedVerts.emplace_back(mv.idx + voffset,mv.label);
    }
    ret.deltaMorphs.reserve(deltaMorphs.size());
    for (auto const & dm : deltaMorphs)
        ret.deltaMorphs.emplace_back(dm.first,indexedToCorrMorph(dm.second,ret.verts.size()));
    ret.targetMorphs.reserve(targMorphs.size());
    for (auto const & tm : targMorphs)
        ret.targetMorphs.emplace_back(tm.first,tm.second);
    return ret;
}

Mesh
fg3dMaskFromUvs(Mesh const & mesh,const Img<FatBool> & mask)
{
    // Make a list of which vertices have UVs that only fall in the excluded regions:
    vector<FatBool>      keep(mesh.verts.size(),false);
    AffineEw2F          otcsToIpcs = cOtcsToIpcs(mask.dims());
    Mat22UI             clampVal(0,mask.width()-1,0,mask.height()-1);
    for (size_t ii=0; ii<mesh.surfaces.size(); ++ii) {
        Surf const & surf = mesh.surfaces[ii];
        if (!surf.quads.uvInds.empty())
            fgThrow("Quads not supported");
        if (surf.tris.uvInds.empty())
            fgThrow("No tri facet UVs");
        for (size_t jj=0; jj<surf.tris.uvInds.size(); ++jj) {
            Vec3UI   uvInd = surf.tris.uvInds[jj];
            Vec3UI   vtInd = surf.tris.posInds[jj];
            for (uint kk=0; kk<3; ++kk) {
                bool    valid = mask[mapClamp(Vec2UI(otcsToIpcs * mesh.uvs[uvInd[kk]]),clampVal)];
                keep[vtInd[kk]] = keep[vtInd[kk]] || valid;
            }
        }
    }
    // Remove the facets that use those vertices:
    Surfs nsurfs;
    for (size_t ii=0; ii<mesh.surfaces.size(); ++ii) {
        Surf const & surf = mesh.surfaces[ii];
        Surf         nsurf;
        for (size_t jj=0; jj<surf.tris.uvInds.size(); ++jj) {
            Vec3UI   vtInd = surf.tris.posInds[jj];
            bool copy = false;
            for (uint kk=0; kk<3; ++kk)
                copy = copy || keep[vtInd[kk]];
            if (copy)
                nsurf.tris.posInds.push_back(vtInd);
        }
        nsurfs.push_back(nsurf);
    }
    // Remove unused vertices:
    return removeUnusedVerts(Mesh(mesh.verts,nsurfs));
}

ImgUC
getUvCover(Mesh const & mesh,Vec2UI dims)
{
    ImgUC                 ret(dims,uchar(0));
    Vec3UIs              tris = mesh.getTriEquivs().uvInds;
    GridTriangles         grid = gridTriangles(mesh.uvs,tris);
    AffineEw2F            ircsToOtcs(
        Mat22F(-0.5f,dims[0]-0.5f,-0.5f,dims[1]-0.5f),
        Mat22F(0,1,1,0));
    for (Iter2UI it(dims); it.valid(); it.next())
        if (!grid.intersects(tris,mesh.uvs,ircsToOtcs * Vec2F(it())).empty())
            ret[it()] = uchar(255);
    return ret;
}

ImgRgba8s
cUvWireframeImages(Mesh const & mesh,Rgba8 wireColor)
{
    ImgRgba8s           ret; ret.reserve(mesh.surfaces.size());
    for (Surf const & surf : mesh.surfaces)
        ret.push_back(cUvWireframeImage(mesh.uvs,surf.tris.uvInds,surf.quads.uvInds,wireColor));
    return ret;
}

Vec3Fs
embossMesh(Mesh const & mesh,const ImgUC & logoImg,double val)
{
    Vec3Fs         ret;
    // Don't check for UV seams, just let the emboss value be the last one traversed:
    Vec3Fs         deltas(mesh.verts.size());
    vector<size_t>  embossedVertInds;
    MeshNormals     norms = cNormals(mesh);
    for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
        Surf const &     surf = mesh.surfaces[ss];
        for (size_t ii=0; ii<surf.numTris(); ++ii) {
            Vec3UI   uvInds = surf.tris.uvInds[ii],
                        vtInds = surf.tris.posInds[ii];
            for (uint jj=0; jj<3; ++jj) {
                Vec2F           uv = mesh.uvs[uvInds[jj]];
                uv[1] = 1.0f - uv[1];       // Convert from OTCS to IUCS
                float           imgSamp = sampleClipIucs(logoImg,uv) / 255.0f;
                uint            vtIdx = vtInds[jj];
                deltas[vtIdx] = norms.vert[vtIdx] * imgSamp;
                if (imgSamp > 0)
                    embossedVertInds.push_back(vtIdx);
            }
        }
        for (size_t ii=0; ii<surf.numQuads(); ++ii) {
            Vec4UI   uvInds = surf.quads.uvInds[ii],
                        vtInds = surf.quads.posInds[ii];
            for (uint jj=0; jj<4; ++jj) {
                Vec2F           uv = mesh.uvs[uvInds[jj]];
                uv[1] = 1.0f - uv[1];       // Convert from OTCS to IUCS
                float           imgSamp = sampleClipIucs(logoImg,uv) / 255.0f;
                uint            vtIdx = vtInds[jj];
                deltas[vtIdx] = norms.vert[vtIdx] * imgSamp;
                if (imgSamp > 0)
                    embossedVertInds.push_back(vtIdx);
            }
        }
    }
    float       fac = cMaxElem(cDims(permute(mesh.verts,embossedVertInds))) * val;
    ret.resize(mesh.verts.size());
    for (size_t ii=0; ii<deltas.size(); ++ii)
        ret[ii] = mesh.verts[ii] + deltas[ii] * fac;
    return ret;
}

Vec3Fs
poseMesh(Mesh const & mesh,MorphVals const & expression)
{
    Vec3Fs          ret;
    Floats          coord(mesh.numMorphs(),0);
    for (size_t ii=0; ii<expression.size(); ++ii) {
        Valid<size_t>     idx = mesh.findMorph(expression[ii].name);
        if (idx.valid())
            coord[idx.val()] = expression[ii].val;
    }
    mesh.morph(coord,ret);
    return ret;
}

void
surfPointsToMarkedVerts_(Mesh const & in,Mesh & out)
{
    for (size_t ii=0; ii<in.surfaces.size(); ++ii) {
        Surf const &     surf = in.surfaces[ii];
        for (size_t jj=0; jj<surf.surfPoints.size(); ++jj) {
            Vec3F        pos = surf.surfPointPos(in.verts,jj);
            out.addMarkedVert(pos,surf.surfPoints[jj].label);
        }
    }
}

TriSurf
cMirror(TriSurf const & ts,uint axis)
{
    FGASSERT(axis < 3);
    TriSurf         ret;
    ret.verts = ts.verts;
    for (Vec3F & vert : ret.verts)
        vert[axis] *= -1.0f;
    ret.tris = ts.tris;
    for (Vec3UI & tri : ret.tris)
        swap(tri[1],tri[2]);
    return ret;
}

Mesh
cMirror(Mesh const & m,uint axis)
{
    FGASSERT(axis < 3);
    Mesh        ret;
    ret.verts = m.verts;
    for (Vec3F & vert : ret.verts)
        vert[axis] *= -1.0f;
    ret.surfaces = m.surfaces;
    for (Surf & surf : ret.surfaces) {
        for (Vec3UI & t : surf.tris.posInds)
            swap(t[1],t[2]);
        for (Vec3UI & t : surf.tris.uvInds)
            swap(t[1],t[2]);
        for (Vec4UI & q : surf.quads.posInds)
            q = Vec4UI {q[0],q[3],q[2],q[1]};
        for (Vec4UI & q : surf.quads.uvInds)
            q = Vec4UI {q[0],q[3],q[2],q[1]};
    }
    // Unchanged:
    ret.uvs = m.uvs;
    ret.markedVerts = m.markedVerts;
    return ret;
}

Mesh
copySurfaceStructure(Mesh const & from,Mesh const & to)
{
    Mesh            ret(to);
    ret.surfaces = {mergeSurfaces(ret.surfaces)};
    Surf            surf = ret.surfaces[0];
    if (!surf.quads.posInds.empty())
        fgThrow("Quads not supported");
    Vec3UIs const & tris = surf.tris.posInds;
    ret.surfaces.clear();
    ret.surfaces.resize(from.surfaces.size());
    for (size_t ss=0; ss<ret.surfaces.size(); ++ss)
        ret.surfaces[ss].name = from.surfaces[ss].name;
    for (size_t ii=0; ii<tris.size(); ++ii) {
        Vec3UI   inds = tris[ii];
        Vec3F    tpos = (ret.verts[inds[0]] + ret.verts[inds[1]] + ret.verts[inds[2]]) / 3;
        Min<float,size_t>     minSurf;
        for (size_t ss=0; ss<from.surfaces.size(); ++ss) {
            Surf const &     fs = from.surfaces[ss];
            for (size_t jj=0; jj<fs.tris.size(); ++jj) {
                Vec3UI   fi = fs.tris.posInds[jj];
                Vec3F    fpos = (from.verts[fi[0]] + from.verts[fi[1]] + from.verts[fi[2]]) / 3;
                float       mag = (fpos-tpos).mag();
                minSurf.update(mag,ss);
            }
        }
        ret.surfaces[minSurf.val()].tris.posInds.push_back(inds);
    }
    return ret;
}

Vec3UIs
meshSurfacesAsTris(Mesh const & m)
{
    Vec3UIs   ret;
    for (size_t ss=0; ss<m.surfaces.size(); ++ss)
        cat_(ret,m.surfaces[ss].convertToTris().tris.posInds);
    return ret;
}

TriSurf
cTriSurface(Mesh const & src,size_t surfIdx)
{
    TriSurf       ts;
    FGASSERT(src.surfaces.size() > surfIdx);
    ts.tris = src.surfaces[surfIdx].tris.posInds;
    ts.verts = src.verts;
    return removeUnusedVerts(ts);
}

Mesh
sortTransparentFaces(Mesh const & src,ImgRgba8 const & albedo,Mesh const & opaque)
{
    FGASSERT(!albedo.empty());
    Tris                    tris = mergeSurfaces(src.surfaces).asTris();
    size_t                  numTransparent = tris.size();
    cat_(tris,mergeSurfaces(opaque.surfaces).asTris());
    FGASSERT(tris.hasUvs());
    Mat32F                  domain = cBounds(src.verts),
                            range = {0,1, 0,1, 0,1};
    AffineEw3F              xform(domain,range);
    Vec3Fs                  verts = mapMul(xform,src.verts);
    Mat23F                  proj {1,0,0,0,1,0};
    Vec2Fs                  pts = mapMulT<Vec2F>(proj,verts);
    GridTriangles           grid = gridTriangles(pts,tris.posInds);
    // Map obsfucating tri indices to extents for each tri:
    vector<map<uint,float> > obsfs(tris.posInds.size());
    for (Iter2UI it(512); it.valid(); it.next()) {
        Vec2F            p = (Vec2F(it()) + Vec2F(0.5f)) / 512.0f;
        TriPoints         tps = grid.intersects(tris.posInds,pts,p);
        Floats              depths;
        for (TriPoint const & tp : tps)
            depths.push_back(cBarycentricVert(tp.vertInds,tp.baryCoord,verts)[2]);
        tps = permute(tps,sortInds(depths));
        float               transTotal = 1.0f;
        for (size_t ii=1; ii<tps.size(); ++ii) {
            TriPoint const &  tp = tps[ii];
            if (tp.triInd >= numTransparent)
                continue;                           // Don't analyze opaque
            TriPoint const &  tpp = tps[ii-1];    // Obsfucating tri
            if (tpp.triInd >= numTransparent)
                continue;                           // Blocked by opaque
            Vec2F            uv = cBarycentricUv(tpp.vertInds,tpp.baryCoord,src.uvs);
            float               alpha = sampleAlpha(albedo,uv).alpha(),
                                trans = 1.0f - alpha/255.0f;
            transTotal *= trans;
            if (transTotal > 0.0f) {
                map<uint,float> &   obsf = obsfs[tp.triInd];
                auto        it2 = obsf.find(tpp.triInd);
                if (it2 == obsf.end())
                    obsf[tpp.triInd] = transTotal;
                else
                    obsf[tpp.triInd] += transTotal;
            }
        }
    }
    // Peel of the ordering:
    Uints                 order;
    set<uint>               done;
    while (done.size() < numTransparent) {
        float               minObs = floatMax();
        uint                mini = 0;
        for (uint ii=0; ii<numTransparent; ++ii) {
            if (done.find(ii) == done.end()) {
                map<uint,float> const &     obsf = obsfs[ii];
                float               obsfTot = 0;
                for (auto it=obsf.begin(); it != obsf.end(); ++it) {
                    if (done.find(it->first) != done.end())
                        obsfTot += it->second;
                }
                if (obsfTot < minObs) {
                    minObs = obsfTot;
                    mini = ii;
                }
            }
        }
        order.push_back(mini);
        done.insert(mini);
    }
    order = cReverse(order);
    Surf     surf;
    surf.tris.posInds = permute(tris.posInds,order);
    surf.tris.uvInds = permute(tris.uvInds,order);
    Mesh        ret;
    ret.verts = src.verts;
    ret.uvs = src.uvs;
    ret.surfaces.push_back(surf);
    return ret;
}

}

// */
