//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgGridTriangles.hpp"
#include "FgIter.hpp"
#include "FgGeometry.hpp"
#include "FgCommand.hpp"
#include "FgApproxEqual.hpp"

using namespace std;

namespace Fg {

GridTriangles::GridTriangles(Vec2Fs const & vs,Vec3UIs const & ts,float binsPerTri)
    : verts{vs}, tris{ts}
{
    FGASSERT(tris.size() > 0);
    Vec2F               domainLo(lims<float>::max()),
                        domainHi(-lims<float>::max()),
                        invalid(lims<float>::max());
    size_t              numValid = 0;
    for (size_t ii=0; ii<verts.size(); ++ii) {
        Vec2F               v = verts[ii];
        if (v != invalid) {
            for (uint dd=0; dd<2; ++dd) {
                updateMin_(domainLo[dd],v[dd]);
                updateMax_(domainHi[dd],v[dd]);
            }
            ++numValid;
        }
    }
    Vec2F               domainSz = domainHi - domainLo;
    FGASSERT((domainSz[0] > 0) && (domainSz[1] > 0));
    float               numBins = numValid * binsPerTri;
    Vec2F               rangeSizef = domainSz * sqrt(numBins/domainSz.cmpntsProduct());
    Vec2UI              rangeSize = Vec2UI(rangeSizef + Vec2F(0.5f));
    rangeSize = mapThreshLo(rangeSize,1U);
    Mat22F              range(0,rangeSize[0],0,rangeSize[1]);
    // We could in theory intersect the client's desired sampling domain with the verts domain but
    // this optimization currently represents an unlikely case; we usually want to fit what we're
    // rendering on the image. This would change for more general-purpose ray casting.
    clientToGridIpcs = AffineEw2F(catHoriz(domainLo,domainHi),range);
    grid.resize(rangeSize);
    for (size_t ii=0; ii<tris.size(); ++ii) {
        Vec3UI          tri = tris[ii];
        Vec2F           p0 = verts[tri[0]],
                        p1 = verts[tri[1]],
                        p2 = verts[tri[2]];
        if ((p0 != invalid) && (p1 != invalid) && (p2 != invalid)) {
            Mat22F          projBounds = iubToEub(cBounds(
                clientToGridIpcs * p0,
                clientToGridIpcs * p1,
                clientToGridIpcs * p2));
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
    Vec2F               gridCoord = clientToGridIpcs * pos;
    if (!isInUpperBounds(grid.dims(),gridCoord))
        return ret;
    Vec2UI              binIdx = Vec2UI(gridCoord);
    Uints const &       bin = grid[binIdx];
    float               bestInvDepth = 0.0f;
    TriPoint            bestTp;
    for (size_t ii=0; ii<bin.size(); ++ii) {
        Vec3UI              tri = tris[bin[ii]];
        Vec2F               v0 = verts[tri[0]],
                            v1 = verts[tri[1]],
                            v2 = verts[tri[2]];
        Opt<Vec3D>          vbc = cBarycentricCoord(pos,v0,v1,v2);
        if (vbc.has_value() && (cMinElem(vbc.value()) >= 0)) {       // We intersect:
            Vec3F               ids(invDepths[tri[0]],invDepths[tri[1]],invDepths[tri[2]]),
                                bc = Vec3F(vbc.value());
            float               invDepth = cDot(bc,ids);        // Interpolation in projected values is harmonic
            if (invDepth > bestInvDepth) {                      // Closer on same ray
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
    Vec2F               gridCoord = clientToGridIpcs * pos;
    if (!isInUpperBounds(grid.dims(),gridCoord))
        return;
    Vec2UI              binIdx = Vec2UI(gridCoord);
    Uints const &       bin = grid[binIdx];
    for (size_t ii=0; ii<bin.size(); ++ii) {
        TriPoint            tp;
        tp.triInd = bin[ii];
        tp.vertInds = tris[bin[ii]];
        Opt<Vec3D>          vbc;
        // All tris in index guaranteed to have valid vertex projection values:
        Vec2F               p0 = verts[tp.vertInds[0]],
                            p1 = verts[tp.vertInds[1]],
                            p2 = verts[tp.vertInds[2]];
        vbc = cBarycentricCoord(pos,p0,p1,p2);
        if (vbc.has_value()) {
            tp.baryCoord = Vec3F(vbc.value());
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
    Vec3UIs             tris;
    for (uint row=0; row<dim; ++row) {
        uint                col=0;
        for (; col<row; ++col) {
            tris.push_back(Vec3UI(row*dimp+col,row*dimp+col+1,(row+1)*dimp+col+1));
            tris.push_back(Vec3UI((row+1)*dimp+col+1,(row+1)*dimp+col,row*dimp+col));
        }
        tris.push_back(Vec3UI((row+1)*dimp+col+1,(row+1)*dimp+col,row*dimp+col));
    }
    // Create the grid and query:
    const Vec2Fs &      verts = vertImg.dataVec();
    GridTriangles       gts {verts,tris};
    for (uint ii=0; ii<100; ++ii) {
        Vec2D               posd(randUniform(),randUniform());
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
            FGASSERT(isApproxEqualRelMag(pos,rpos,30));
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
