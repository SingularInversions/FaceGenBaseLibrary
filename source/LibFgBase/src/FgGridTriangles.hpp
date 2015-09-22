//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     April 10, 2010
//
// 2D grid spatial index for point-triangle intersections
//
// POSSIBLE SPEEDUPS:
//
// * Use a rasterization algorithm to only store tris in precisly intersected bins
//   rather than all bins in vertex bounds.
// * Store the tris directly by duplicating the vert values.

#ifndef FG_GRIDTRIANGLES_HPP
#define FG_GRIDTRIANGLES_HPP

#include "FgImage.hpp"
#include "FgAffineCwPreC.hpp"

struct  FgTriPoint
{
    uint        triInd;
    FgVect3UI   pointInds;
    FgVect3F    baryCoord;
};

struct  FgGridTriangles
{
    vector<FgVect2F>        m_points;           // Client coordinates
    vector<FgVect3UI>       m_tris;             // Indices into m_verts
    FgAffineCwPre2F         m_clientToGridCoords;
    FgImage<vector<uint> >  m_grid;             // Bins of indices into m_tris

    void
    intersects(FgVect2F pos,vector<FgTriPoint> & ret) const;

    vector<FgTriPoint>
    intersects(FgVect2F pos) const
    {
        vector<FgTriPoint>   ret;
        intersects(pos,ret);
        return ret;
    }
};

FgGridTriangles
fgGridTriangles(
    const vector<FgVect2F> &    points,
    const vector<FgVect3UI> &   tris,
    float                       binsPerTri=1.0f);

#endif

// */
