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

FgGridTriangles
fgGridTriangles(
    const vector<FgVect2F> &    points,
    const vector<FgVect3UI> &   tris,
    float                       binsPerTri)
{
    FgGridTriangles         ret;
    ret.m_points = points;
    ret.m_tris = tris;
    FgMat22F    domain = fgBounds(ret.m_points);
    FgVect2F    domainLo = domain.colVec(0),
                domainSize = domain.colVec(1)-domainLo;
    FGASSERT(domainSize.volume() > 0.0f);
    FGASSERT(tris.size() > 0);
    float       numBins = float(tris.size()) * binsPerTri;
    FgVect2F    rangeSizef = domainSize * std::sqrt(numBins/domainSize.volume());
    FgVect2UI   rangeSize = FgVect2UI(rangeSizef + FgVect2F(0.5f));
    rangeSize = fgClipLo(rangeSize,1U);
    FgMat22F    range = fgConcatHoriz(FgVect2F(0.01f),FgVect2F(rangeSize)-FgVect2F(0.01f));
    // Ensure all points fall into the grid despite rounding error by leaving a 1% pixel boundary:
    ret.m_clientToGridCoords = FgAffineCwPre2F(domain,range);
    ret.m_grid.resize(rangeSize);
    for (size_t ii=0; ii<tris.size(); ++ii) {
        FgVect3UI       tri = tris[ii];
        FgMat22UI       bounds = FgMat22UI(fgBounds(
            ret.m_clientToGridCoords * ret.m_points[tri[0]],
            ret.m_clientToGridCoords * ret.m_points[tri[1]],
            ret.m_clientToGridCoords * ret.m_points[tri[2]]));
        for (FgIter2UI it(bounds); it.valid(); it.next())
            ret.m_grid[it()].push_back(uint(ii));
    }
    return ret;
}

void
FgGridTriangles::intersects(FgVect2F pos,vector<FgTriPoint> & ret) const
{
    ret.clear();
    FgVect2F            gridCoord = m_clientToGridCoords * pos;
    if ((fgMinElem(gridCoord) < 0.0f) ||
        (gridCoord[0] >= float(m_grid.width())) ||
        (gridCoord[1] >= float(m_grid.height())))
        return;
    FgVect2UI           binIdx = FgVect2UI(gridCoord);
    const vector<uint> & bin = m_grid[binIdx];
    for (size_t ii=0; ii<bin.size(); ++ii) {
        FgTriPoint      tp;
        tp.triInd = bin[ii];
        tp.pointInds = m_tris[bin[ii]];
        FgOpt<FgVect3D>    vbc = fgBarycentricCoords(pos,
            m_points[tp.pointInds[0]],
            m_points[tp.pointInds[1]],
            m_points[tp.pointInds[2]]);
        if (vbc.valid()) {
            tp.baryCoord = FgVect3F(vbc.val());
            if (fgMinElem(tp.baryCoord) >= 0.0f)
                ret.push_back(tp);
        }
    }
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
            tris.push_back(
                FgVect3UI(
                    row*dimp+col,
                    row*dimp+col+1,
                    (row+1)*dimp+col+1));
            tris.push_back(
                FgVect3UI(
                    (row+1)*dimp+col+1,
                    (row+1)*dimp+col,
                    row*dimp+col));
        }
        tris.push_back(
            FgVect3UI(
                (row+1)*dimp+col+1,
                (row+1)*dimp+col,
                row*dimp+col));
    }
    // Create the grid and query:
    const vector<FgVect2F> &    verts = vertImg.dataVec();
    FgGridTriangles             grid = fgGridTriangles(verts,tris);
    for (uint ii=0; ii<100; ++ii) {
        FgVect2D        posd(fgRand(),fgRand());
        FgVect2F        pos = FgVect2F(posd) * 10.0f;
        vector<FgTriPoint> res = grid.intersects(pos);
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
    vector<FgTriPoint> res = grid.intersects(FgVect2F(-0.1f,0.0f));
    FGASSERT(res.size() == 0);
    res = grid.intersects(FgVect2F(10.1f,0.0f));
    FGASSERT(res.size() == 0);
    res = grid.intersects(FgVect2F(5.0f,-0.1f));
    FGASSERT(res.size() == 0);
    res = grid.intersects(FgVect2F(5.0f,10.1f));
    FGASSERT(res.size() == 0);
}

// */
