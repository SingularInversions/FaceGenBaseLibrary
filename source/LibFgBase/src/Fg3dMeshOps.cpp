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

using namespace std;

Fg3dMesh
fgMeshFromImage(const FgImgD & img)
{
    FGASSERT((img.height() > 1) && (img.width() > 1));
    FgMat22D             imgIdxBounds(0,img.width()-1,0,img.height()-1);
    FgAffineCw2D          imgIdxToSpace(imgIdxBounds,FgMat22D(0,1,0,1)),
                            imgIdxToOtcs(imgIdxBounds,FgMat22D(0,1,1,0));
    FgAffine1D         imgValToSpace(fgBounds(img.dataVec()),FgVectD2(0,1));
    FgVerts                 verts;
    std::vector<FgVect2F>   uvs;
    for (FgIter2UI it(img.dims()); it.valid(); it.next()) {
        FgVect2D            imgCrd = FgVect2D(it()),
                            xy = imgIdxToSpace * imgCrd;
        verts.push_back(FgVect3F(xy[0],xy[1],imgValToSpace * img[it()]));
        uvs.push_back(FgVect2F(imgIdxToOtcs * imgCrd)); }
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
        texInds.push_back(inds); }
    Fg3dSurface      surf(quads,texInds);
    return Fg3dMesh(verts,uvs,fgSvec(surf));
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
    fgAppend(uvd,fgTransform(equi,fgTranslate(0.5,0.0)));
    fgAppend(uvd,fgTransform(equi,fgTranslate(0.5,0.5)));
    fgAppend(uvd,fgTransform(equi,fgTranslate(0.0,0.5)));
    vector<FgVect2F>    uvs = fgToFloat(uvd);

    Fg3dSurface          surf(tris);
    Fg3dMesh             mesh(verts,surf);
    for (uint ss=0; ss<subdivisions; ss++) {
        for (uint ii=0; ii<mesh.verts.size(); ii++)
            mesh.verts[ii] *= radius / mesh.verts[ii].length();
        mesh = mesh.subdivideFlat();
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
    Fg3dMesh        ret = mesh;
    vector<bool>    vertUsed(mesh.verts.size(),false);
    for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
        const Fg3dSurface & surf = mesh.surfaces[ss];
        uint    num = surf.numTriEquivs();
        for (uint jj=0; jj<num; ++jj) {
            FgVect3UI   verts = surf.getTriEquiv(jj);
            vertUsed[verts[0]] = true;
            vertUsed[verts[1]] = true;
            vertUsed[verts[2]] = true;
        }
    }
    vector<uint>    remapIndices(mesh.verts.size(),0);
    FgVerts         verts;
    uint            cnt = 0;
    for (size_t ii=0; ii<vertUsed.size(); ++ii) {
        if (vertUsed[ii]) {
            verts.push_back(mesh.verts[ii]);
            remapIndices[ii] = cnt++;
        }
    }
    ret.verts = verts;
    for (size_t ss=0; ss<ret.surfaces.size(); ++ss) {
        Fg3dSurface &           surf = ret.surfaces[ss];
        for (size_t ii=0; ii<surf.tris.vertInds.size(); ++ii)
            for (uint jj=0; jj<3; ++jj)
                surf.tris.vertInds[ii][jj] = remapIndices[surf.tris.vertInds[ii][jj]];
        for (size_t ii=0; ii<surf.quads.vertInds.size(); ++ii)
            for (uint jj=0; jj<4; ++jj)
                surf.quads.vertInds[ii][jj] = remapIndices[surf.quads.vertInds[ii][jj]];
    }
    for (size_t ii=0; ii<ret.targetMorphs.size(); ++ii) {
        FgIndexedMorph &    im = ret.targetMorphs[ii];
        for (size_t jj=0; jj<im.baseInds.size(); ++jj)
            im.baseInds[jj] = remapIndices[im.baseInds[jj]];
    }
    for (size_t ii=0; ii<ret.markedVerts.size(); ++ii)
        ret.markedVerts[ii].idx = remapIndices[ret.markedVerts[ii].idx];
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

// Only currently works on quads:
Fg3dMesh
fgMergeIdenticalUvInds(const Fg3dMesh & in)
{
    Fg3dMesh    ret = in;
    const vector<FgVect2F> &    uvs = ret.uvs;
    vector<FgValid<uint> >      merge(uvs.size());
    uint                        cnt0 = 0, cnt1 = 0;
    for (size_t ii=1; ii<uvs.size(); ++ii)
        for (size_t jj=0; jj<ii-1; ++jj)
            if (uvs[ii] == uvs[jj])
                merge[ii] = uint(jj), ++cnt0;
    for (size_t ss=0; ss<ret.surfaces.size(); ++ss) {
        Fg3dSurface &           surf = const_cast<Fg3dSurface&>(ret.surfaces[ss]);
        vector<FgVect4UI> &     uvInds = const_cast<vector<FgVect4UI>&>(surf.quads.uvInds);
        for (size_t ii=0; ii<uvInds.size(); ++ii)
            for (uint jj=0; jj<4; ++jj)
                if (merge[uvInds[ii][jj]].valid())
                    uvInds[ii][jj] = merge[uvInds[ii][jj]].val(), ++cnt1;
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
    Fg3dMesh                    mesh = in;
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
    vector<Fg3dSurface>         surfs;
    for (size_t ii=0; ii<numColours; ++ii)
        surfs.push_back(Fg3dSurface(newVertInds[ii],newUvInds[ii]));
    fgout << fgnl << numColours << " separate UV-contiguous surfaces created" << endl;
    return Fg3dMesh(mesh.verts,mesh.uvs,surfs);
}

Fg3dMesh
fgMergeMeshSurfaces(
    const Fg3dMesh &    m0,
    const Fg3dMesh &    m1)
{
    FGASSERT(m0.verts != m1.verts);
    FGASSERT(m0.uvs != m1.uvs);
    vector<Fg3dSurface> surfs;
    for (size_t ss=0; ss<m0.surfaces.size(); ++ss)
        surfs.push_back(m0.surfaces[ss]);
    for (size_t ss=0; ss<m1.surfaces.size(); ++ss)
        surfs.push_back(m1.surfaces[ss]);
    return
        Fg3dMesh(m0.verts,m0.uvs,surfs);
}

Fg3dMesh
fgMergeMeshes(
    const Fg3dMesh &    m0,
    const Fg3dMesh &    m1)
{
    Fg3dMesh            ret;
    ret.name = m0.name + m1.name;
    ret.verts = fgConcat(m0.verts,m1.verts);
    ret.uvs = fgConcat(m0.uvs,m1.uvs);
    ret.surfaces = m0.surfaces;
    for (uint ss=0; ss<m1.surfaces.size(); ++ss)
        ret.surfaces.push_back(m1.surfaces[ss].offset(m0.verts.size(),m0.uvs.size()));
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
            ret.deltaMorphs[idx.val()].verts = fgConcat(m0.deltaMorphs[idx.val()].verts,dm.verts);
        else {
            dm.verts = fgConcat(FgVerts(m0.verts.size()),dm.verts);
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
            fgAppend(ret.targetMorphs[idx.val()].baseInds,im.baseInds);
            fgAppend(ret.targetMorphs[idx.val()].verts,im.verts);
        }
        else
            ret.targetMorphs.push_back(im);
    }
    return ret;
}

Fg3dMesh
fg3dMaskFromUvs(const Fg3dMesh & mesh,const FgImage<FgBool> & mask)
{
    // Make a list of which vertices have UVs that only fall in the excluded regions:
    vector<FgBool>      keep(mesh.verts.size(),false);
    FgAffineCw2F        otcsToRasterOffset(
                            FgMat22F(0,1,1,0),
                            FgMat22F(0,mask.width(),0,mask.height()));
    FgMat22UI        clampVal(0,mask.width()-1,0,mask.height()-1);
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
                bool    valid = mask[fgClamp(FgVect2UI(otcsToRasterOffset * mesh.uvs[uvInd[kk]]),clampVal)];
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

FgImgRgbaUb
fgUvImage(const Fg3dMesh & mesh,const FgImgRgbaUb & in)
{
    FgImgRgbaUb     img(2048,2048,FgRgbaUB(128,128,128,255));
    if (!in.empty())
        img = fgImgMagnify(in,2);
    FgRgbaUB        green(0,255,0,255);
    FgMat22F     domain(0,1,1,0),
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

// */
