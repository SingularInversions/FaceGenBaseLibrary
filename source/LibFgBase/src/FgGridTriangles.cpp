//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:		Andrew Beatty
// Created:		April 10, 2010
//

#include "stdafx.h"

#include "FgGridTriangles.hpp"
#include "FgIter.hpp"
#include "FgGeometry.hpp"
#include "FgCommand.hpp"
#include "FgApproxEqual.hpp"

using namespace std;

FgOpt<FgTriPoint>
FgGridTriangles::nearestIntersect(const FgVect3UIs & tris,const FgVect2Fs & verts,const FgFlts & depths,FgVect2F pos) const
{
    FgOpt<FgTriPoint>   ret;
    FgVect2F            gridCoord = clientToGridIpcs * pos;
    if (!fgBoundsIncludes(grid.dims(),gridCoord))
        return ret;
    FgVect2UI           binIdx = FgVect2UI(gridCoord);
    const FgUints &     bin = grid[binIdx];
    float               bestDepth = numeric_limits<float>::max();
    FgTriPoint          bestTp;
    for (size_t ii=0; ii<bin.size(); ++ii) {
        FgVect3UI           tri = tris[bin[ii]];
        FgVect2F            v0 = verts[tri[0]],
                            v1 = verts[tri[1]],
                            v2 = verts[tri[2]];
        FgOpt<FgVect3D>     vbc = fgBarycentricCoords(pos,v0,v1,v2);
        if (vbc.valid() && (fgMinElem(vbc.val()) >= 0)) {       // We intersect:
            FgVect3F        dpths(depths[tri[0]],depths[tri[1]],depths[tri[2]]),
                            bc = FgVect3F(vbc.val());
            float           depth = fgDot(bc,dpths);
            if (depth < bestDepth) {
                bestDepth = depth;
                bestTp.triInd = bin[ii];
                bestTp.pointInds = tri;
                bestTp.baryCoord = bc;
            }
        }
    }
    if (bestDepth < numeric_limits<float>::max())
        ret = bestTp;
    return ret;
}

void
FgGridTriangles::intersects(const FgVect3UIs & tris,const FgVect2Fs & verts,FgVect2F pos,vector<FgTriPoint> & ret) const
{
    ret.clear();
    FgVect2F            gridCoord = clientToGridIpcs * pos;
    if (!fgBoundsIncludes(grid.dims(),gridCoord))
        return;
    FgVect2UI           binIdx = FgVect2UI(gridCoord);
    const FgUints &     bin = grid[binIdx];
    for (size_t ii=0; ii<bin.size(); ++ii) {
        FgTriPoint      tp;
        tp.triInd = bin[ii];
        tp.pointInds = tris[bin[ii]];
        FgOpt<FgVect3D>    vbc;
        // All tris in index guaranteed to have valid vertex projection values:
        FgVect2F            p0 = verts[tp.pointInds[0]],
                            p1 = verts[tp.pointInds[1]],
                            p2 = verts[tp.pointInds[2]];
        vbc = fgBarycentricCoords(pos,p0,p1,p2);
        if (vbc.valid()) {
            tp.baryCoord = FgVect3F(vbc.val());
            if (fgMinElem(tp.baryCoord) >= 0.0f)
                ret.push_back(tp);
        }
    }
}

FgGridTriangles
fgGridTriangles(const FgVect2Fs & verts,const FgVect3UIs & tris,float binsPerTri)
{
    FgGridTriangles     ret;
    FGASSERT(tris.size() > 0);
    float               fmax = numeric_limits<float>::max();
    FgVect2F            domainLo(fmax),
                        domainHi(-fmax),
                        invalid(fmax);
    size_t              numValid = 0;
    for (size_t ii=0; ii<verts.size(); ++ii) {
        FgVect2F        v = verts[ii];
        if (v != invalid) {
            for (uint dd=0; dd<2; ++dd) {
                fgSetIfLess(domainLo[dd],v[dd]);
                fgSetIfGreater(domainHi[dd],v[dd]);
            }
            ++numValid;
        }
    }
    FgVect2F    domainSz = domainHi - domainLo;
    FGASSERT((domainSz[0] > 0) && (domainSz[1] > 0));
    float       numBins = numValid * binsPerTri;
    FgVect2F    rangeSizef = domainSz * sqrt(numBins/domainSz.volume());
    FgVect2UI   rangeSize = FgVect2UI(rangeSizef + FgVect2F(0.5f));
    rangeSize = fgClipLo(rangeSize,1U);
    FgMat22F    range(0,rangeSize[0],0,rangeSize[1]);
    // We could in theory intersect the client's desired sampling domain with the verts domain but
    // this optimization currently represents an unlikely case; we usually want to fit what we're
    // rendering on the image. This would change for more general-purpose ray casting.
    ret.clientToGridIpcs = FgAffineCw2F(fgConcatHoriz(domainLo,domainHi),range);
    ret.grid.resize(rangeSize);
    for (size_t ii=0; ii<tris.size(); ++ii) {
        FgVect3UI       tri = tris[ii];
        FgVect2F        p0 = verts[tri[0]],
                        p1 = verts[tri[1]],
                        p2 = verts[tri[2]];
        if ((p0 != invalid) && (p1 != invalid) && (p2 != invalid)) {
            FgMat22F    projBounds = fgInclToExcl(fgBounds(
                ret.clientToGridIpcs * p0,
                ret.clientToGridIpcs * p1,
                ret.clientToGridIpcs * p2));
            if (fgBoundsIntersect(projBounds,range,projBounds)) {
                FgMat22UI       bnds = FgMat22UI(projBounds);
                for (FgIter2UI it(bnds); it.valid(); it.next())
                    ret.grid[it()].push_back(uint(ii));
            }
        }
    }
    return ret;
}

void
fgGridTrianglesTest(const FgArgs &)
{
    // Create a triangular patch of tris:
    const uint                  dim = 10,
                                dimp = dim+1;
    FgImage<FgVect2F>           vertImg(dimp,dimp);
    for (FgIter2UI vit(dimp); vit.valid(); vit.next())
        vertImg[vit()] = FgVect2F(vit());
    vector<FgVect3UI>           tris;
    for (uint row=0; row<dim; ++row) {
        uint    col=0;
        for (; col<row; ++col) {
            tris.push_back(FgVect3UI(row*dimp+col,row*dimp+col+1,(row+1)*dimp+col+1));
            tris.push_back(FgVect3UI((row+1)*dimp+col+1,(row+1)*dimp+col,row*dimp+col));
        }
        tris.push_back(FgVect3UI((row+1)*dimp+col+1,(row+1)*dimp+col,row*dimp+col));
    }
    // Create the grid and query:
    const vector<FgVect2F> &    verts = vertImg.dataVec();
    FgGridTriangles             gts = fgGridTriangles(verts,tris);
    for (uint ii=0; ii<100; ++ii) {
        FgVect2D        posd(fgRand(),fgRand());
        FgVect2F        pos = FgVect2F(posd) * 10.0f;
        vector<FgTriPoint> res = gts.intersects(tris,verts,pos);
        if (pos[0] < pos[1]) {
            FGASSERT(res.size() == 1);
            FgTriPoint &   isect(res[0]);
            FgVect2F
                rpos(
                    verts[isect.pointInds[0]] * isect.baryCoord[0] +
                    verts[isect.pointInds[1]] * isect.baryCoord[1] +
                    verts[isect.pointInds[2]] * isect.baryCoord[2]);
            FGASSERT(fgApproxEqual(pos,rpos,64));
        }
        else
            FGASSERT(res.size() == 0);
    }
    // Query outside grid area:
    vector<FgTriPoint> res = gts.intersects(tris,verts,FgVect2F(-0.1f,0.0f));
    FGASSERT(res.size() == 0);
    res = gts.intersects(tris,verts,FgVect2F(10.1f,0.0f));
    FGASSERT(res.size() == 0);
    res = gts.intersects(tris,verts,FgVect2F(5.0f,-0.1f));
    FGASSERT(res.size() == 0);
    res = gts.intersects(tris,verts,FgVect2F(5.0f,10.1f));
    FGASSERT(res.size() == 0);
}

// */
