//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
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

Opt<TriPoint>
GridTriangles::nearestIntersect(Vec3UIs const & tris,Vec2Fs const & verts,const Floats & invDepths,Vec2F pos) const
{
    Opt<TriPoint>   ret;
    Vec2F            gridCoord = clientToGridIpcs * pos;
    if (!boundsIncludes(grid.dims(),gridCoord))
        return ret;
    Vec2UI           binIdx = Vec2UI(gridCoord);
    const Uints &     bin = grid[binIdx];
    float               bestInvDepth = 0.0f;
    TriPoint          bestTp;
    for (size_t ii=0; ii<bin.size(); ++ii) {
        Vec3UI           tri = tris[bin[ii]];
        Vec2F            v0 = verts[tri[0]],
                            v1 = verts[tri[1]],
                            v2 = verts[tri[2]];
        Opt<Vec3D>     vbc = barycentricCoord(pos,v0,v1,v2);
        if (vbc.valid() && (cMinElem(vbc.val()) >= 0)) {       // We intersect:
            Vec3F        ids(invDepths[tri[0]],invDepths[tri[1]],invDepths[tri[2]]),
                            bc = Vec3F(vbc.val());
            float           invDepth = cDot(bc,ids);           // Interpolation in projected values is harmonic
            if (invDepth > bestInvDepth) {                      // Closer on same ray
                bestInvDepth = invDepth;
                bestTp.triInd = bin[ii];
                bestTp.pointInds = tri;
                bestTp.baryCoord = bc;
            }
        }
    }
    if (bestInvDepth > 0.0f)
        ret = bestTp;
    return ret;
}

void
GridTriangles::intersects_(Vec3UIs const & tris,Vec2Fs const & verts,Vec2F pos,vector<TriPoint> & ret) const
{
    ret.clear();
    Vec2F            gridCoord = clientToGridIpcs * pos;
    if (!boundsIncludes(grid.dims(),gridCoord))
        return;
    Vec2UI           binIdx = Vec2UI(gridCoord);
    const Uints &     bin = grid[binIdx];
    for (size_t ii=0; ii<bin.size(); ++ii) {
        TriPoint      tp;
        tp.triInd = bin[ii];
        tp.pointInds = tris[bin[ii]];
        Opt<Vec3D>    vbc;
        // All tris in index guaranteed to have valid vertex projection values:
        Vec2F            p0 = verts[tp.pointInds[0]],
                            p1 = verts[tp.pointInds[1]],
                            p2 = verts[tp.pointInds[2]];
        vbc = barycentricCoord(pos,p0,p1,p2);
        if (vbc.valid()) {
            tp.baryCoord = Vec3F(vbc.val());
            if (cMinElem(tp.baryCoord) >= 0.0f)
                ret.push_back(tp);
        }
    }
}

GridTriangles
gridTriangles(Vec2Fs const & verts,Vec3UIs const & tris,float binsPerTri)
{
    GridTriangles     ret;
    FGASSERT(tris.size() > 0);
    float               fmax = maxFloat();
    Vec2F            domainLo(fmax),
                        domainHi(-fmax),
                        invalid(fmax);
    size_t              numValid = 0;
    for (size_t ii=0; ii<verts.size(); ++ii) {
        Vec2F        v = verts[ii];
        if (v != invalid) {
            for (uint dd=0; dd<2; ++dd) {
                setIfLess(domainLo[dd],v[dd]);
                setIfGreater(domainHi[dd],v[dd]);
            }
            ++numValid;
        }
    }
    Vec2F    domainSz = domainHi - domainLo;
    FGASSERT((domainSz[0] > 0) && (domainSz[1] > 0));
    float       numBins = numValid * binsPerTri;
    Vec2F    rangeSizef = domainSz * sqrt(numBins/domainSz.cmpntsProduct());
    Vec2UI   rangeSize = Vec2UI(rangeSizef + Vec2F(0.5f));
    rangeSize = clampLo(rangeSize,1U);
    Mat22F    range(0,rangeSize[0],0,rangeSize[1]);
    // We could in theory intersect the client's desired sampling domain with the verts domain but
    // this optimization currently represents an unlikely case; we usually want to fit what we're
    // rendering on the image. This would change for more general-purpose ray casting.
    ret.clientToGridIpcs = AffineEw2F(catHoriz(domainLo,domainHi),range);
    ret.grid.resize(rangeSize);
    for (size_t ii=0; ii<tris.size(); ++ii) {
        Vec3UI       tri = tris[ii];
        Vec2F        p0 = verts[tri[0]],
                        p1 = verts[tri[1]],
                        p2 = verts[tri[2]];
        if ((p0 != invalid) && (p1 != invalid) && (p2 != invalid)) {
            Mat22F    projBounds = fgInclToExcl(cBounds(
                ret.clientToGridIpcs * p0,
                ret.clientToGridIpcs * p1,
                ret.clientToGridIpcs * p2));
            if (boundsIntersect(projBounds,range,projBounds)) {
                Mat22UI       bnds = Mat22UI(projBounds);
                for (Iter2UI it(bnds); it.valid(); it.next())
                    ret.grid[it()].push_back(uint(ii));
            }
        }
    }
    return ret;
}

void
fgGridTrianglesTest(CLArgs const &)
{
    // Create a triangular patch of tris:
    const uint                  dim = 10,
                                dimp = dim+1;
    Img2F           vertImg(dimp,dimp);
    for (Iter2UI vit(dimp); vit.valid(); vit.next())
        vertImg[vit()] = Vec2F(vit());
    vector<Vec3UI>           tris;
    for (uint row=0; row<dim; ++row) {
        uint    col=0;
        for (; col<row; ++col) {
            tris.push_back(Vec3UI(row*dimp+col,row*dimp+col+1,(row+1)*dimp+col+1));
            tris.push_back(Vec3UI((row+1)*dimp+col+1,(row+1)*dimp+col,row*dimp+col));
        }
        tris.push_back(Vec3UI((row+1)*dimp+col+1,(row+1)*dimp+col,row*dimp+col));
    }
    // Create the grid and query:
    const vector<Vec2F> &    verts = vertImg.dataVec();
    GridTriangles             gts = gridTriangles(verts,tris);
    for (uint ii=0; ii<100; ++ii) {
        Vec2D        posd(randUniform(),randUniform());
        Vec2F        pos = Vec2F(posd) * 10.0f;
        vector<TriPoint> res = gts.intersects(tris,verts,pos);
        if (pos[0] < pos[1]) {
            FGASSERT(res.size() == 1);
            TriPoint &   isect(res[0]);
            Vec2F
                rpos(
                    verts[isect.pointInds[0]] * isect.baryCoord[0] +
                    verts[isect.pointInds[1]] * isect.baryCoord[1] +
                    verts[isect.pointInds[2]] * isect.baryCoord[2]);
            FGASSERT(isApproxEqualRelMag(pos,rpos,30));
        }
        else
            FGASSERT(res.size() == 0);
    }
    // Query outside grid area:
    vector<TriPoint> res = gts.intersects(tris,verts,Vec2F(-0.1f,0.0f));
    FGASSERT(res.size() == 0);
    res = gts.intersects(tris,verts,Vec2F(10.1f,0.0f));
    FGASSERT(res.size() == 0);
    res = gts.intersects(tris,verts,Vec2F(5.0f,-0.1f));
    FGASSERT(res.size() == 0);
    res = gts.intersects(tris,verts,Vec2F(5.0f,10.1f));
    FGASSERT(res.size() == 0);
}

}

// */
