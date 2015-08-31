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

void
FgGridTriangles::init(
    const std::vector<FgVect2F> &   points,
    const std::vector<FgVect3UI> &  tris,
    float                           binSampleRatio)
{
    m_points = points;
    m_tris = tris;
    FgMat22F bounds = fgBounds(m_points);
    FgVect2F    lo = bounds.colVec(0),
                size = bounds.colVec(1)-lo;
    FGASSERT(fgMinElem(size) > 0.0f);
    FGASSERT(tris.size() > 0);
    float       count = float(tris.size()) * binSampleRatio,
                widHgtRatio = size[0] / size[1];
    uint        dimX = uint(std::sqrt(count*widHgtRatio) + 0.5f),
                dimY = uint(std::sqrt(count/widHgtRatio) + 0.5f);
    FgVect2UI   dims((dimX > 0) ? dimX : 1,(dimY > 0) ? dimY : 1);
    // Ensure the upper bound points fall into the grid by adding 4 epsilons to ensure
    // that d/(s+4e) * d < d. Note that it is important that FgAffineCwPre2F applies the
    // translation before the scaling (instead of premultiplying a post-translation) otherwise
    // the inequality may no longer hold:
    size *= 1.0f + 4.0f * std::numeric_limits<float>::epsilon();
    FgVect2F    scales(float(dims[0])/size[0],float(dims[1])/size[1]);
    m_clientToGridCoords = FgAffineCwPre2F(-lo,scales);
    m_grid.resize(dims);
    for (size_t ii=0; ii<tris.size(); ++ii) {
        FgVect3UI       tri = tris[ii];
        FgMat22UI
            bounds = 
                FgMat22UI(
                    fgBounds(
                        m_clientToGridCoords * m_points[tri[0]],
                        m_clientToGridCoords * m_points[tri[1]],
                        m_clientToGridCoords * m_points[tri[2]]));
        for (FgIter2UI it(bounds); it.valid(); it.next())
            m_grid[it()].push_back(uint(ii));
    }
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
        FgVect3UI       tri = m_tris[bin[ii]];
        FgVect3F        bcoord =
            FgVect3F(fgBarycentricCoords(pos,m_points[tri[0]],m_points[tri[1]],m_points[tri[2]]).val());
        if (fgMinElem(bcoord) >= 0.0f)
            ret.push_back(FgTriPoint(bin[ii],tri,bcoord));
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
    for (uint row=0; row<dim; ++row)
    {
        uint    col=0;
        for (; col<row; ++col)
        {
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
    FgGridTriangles      grid;
    grid.init(verts,tris);
    for (uint ii=0; ii<100; ++ii)
    {
        FgVect2D        posd(fgRand(),fgRand());
        FgVect2F        pos = FgVect2F(posd) * 10.0f;
        vector<FgTriPoint> res = grid.intersects(pos);
        if (pos[0] < pos[1])
        {
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
