//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgGridIndex.hpp"
#include "FgMain.hpp"

using namespace std;

namespace Fg {

GridTriangles::GridTriangles(
    Vec2Fs const &      vs,
    Arr3UIs const &     ts,
    float               binsPerTri)
    :
    verts{vs}, tris{ts}
{
    FGASSERT(tris.size() > 0);
    Arr<Vec2F,2>        bounds = nullBounds<Vec2F>();
    Vec2F const         invalid(lims<float>::max());
    size_t              numValid = 0;
    for (Vec2F v : verts) {
        if (v != invalid) {
            updateMin_(bounds[0],v);
            updateMax_(bounds[1],v);
            ++numValid;
        }
    }
    Vec2F               domainSz = bounds[1] - bounds[0];
    FGASSERT(allGtZero(domainSz.m));
    float               numBins = numValid * binsPerTri;
    Vec2F               rangeSizef = domainSz * sqrt(numBins/domainSz.elemsProduct());
    Vec2UI              rangeSize = Vec2UI(rangeSizef + Vec2F(0.5f));
    rangeSize = mapMax(rangeSize,1U);
    Mat22F              range(0,rangeSize[0],0,rangeSize[1]);
    // We could in theory intersect the client's desired sampling domain with the verts domain but
    // this optimization currently represents an unlikely case; we usually want to fit what we're
    // rendering on the image. This would change for more general-purpose ray casting.
    clientToGridPacs = AxAffine2F(catH(bounds),range);
    grid.resize(rangeSize);
    for (size_t ii=0; ii<tris.size(); ++ii) {
        Arr3UI          tri = tris[ii];
        Arr<Vec2F,3>    tvs = mapIndex(tri,verts);
        if ((tvs[0] != invalid) && (tvs[1] != invalid) && (tvs[2] != invalid)) {
            Mat22F          projBounds = iubToEub(catH(cBounds(mapMulR(clientToGridPacs,tvs))));
            projBounds = intersectBounds(projBounds,range);
            if (!isBoundEubEmpty(projBounds)) {
                Mat22UI         bnds = Mat22UI(projBounds);
                for (Iter2UI it(bnds); it.valid(); it.next())
                    grid[it()].push_back(uint(ii));
            }
        }
    }
}

Opt<TriPoint>       GridTriangles::nearestIntersect(
    Floats const &          invDepths,
    Vec2F                   pos)
    const
{
    Opt<TriPoint>       ret;
    Vec2F               gridCoord = clientToGridPacs * pos;
    if (!isInUpperBounds(grid.dims(),gridCoord))
        return ret;
    Vec2UI              binIdx = Vec2UI(gridCoord);
    Uints const &       bin = grid[binIdx];
    float               bestInvDepth = 0.0f;
    TriPoint            bestTp {0,{0,0,0},{0,0,0}};                 // zero fill to avoid warning
    for (size_t ii=0; ii<bin.size(); ++ii) {
        Arr3UI              tri = tris[bin[ii]];
        Arr<Vec2F,3>        v = mapIndex(tri,verts);
        Opt<Arr3F>          vbc = cBarycentricCoord(pos,v);
        if (vbc.has_value() && (cMinElem(vbc.value()) >= 0)) {       // We intersect:
            Arr3F               ids = mapIndex(tri,invDepths);
            Arr3F               bc = vbc.value();
            float               invDepth = multAcc(bc,ids);         // Interpolation in projected values is harmonic
            if (invDepth > bestInvDepth) {                          // Closer on same ray
                bestInvDepth = invDepth;
                bestTp.triInd = bin[ii];
                bestTp.vertInds = tri;
                bestTp.baryCoord = bc;
            }
        }
    }
    if (bestInvDepth > 0.0f)
        ret = bestTp;
    return ret;
}

void                GridTriangles::intersects_(
    Vec2F                   pos,
    TriPoints &             ret)
    const
{
    ret.clear();
    Vec2F               gridCoord = clientToGridPacs * pos;
    if (!isInUpperBounds(grid.dims(),gridCoord))
        return;
    Vec2UI              binIdx = Vec2UI(gridCoord);
    Uints const &       bin = grid[binIdx];
    for (size_t ii=0; ii<bin.size(); ++ii) {
        TriPoint            tp;
        tp.triInd = bin[ii];
        tp.vertInds = tris[bin[ii]];
        Opt<Arr3F>          vbc;
        // All tris in index guaranteed to have valid vertex projection values:
        Arr<Vec2F,3>        p = mapIndex(tp.vertInds,verts);
        vbc = cBarycentricCoord(pos,p);
        if (vbc.has_value()) {
            tp.baryCoord = vbc.value();
            if (cMinElem(tp.baryCoord) >= 0.0f)
                ret.push_back(tp);
        }
    }
}

void                testGridTriangles(CLArgs const &)
{
    // Create a triangular patch of tris:
    const uint          dim = 10,
                        dimp = dim+1;
    Img2F               vertImg(dimp,dimp);
    for (Iter2UI vit(dimp); vit.valid(); vit.next())
        vertImg[vit()] = Vec2F(vit());
    Arr3UIs             tris;
    for (uint row=0; row<dim; ++row) {
        uint                col=0;
        for (; col<row; ++col) {
            tris.push_back(Arr3UI(row*dimp+col,row*dimp+col+1,(row+1)*dimp+col+1));
            tris.push_back(Arr3UI((row+1)*dimp+col+1,(row+1)*dimp+col,row*dimp+col));
        }
        tris.push_back(Arr3UI((row+1)*dimp+col+1,(row+1)*dimp+col,row*dimp+col));
    }
    // Create the grid and query:
    const Vec2Fs &      verts = vertImg.dataVec();
    GridTriangles       gts {verts,tris};
    for (uint ii=0; ii<100; ++ii) {
        Vec2D               posd(cRandUniform(),cRandUniform());
        Vec2F               pos = Vec2F(posd) * 10.0f;
        vector<TriPoint>    res = gts.intersects(pos);
        if (pos[0] < pos[1]) {
            FGASSERT(res.size() == 1);
            TriPoint &          isect(res[0]);
            Vec2F               rpos {
                    verts[isect.vertInds[0]] * isect.baryCoord[0] +
                    verts[isect.vertInds[1]] * isect.baryCoord[1] +
                    verts[isect.vertInds[2]] * isect.baryCoord[2]
            };
            FGASSERT(isApproxEqual(pos.m,rpos.m,epsBits(20)));
        }
        else
            FGASSERT(res.size() == 0);
    }
    // Query outside grid area:
    vector<TriPoint> res = gts.intersects(Vec2F(-0.1f,0.0f));
    FGASSERT(res.size() == 0);
    res = gts.intersects(Vec2F(10.1f,0.0f));
    FGASSERT(res.size() == 0);
    res = gts.intersects(Vec2F(5.0f,-0.1f));
    FGASSERT(res.size() == 0);
    res = gts.intersects(Vec2F(5.0f,10.1f));
    FGASSERT(res.size() == 0);
}

}

// */
