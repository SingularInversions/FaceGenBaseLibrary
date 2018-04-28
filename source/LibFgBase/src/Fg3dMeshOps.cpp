//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     July 28, 2006
//

#include "stdafx.h"

#include "Fg3dMeshOps.hpp"
#include "Fg3dTopology.hpp"
#include "FgMath.hpp"
#include "FgAffineCwC.hpp"
#include "FgAffine1.hpp"
#include "FgBounds.hpp"
#include "FgDraw.hpp"
#include "FgAffineCwPreC.hpp"
#include "FgBestN.hpp"
#include "FgGridTriangles.hpp"

using namespace std;

Fg3dMesh
fgMeshFromImage(const FgImgD & img)
{
    Fg3dMesh                ret;
    FGASSERT((img.height() > 1) && (img.width() > 1));
    FgMat22D                imgIdxBounds(0,img.width()-1,0,img.height()-1);
    FgAffineCw2D            imgIdxToSpace(imgIdxBounds,FgMat22D(0,1,0,1)),
                            imgIdxToOtcs(imgIdxBounds,FgMat22D(0,1,1,0));
    FgAffine1D              imgValToSpace(fgBounds(img.dataVec()),FgVectD2(0,1));
    for (FgIter2UI it(img.dims()); it.valid(); it.next()) {
        FgVect2D            imgCrd = FgVect2D(it()),
                            xy = imgIdxToSpace * imgCrd;
        ret.verts.push_back(FgVect3F(xy[0],xy[1],imgValToSpace * img[it()]));
        ret.uvs.push_back(FgVect2F(imgIdxToOtcs * imgCrd));
    }
    std::vector<FgVect4UI>  quads,
                            texInds;
    uint                    w = img.width();
    for (FgIter2UI it(img.dims()-FgVect2UI(1)); it.valid(); it.next()) {
        uint                x = it()[0],
                            x1 = x + 1,
                            y = it()[1] * w,
                            y1 = y + w;
        FgVect4UI           inds(x1+y,x+y,x+y1,x1+y1);      // CC winding
        quads.push_back(inds);
        texInds.push_back(inds);
    }
    ret.surfaces.push_back(Fg3dSurface(quads,texInds));
    return ret;
}

Fg3dMesh
fgCreateSphere(
    float       radius,
    uint        subdivisions)
{
    FGASSERT(subdivisions < 10);    // 1M faces is probably overkill.
    FgVerts            verts(4);
    vector<FgVect3UI>  tris(4);

    // Tetrahedron centred on the origin:
    verts[0] = FgVect3F(1.0f,1.0f,1.0f);
    verts[1] = FgVect3F(1.0f,-1.0f,-1.0f);
    verts[2] = FgVect3F(-1.0f,1.0f,-1.0f);
    verts[3] = FgVect3F(-1.0f,-1.0f,1.0f);
    tris[0] =  FgVect3UI(0,1,2);                // CC winding is the default.
    tris[1] =  FgVect3UI(0,3,1);
    tris[2] =  FgVect3UI(0,2,3);
    tris[3] =  FgVect3UI(1,3,2);

    // Equilateral triangle with radius 2 around origin (CC):
    double              root3 = sqrt(3.0);
    vector<FgVect2D>    equi =
        fgSvec(
            FgVect2D(-root3,-1.0),
            FgVect2D( root3,-1.0),
            FgVect2D(     0,root3));
    // Put 4 of these in OpenGL texture coordinates:
    FgAffineCwPre2D     xform(FgVect2D(root3,1.0),FgVect2D(0.5/(1.0+root3)));
    equi = fgTransform(equi,xform);
    vector<FgVect2D>    uvd = equi;
    fgCat_(uvd,fgTransform(equi,fgTranslate(0.5,0.0)));
    fgCat_(uvd,fgTransform(equi,fgTranslate(0.5,0.5)));
    fgCat_(uvd,fgTransform(equi,fgTranslate(0.0,0.5)));
    vector<FgVect2F>    uvs = fgToFloat(uvd);

    Fg3dSurface          surf(tris);
    Fg3dMesh             mesh(verts,surf);
    for (uint ss=0; ss<subdivisions; ss++) {
        for (uint ii=0; ii<mesh.verts.size(); ii++)
            mesh.verts[ii] *= radius / mesh.verts[ii].length();
        mesh = fgSubdivide(mesh,false);
    }
    for (uint ii=0; ii<mesh.verts.size(); ii++)
        mesh.verts[ii] *= radius / mesh.verts[ii].length();
    return mesh;
}

Fg3dMesh
fgRemoveDuplicateFacets(const Fg3dMesh & mesh)
{
    Fg3dMesh    ret = mesh;
    for (size_t ss=0; ss<ret.surfaces.size(); ++ss) {
        ret.surfaces[ss] = fgRemoveDuplicateFacets(ret.surfaces[ss]);
    }
    return ret;
}

Fg3dMesh
fgRemoveUnusedVerts(const Fg3dMesh & mesh)
{
    Fg3dMesh            ret;
    ret.name = mesh.name;
    ret.surfaces = mesh.surfaces;
    // Which vertices & uvs are referenced by a surface or marked vertex:
    vector<bool>        vertUsed(mesh.verts.size(),false);
    vector<bool>        uvsUsed(mesh.uvs.size(),false);
    for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
        const Fg3dSurface & surf = mesh.surfaces[ss];
        for (size_t tt=0; tt<surf.tris.size(); ++tt) {
            FgVect3UI   v = surf.tris.vertInds[tt];
            for (uint ii=0; ii<3; ++ii)
                vertUsed[v[ii]] = true;
        }
        for (size_t tt=0; tt<surf.quads.size(); ++tt) {
            FgVect4UI   v = surf.quads.vertInds[tt];
            for (uint ii=0; ii<4; ++ii)
                vertUsed[v[ii]] = true;
        }
        for (size_t tt=0; tt<surf.tris.uvInds.size(); ++tt) {
            FgVect3UI   u = surf.tris.uvInds[tt];
            for (uint ii=0; ii<3; ++ii)
                uvsUsed[u[ii]] = true;
        }
        for (size_t tt=0; tt<surf.quads.uvInds.size(); ++tt) {
            FgVect4UI   u = surf.quads.uvInds[tt];
            for (uint ii=0; ii<4; ++ii)
                uvsUsed[u[ii]] = true;
        }
    }
    for (size_t ii=0; ii<mesh.markedVerts.size(); ++ii)
        vertUsed[mesh.markedVerts[ii].idx] = true;
    // Create the new vertex list:
    vector<uint>    mapVerts(mesh.verts.size(),numeric_limits<uint>::max());
    uint            cnt = 0;
    for (size_t ii=0; ii<vertUsed.size(); ++ii) {
        if (vertUsed[ii]) {
            ret.verts.push_back(mesh.verts[ii]);
            mapVerts[ii] = cnt++;
        }
    }
    // Create the new uv list:
    vector<uint>    mapUvs(mesh.uvs.size(),numeric_limits<uint>::max());
    cnt = 0;
    for (size_t ii=0; ii<uvsUsed.size(); ++ii) {
        if (uvsUsed[ii]) {
            ret.uvs.push_back(mesh.uvs[ii]);
            mapUvs[ii] = cnt++;
        }
    }
    // Remap the surfaces and marked verts, which we know all map to valid indices in the new list:
    for (size_t ss=0; ss<ret.surfaces.size(); ++ss) {
        Fg3dSurface &           surf = ret.surfaces[ss];
        for (size_t ii=0; ii<surf.tris.size(); ++ii)
            for (uint jj=0; jj<3; ++jj)
                surf.tris.vertInds[ii][jj] = mapVerts[surf.tris.vertInds[ii][jj]];
        for (size_t ii=0; ii<surf.quads.size(); ++ii)
            for (uint jj=0; jj<4; ++jj)
                surf.quads.vertInds[ii][jj] = mapVerts[surf.quads.vertInds[ii][jj]];
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
    FgVect3F            zero(0);
    for (size_t ii=0; ii<mesh.deltaMorphs.size(); ++ii) {
        const FgMorph & src = mesh.deltaMorphs[ii];
        FgMorph         dst;
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
        const FgIndexedMorph &      src = mesh.targetMorphs[ii];
        FgIndexedMorph              dst;
        dst.name = src.name;
        for (size_t jj=0; jj<src.baseInds.size(); ++jj) {
            uint    idx = mapVerts[src.baseInds[jj]];
            if (idx != numeric_limits<uint>::max()) {
                dst.baseInds.push_back(idx);
                dst.verts.push_back(src.verts[jj]);
            }
        }
        if (!dst.baseInds.empty())
            ret.targetMorphs.push_back(dst);
    }
    return  ret;
}

Fg3dMesh
fgTetrahedron(bool open)
{
    // Coordinates of a regular tetrahedron with edges of length 2*sqrt(2):
    FgVerts             verts;
    verts.push_back(FgVect3F( 1.0f, 1.0f, 1.0f));
    verts.push_back(FgVect3F(-1.0f,-1.0f, 1.0f));
    verts.push_back(FgVect3F(-1.0f, 1.0f,-1.0f));
    verts.push_back(FgVect3F( 1.0f,-1.0f,-1.0f));

    vector<FgVect3UI>   tris;
    tris.push_back(FgVect3UI(0,1,3));
    tris.push_back(FgVect3UI(0,2,1));
    tris.push_back(FgVect3UI(2,0,3));
    if (!open)
        tris.push_back(FgVect3UI(1,2,3));

    return Fg3dMesh(verts,Fg3dSurface(tris));
}

Fg3dMesh
fgPyramid(bool open)
{
    FgVerts         verts;
    verts.push_back(FgVect3F(-1.0f,0.0f,-1.0f));
    verts.push_back(FgVect3F(1.0f,0.0f,-1.0f));
    verts.push_back(FgVect3F(-1.0f,0.0f,1.0f));
    verts.push_back(FgVect3F(1.0f,0.0f,1.0f));
    verts.push_back(FgVect3F(0.0f,1.0f,0.0f));
    vector<FgVect3UI>   tris;
    tris.push_back(FgVect3UI(0,4,1));
    tris.push_back(FgVect3UI(0,2,4));
    tris.push_back(FgVect3UI(2,3,4));
    tris.push_back(FgVect3UI(1,4,3));
    if (!open) {
        tris.push_back(FgVect3UI(0,1,3));
        tris.push_back(FgVect3UI(3,2,0));
    }
    return Fg3dMesh(verts,Fg3dSurface(tris));
}

Fg3dMesh
fgCube(bool open)
{
    FgVerts             verts;
    for (uint vv=0; vv<8; ++vv)
        verts.push_back(
            FgVect3F(
                float(vv & 0x01),
                float((vv >> 1) & 0x01),
                float((vv >> 2) & 0x01)) * 2.0f -
            FgVect3F(1.0f));
    vector<FgVect3UI>   tris;
    // X planes:
    tris.push_back(FgVect3UI(0,4,6));
    tris.push_back(FgVect3UI(6,2,0));
    tris.push_back(FgVect3UI(5,1,3));
    tris.push_back(FgVect3UI(3,7,5));
    // Z planes:
    tris.push_back(FgVect3UI(1,0,2));
    tris.push_back(FgVect3UI(2,3,1));
    tris.push_back(FgVect3UI(4,5,7));
    tris.push_back(FgVect3UI(7,6,4));
    // Y planes:
    tris.push_back(FgVect3UI(0,1,5));
    tris.push_back(FgVect3UI(5,4,0));
    if (!open) {
        tris.push_back(FgVect3UI(3,2,6));
        tris.push_back(FgVect3UI(6,7,3));
    }
    return Fg3dMesh(verts,Fg3dSurface(tris));
}

Fg3dMesh
fgOctahedron()
{
    FgVerts             verts(6);
    uint                cnt = 0;
    for (uint dd=0; dd<3; ++dd)
        for (int ss=-1; ss<=1; ss+=2)
            verts[cnt++][dd] = float(ss);
    vector<FgVect3UI>   tris;
    tris.push_back(FgVect3UI(0,2,5));
    tris.push_back(FgVect3UI(0,3,4));
    tris.push_back(FgVect3UI(0,4,2));
    tris.push_back(FgVect3UI(0,5,3));
    tris.push_back(FgVect3UI(1,2,4));
    tris.push_back(FgVect3UI(1,3,5));
    tris.push_back(FgVect3UI(1,4,3));
    tris.push_back(FgVect3UI(1,5,2));
    return Fg3dMesh(verts,Fg3dSurface(tris));
}

Fg3dMesh
fgNTent(uint nn)
{
    FGASSERT(nn > 2);
    FgVerts             verts;
    verts.push_back(FgVect3F(0.0f,1.0f,0.0f));
    float   step = 2.0f * float(fgPi()) / float(nn);
    for (uint ii=0; ii<nn; ++ii) {
        float   angle = step * float(ii);
        verts.push_back(FgVect3F(cos(angle),0.0f,sin(angle)));
    }
    vector<FgVect3UI>   tris;
    for (uint ii=0; ii<nn; ++ii)
        tris.push_back(FgVect3UI(0,ii+1,((ii+1)%nn)+1));
    return Fg3dMesh(verts,Fg3dSurface(tris));
}

//Fg3dMesh
//fgFddCage(float size,float thick)
//{
//    Fg3dMesh            ret;
//    vector<FgVect3F>    sqrVerts;
//    sqrVerts.push_back(FgVect3F(-thick,-thick,0));
//    sqrVerts.push_back(FgVect3F(-thick,thick,0));
//    sqrVerts.push_back(FgVect3F(thick,thick,0));
//    sqrVerts.push_back(FgVect3F(thick,-thick,0));
//    FgVect4UI           sqrInds(0,1,2,3);
//    //fgCat_(ret.verts,fgMap(sqrVerts
//    for (uint axis=0; axis<3; ++axis) {
//        FgVect3F    l(0);
//        for (int aa=-1; aa<2; aa+=2)
//            for (int bb=
//    }
//}

Fg3dMesh
fgMergeSameNameSurfaces(const Fg3dMesh & in)
{
    Fg3dMesh            ret = in;
    vector<Fg3dSurface> surfs;
    for (size_t ss=0; ss<in.surfaces.size(); ++ss) {
        const Fg3dSurface & surf = in.surfaces[ss];
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

Fg3dMesh
fgUnifyIdenticalVerts(const Fg3dMesh & mesh)
{
    Fg3dMesh            ret(mesh);
    FgVerts             verts;
    vector<uint>        map;
    map.reserve(mesh.verts.size());
    uint                cnt = 0;
    for (uint vv=0; vv<mesh.verts.size(); ++vv) {
        bool            dup = false;
        FgVect3F        v = mesh.verts[vv];
        for (uint ww=0; ww<verts.size(); ++ww) {
            if (verts[ww] == v) {
                dup = true;
                map.push_back(ww);
                break;
            }
        }
        if (!dup) {
            verts.push_back(mesh.verts[vv]);
            map.push_back(cnt);
            ++cnt;
        }
    }
    FGASSERT(mesh.verts.size() == map.size());
    for (size_t ss=0; ss<ret.surfaces.size(); ++ss) {
        Fg3dSurface &           surf = ret.surfaces[ss];
        for (size_t ii=0; ii<surf.tris.size(); ++ii)
            for (uint jj=0; jj<3; ++jj)
                surf.tris.vertInds[ii][jj] = map[surf.tris.vertInds[ii][jj]];
        for (size_t ii=0; ii<surf.quads.size(); ++ii)
            for (uint jj=0; jj<4; ++jj)
                surf.quads.vertInds[ii][jj] = map[surf.quads.vertInds[ii][jj]];
    }
    for (size_t ii=0; ii<ret.deltaMorphs.size(); ++ii) {
        const FgMorph &     src = mesh.deltaMorphs[ii];
        FgMorph &           dst = ret.deltaMorphs[ii];
        FGASSERT(dst.verts.size() == map.size());
        for (size_t jj=0; jj<dst.verts.size(); ++jj)
            dst.verts[jj] = src.verts[map[jj]];
    }
    for (size_t ii=0; ii<ret.targetMorphs.size(); ++ii) {
        FgIndexedMorph &    im = ret.targetMorphs[ii];
        for (size_t jj=0; jj<im.baseInds.size(); ++jj)
            im.baseInds[jj] = map[im.baseInds[jj]];
    }
    for (size_t ii=0; ii<ret.markedVerts.size(); ++ii)
        ret.markedVerts[ii].idx = map[ret.markedVerts[ii].idx];
    ret.verts = verts;
    return ret;
}

Fg3dMesh
fgUnifyIdenticalUvs(const Fg3dMesh & in)
{
    Fg3dMesh                    ret(in);
    const vector<FgVect2F> &    uvs = ret.uvs;
    vector<FgValid<uint> >      merge(uvs.size());
    size_t                      cnt0 = 0,
                                cnt1 = 0;
    for (size_t ii=1; ii<uvs.size(); ++ii)
        for (size_t jj=0; jj<ii-1; ++jj)
            if (uvs[ii] == uvs[jj])
                merge[ii] = uint(jj), ++cnt0;
    for (size_t ss=0; ss<ret.surfaces.size(); ++ss) {
        Fg3dSurface &           surf = ret.surfaces[ss];
        vector<FgVect3UI> &     triUvInds = surf.tris.uvInds;
        for (size_t ii=0; ii<triUvInds.size(); ++ii) {
            for (uint jj=0; jj<3; ++jj) {
                if (merge[triUvInds[ii][jj]].valid()) {
                    triUvInds[ii][jj] = merge[triUvInds[ii][jj]].val();
                    ++cnt1;
                }
            }
        }
        vector<FgVect4UI> &     quadUvInds = surf.quads.uvInds;
        for (size_t ii=0; ii<quadUvInds.size(); ++ii) {
            for (uint jj=0; jj<4; ++jj) {
                if (merge[quadUvInds[ii][jj]].valid()) {
                    quadUvInds[ii][jj] = merge[quadUvInds[ii][jj]].val();
                    ++cnt1;
                }
            }
        }
    }
    fgout << fgnl << cnt0 << " UVs merged " << cnt1 << " UV indices redirected" << endl;
    return ret;
}

// Warning: This function will yield stack overflow for large meshes unless you
// increase your stack size:
static
void
traverse(
    const vector<vector<uint> > &   uvToQuadsIndex,
    vector<uint> &              colourMap,
    const vector<FgVect4UI> &   uvInds,
    uint                        colour,
    uint                        quadIdx)
{
    FGASSERT(colour > 0);
    colourMap[quadIdx] = colour;
    for (uint ii=0; ii<4; ++ii) {
        uint                    uvIdx = uvInds[quadIdx][ii];
        const vector<uint> &    quadInds = uvToQuadsIndex[uvIdx];
        for (size_t jj=0; jj<quadInds.size(); ++jj)
            if (colourMap[quadInds[jj]] == 0)
                traverse(uvToQuadsIndex,colourMap,uvInds,colour,quadInds[jj]);
    }
}

Fg3dMesh
fgSplitSurfsByUvs(const Fg3dMesh & in)
{
    Fg3dMesh                    ret,
                                mesh = in;
    mesh.mergeAllSurfaces();
    const Fg3dSurface &         surf = mesh.surfaces[0];
    if (!surf.tris.uvInds.empty())
        fgThrow("Tris not currently supported for this operation (quads only)");
    const vector<FgVect4UI> &   uvInds = surf.quads.uvInds,
                            &   quadInds = surf.quads.vertInds;
    vector<vector<uint> >       uvToQuadsIndex(mesh.uvs.size());
    for (size_t ii=0; ii<uvInds.size(); ++ii)
        for (uint jj=0; jj<4; ++jj)
            uvToQuadsIndex[uvInds[ii][jj]].push_back(uint(ii));
    vector<uint>                colourMap(uvInds.size(),0);
    uint                        numColours = 0;
    for (size_t ii=0; ii<colourMap.size(); ++ii)
        if (colourMap[ii] == 0)
            traverse(uvToQuadsIndex,colourMap,uvInds,++numColours,uint(ii));
    vector<vector<FgVect4UI> >  newVertInds(numColours),
                                newUvInds(numColours);
    for (size_t ii=0; ii<colourMap.size(); ++ii) {
        newVertInds[colourMap[ii]-1].push_back(quadInds[ii]);
        newUvInds[colourMap[ii]-1].push_back(uvInds[ii]);
    }
    ret.verts = mesh.verts;
    ret.uvs = mesh.uvs;
    for (size_t ii=0; ii<numColours; ++ii)
        ret.surfaces.push_back(Fg3dSurface(newVertInds[ii],newUvInds[ii]));
    fgout << fgnl << numColours << " separate UV-contiguous surfaces created" << endl;
    return ret;
}

Fg3dMesh
fgMergeMeshSurfaces(
    const Fg3dMesh &    m0,
    const Fg3dMesh &    m1)
{
    Fg3dMesh        ret;
    ret.verts = m0.verts;
    ret.uvs = m0.uvs;
    FGASSERT(m0.verts != m1.verts);
    FGASSERT(m0.uvs != m1.uvs);
    for (size_t ss=0; ss<m0.surfaces.size(); ++ss)
        ret.surfaces.push_back(m0.surfaces[ss]);
    for (size_t ss=0; ss<m1.surfaces.size(); ++ss)
        ret.surfaces.push_back(m1.surfaces[ss]);
    return ret;
}

Fg3dMesh
fgMergeMeshes(
    const Fg3dMesh &    m0,
    const Fg3dMesh &    m1)
{
    Fg3dMesh            ret;
    if (!m0.name.empty() && !m1.name.empty())
        ret.name = m0.name + "_" + m1.name;
    else
        ret.name = m0.name + m1.name;
    ret.verts = fgCat(m0.verts,m1.verts);
    ret.uvs = fgCat(m0.uvs,m1.uvs);
    ret.surfaces = fgEnsureNamed(m0.surfaces,m0.name);
    vector<Fg3dSurface>     s1s = fgEnsureNamed(m1.surfaces,m1.name);
    for (uint ss=0; ss<s1s.size(); ++ss)
        ret.surfaces.push_back(s1s[ss].offsetIndices(m0.verts.size(),m0.uvs.size()));
    for (size_t ii=0; ii<m0.deltaMorphs.size(); ++ii) {
        FgMorph     dm = m0.deltaMorphs[ii];
        dm.verts.resize(m0.verts.size()+m1.verts.size());
        ret.deltaMorphs.push_back(dm);
    }
    for (size_t ii=0; ii<m1.deltaMorphs.size(); ++ii) {
        FgMorph             dm = m1.deltaMorphs[ii];
        // Search m0 not ret in case of duplicate morph names in m1:
        FgValid<size_t>     idx = m0.findDeltaMorph(dm.name);
        if (idx.valid())
            ret.deltaMorphs[idx.val()].verts = fgCat(m0.deltaMorphs[idx.val()].verts,dm.verts);
        else {
            dm.verts = fgCat(FgVerts(m0.verts.size()),dm.verts);
            ret.deltaMorphs.push_back(dm);
        }
    }
    for (size_t ii=0; ii<m0.targetMorphs.size(); ++ii)
        ret.targetMorphs.push_back(m0.targetMorphs[ii]);
    for (size_t ii=0; ii<m1.targetMorphs.size(); ++ii) {
        FgIndexedMorph      im = m1.targetMorphs[ii];
        im.baseInds = im.baseInds + vector<uint32>(im.baseInds.size(),uint32(m0.verts.size()));
        FgValid<size_t>     idx = m0.findTargMorph(im.name);
        if (idx.valid()) {
            fgCat_(ret.targetMorphs[idx.val()].baseInds,im.baseInds);
            fgCat_(ret.targetMorphs[idx.val()].verts,im.verts);
        }
        else
            ret.targetMorphs.push_back(im);
    }
    ret.markedVerts = m0.markedVerts;
    for (FgMarkedVert mv : m1.markedVerts) {
        mv.idx += uint(m0.verts.size());
        ret.markedVerts.push_back(mv);
    }
    return ret;
}

Fg3dMesh
fgMergeMeshes(const vector<Fg3dMesh> & meshes)
{
    Fg3dMesh        ret;
    if (meshes.empty())
        return ret;
    ret = meshes[0];
    for (size_t mm=1; mm<meshes.size(); ++mm)
        ret = fgMergeMeshes(ret,meshes[mm]);
    return ret;
}

Fg3dMesh
fg3dMaskFromUvs(const Fg3dMesh & mesh,const FgImage<FgBool> & mask)
{
    // Make a list of which vertices have UVs that only fall in the excluded regions:
    vector<FgBool>      keep(mesh.verts.size(),false);
    FgAffineCw2F        otcsToIpcs = fgOtcsToIpcs(mask.dims());
    FgMat22UI           clampVal(0,mask.width()-1,0,mask.height()-1);
    for (size_t ii=0; ii<mesh.surfaces.size(); ++ii) {
        const Fg3dSurface & surf = mesh.surfaces[ii];
        if (!surf.quads.uvInds.empty())
            fgThrow("Quads not supported");
        if (surf.tris.uvInds.empty())
            fgThrow("No tri facet UVs");
        for (size_t jj=0; jj<surf.tris.uvInds.size(); ++jj) {
            FgVect3UI   uvInd = surf.tris.uvInds[jj];
            FgVect3UI   vtInd = surf.tris.vertInds[jj];
            for (uint kk=0; kk<3; ++kk) {
                bool    valid = mask[fgClipToBounds(FgVect2UI(otcsToIpcs * mesh.uvs[uvInd[kk]]),clampVal)];
                keep[vtInd[kk]] = keep[vtInd[kk]] || valid;
            }
        }
    }
    // Remove the facets that use those vertices:
    vector<Fg3dSurface> nsurfs;
    for (size_t ii=0; ii<mesh.surfaces.size(); ++ii) {
        const Fg3dSurface & surf = mesh.surfaces[ii];
        Fg3dSurface         nsurf;
        for (size_t jj=0; jj<surf.tris.uvInds.size(); ++jj) {
            FgVect3UI   vtInd = surf.tris.vertInds[jj];
            bool copy = false;
            for (uint kk=0; kk<3; ++kk)
                copy = copy || keep[vtInd[kk]];
            if (copy)
                nsurf.tris.vertInds.push_back(vtInd);
        }
        nsurfs.push_back(nsurf);
    }
    // Remove unused vertices:
    return fgRemoveUnusedVerts(Fg3dMesh(mesh.verts,nsurfs));
}

FgImgUC
fgGetUvCover(const Fg3dMesh & mesh,FgVect2UI dims)
{
    FgImgUC                 ret(dims,uchar(0));
    FgVect3UIs              tris = mesh.getTriEquivs().uvInds;
    FgGridTriangles         grid = fgGridTriangles(mesh.uvs,tris);
    FgAffineCw2F            ircsToOtcs(
        FgMat22F(-0.5f,dims[0]-0.5f,-0.5f,dims[1]-0.5f),
        FgMat22F(0,1,1,0));
    for (FgIter2UI it(dims); it.valid(); it.next())
        if (!grid.intersects(tris,mesh.uvs,ircsToOtcs * FgVect2F(it())).empty())
            ret[it()] = uchar(255);
    return ret;
}

FgImgRgbaUb
fgUvImage(const Fg3dMesh & mesh,const FgImgRgbaUb & in)
{
    FgMat22F        uvb = fgBounds(mesh.uvs);
    FgImgRgbaUb     img(2048,2048,FgRgbaUB(128,128,128,255));
    if (!in.empty())
        img = fgImgMagnify(in,2);
    FgRgbaUB        green(0,255,0,255);
    // Bounds are normally (0,1,1,0) because we have to invert Y to go from OTCS to IPCS:
    FgMat22F        domain(floor(uvb[0]),ceil(uvb[1]),ceil(uvb[3]),floor(uvb[2])),
                    range(0,img.width()+1,0,img.height()+1);
    FgAffineCw2F    xf(domain,range);
    const vector<FgVect2F> &    uvs = mesh.uvs;
    for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
        const Fg3dSurface &     surf = mesh.surfaces[ss];
        for (size_t tt=0; tt<surf.tris.uvInds.size(); ++tt) {
            FgVect3UI           tri = surf.tris.uvInds[tt];
            for (size_t vv=0; vv<3; ++vv)
                fgDrawLineIrcs(img,FgVect2I(xf*uvs[tri[vv]]),FgVect2I(xf*uvs[tri[(vv+1)%3]]),green);
        }
        for (size_t tt=0; tt<surf.quads.uvInds.size(); ++tt) {
            FgVect4UI           quad = surf.quads.uvInds[tt];
            for (size_t vv=0; vv<4; ++vv)
                fgDrawLineIrcs(img,FgVect2I(xf*uvs[quad[vv]]),FgVect2I(xf*uvs[quad[(vv+1)%4]]),green);
        }
    }
    FgImgRgbaUb     final;
    fgImgShrink2(img,final);
    return final;
}

FgVerts
fgEmboss(const Fg3dMesh & mesh,const FgImgUC & logoImg,double val)
{
    FgVerts         ret;
    // Don't check for UV seams, just let the emboss value be the last one traversed:
    FgVerts         deltas(mesh.verts.size());
    vector<size_t>  embossedVertInds;
    Fg3dNormals     norms = fgNormals(mesh);
    for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
        const Fg3dSurface &     surf = mesh.surfaces[ss];
        for (size_t ii=0; ii<surf.numTris(); ++ii) {
            FgVect3UI   uvInds = surf.tris.uvInds[ii],
                        vtInds = surf.tris.vertInds[ii];
            for (uint jj=0; jj<3; ++jj) {
                FgVect2F        uv = mesh.uvs[uvInds[jj]];
                uv[1] = 1.0f - uv[1];       // Convert from OTCS to IUCS
                float           imgSamp = fgBlerpClipIucs(logoImg,uv) / 255.0f;
                uint            vtIdx = vtInds[jj];
                deltas[vtIdx] = norms.vert[vtIdx] * imgSamp;
                if (imgSamp > 0)
                    embossedVertInds.push_back(vtIdx);
            }
        }
        for (size_t ii=0; ii<surf.numQuads(); ++ii) {
            FgVect4UI   uvInds = surf.quads.uvInds[ii],
                        vtInds = surf.quads.vertInds[ii];
            for (uint jj=0; jj<4; ++jj) {
                FgVect2F        uv = mesh.uvs[uvInds[jj]];
                uv[1] = 1.0f - uv[1];       // Convert from OTCS to IUCS
                float           imgSamp = fgBlerpClipIucs(logoImg,uv) / 255.0f;
                uint            vtIdx = vtInds[jj];
                deltas[vtIdx] = norms.vert[vtIdx] * imgSamp;
                if (imgSamp > 0)
                    embossedVertInds.push_back(vtIdx);
            }
        }
    }
    float       fac = fgMaxElem(fgDims(fgReorder(mesh.verts,embossedVertInds))) * val;
    ret.resize(mesh.verts.size());
    for (size_t ii=0; ii<deltas.size(); ++ii)
        ret[ii] = mesh.verts[ii] + deltas[ii] * fac;
    return ret;
}

FgVerts
fgApplyExpression(const Fg3dMesh & mesh,const vector<FgMorphVal> & expression)
{
    FgVerts         ret;
    FgFlts        coord(mesh.numMorphs(),0);
    for (size_t ii=0; ii<expression.size(); ++ii) {
        FgValid<size_t>     idx = mesh.findMorph(expression[ii].name);
        if (idx.valid())
            coord[idx.val()] = expression[ii].val;
    }
    mesh.morph(coord,ret);
    return ret;
}

void
fgSurfPointsToMarkedVerts(const Fg3dMesh & in,Fg3dMesh & out)
{
    for (size_t ii=0; ii<in.surfaces.size(); ++ii) {
        const Fg3dSurface &     surf = in.surfaces[ii];
        for (size_t jj=0; jj<surf.surfPoints.size(); ++jj) {
            FgVect3F        pos = surf.surfPointPos(in.verts,jj);
            out.addMarkedVert(pos,surf.surfPoints[jj].label);
        }
    }
}

FgMeshMirror
fgMeshMirrorX(const Fg3dMesh & in)
{
    FgMeshMirror    ret;
    ret.mesh = in;
    ret.mesh.deltaMorphs.clear();
    ret.mesh.targetMorphs.clear();
    // Mirror the vertices not on the saggital plane. Track this by index not position
    // since lip touch may have co-incident (but different) verts:
    ret.mirrorInds.resize(in.verts.size());
    for (size_t ii=0; ii<in.verts.size(); ++ii) {
        FgVect3F    pos = in.verts[ii];
        if (pos[0] == 0)
            ret.mirrorInds[ii] = uint(ii);
        else {
            ret.mirrorInds[ii] = uint(ret.mesh.verts.size());
            pos[0] *= -1;
            ret.mesh.verts.push_back(pos);
            ret.mirrorInds.push_back(uint(ii));
        }
    }
    for (size_t ss=0; ss<ret.mesh.surfaces.size(); ++ss) {
        const Fg3dSurface &     si = in.surfaces[ss];
        Fg3dSurface &           so = ret.mesh.surfaces[ss];
        FGASSERT(si.quads.empty());
        for (size_t ff=0; ff<si.tris.size(); ++ff) {
            FgVect3UI           tri = si.tris.vertInds[ff],
                                ntri;
            for (uint ii=0; ii<3; ++ii) {
                FgVect3F        p = in.verts[tri[ii]];
                if (p[0] == 0)
                    ntri[ii] = tri[ii];
                else
                    ntri[ii] = ret.mirrorInds[tri[ii]];
            }
            std::swap(ntri[1],ntri[2]);     // Avoid mirrored winding
            so.tris.vertInds.push_back(ntri);
        }
    }
    return ret;
}

Fg3dMesh
fgCopySurfaceStructure(const Fg3dMesh & from,const Fg3dMesh & to)
{
    Fg3dMesh        ret(to);
    ret.mergeAllSurfaces();
    Fg3dSurface     surf = ret.surfaces[0];
    if (!surf.quads.vertInds.empty())
        fgThrow("Quads not supported");
    const vector<FgVect3UI> & tris = surf.tris.vertInds;
    ret.surfaces.clear();
    ret.surfaces.resize(from.surfaces.size());
    for (size_t ss=0; ss<ret.surfaces.size(); ++ss)
        ret.surfaces[ss].name = from.surfaces[ss].name;
    for (size_t ii=0; ii<tris.size(); ++ii) {
        FgVect3UI   inds = tris[ii];
        FgVect3F    tpos = (ret.verts[inds[0]] + ret.verts[inds[1]] + ret.verts[inds[2]]) / 3;
        FgMin<float,size_t>     minSurf;
        for (size_t ss=0; ss<from.surfaces.size(); ++ss) {
            const Fg3dSurface &     fs = from.surfaces[ss];
            for (size_t jj=0; jj<fs.tris.size(); ++jj) {
                FgVect3UI   fi = fs.tris.vertInds[jj];
                FgVect3F    fpos = (from.verts[fi[0]] + from.verts[fi[1]] + from.verts[fi[2]]) / 3;
                float       mag = (fpos-tpos).mag();
                minSurf.update(mag,ss);
            }
        }
        ret.surfaces[minSurf.val()].tris.vertInds.push_back(inds);
    }
    return ret;
}

vector<FgVect3UI>
fgMeshSurfacesAsTris(const Fg3dMesh & m)
{
    vector<FgVect3UI>   ret;
    for (size_t ss=0; ss<m.surfaces.size(); ++ss)
        fgCat_(ret,m.surfaces[ss].convertToTris().tris.vertInds);
    return ret;
}

FgTriSurf
fgTriSurface(const Fg3dMesh & src,size_t surfIdx)
{
    FgTriSurf       ts;
    FGASSERT(src.surfaces.size() > surfIdx);
    ts.tris = src.surfaces[surfIdx].tris.vertInds;
    ts.verts = src.verts;
    return fgRemoveUnusedVerts(ts);
}

// */
