//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     April 10, 2010
//
// 2D grid spatial index for point-triangle intersections

#ifndef FG_GRIDTRIANGLES_HPP
#define FG_GRIDTRIANGLES_HPP

#include "FgImage.hpp"
#include "FgAffineCwC.hpp"

struct  FgTriPoint
{
    uint        triInd;         // Index of triangle in list
    // Indices of vertices of triangle in winding order (redundant to above for efficiency):
    FgVect3UI   pointInds;
    // Barycentric coordinate of intersection point in projected coordinates. Note that that harmonic interpolation
    // must be used to linaerly interpolate attributes in the original space:
    FgVect3F    baryCoord;
};

struct  FgGridTriangles
{
    FgAffineCw2F            clientToGridIpcs;
    FgImage<FgUints>        grid;               // Bins of indices into client triangle array

    FgOpt<FgTriPoint>
    nearestIntersect(
        const FgVect3UIs &  tris,           // Must be same list used to initialize index
        const FgVect2Fs &   verts,          // "
        const FgFlts &      inverseDepths,  // 1/Z values. Must be 1-1 with 'verts'
        FgVect2F            pos) const;

    // NB: If 'pos' lies outside the bounds specified during construction no intersections will be computed:
    void
    intersects_(
        const FgVect3UIs &  tris,       // Must be same list used to initialize index
        const FgVect2Fs &   verts,      // "
        FgVect2F            pos,
        vector<FgTriPoint> & ret) const;

    vector<FgTriPoint>
    intersects(
        const FgVect3UIs &  tris,       // Must be same list used to initialize index
        const FgVect2Fs &   verts,      // "
        FgVect2F pos) const
    {
        vector<FgTriPoint>   ret;
        intersects_(tris,verts,pos,ret);
        return ret;
    }
};

FgGridTriangles
fgGridTriangles(
    // tris containing invalid verts [max,max] will not be indexed:
    const FgVect2Fs &   verts,
    const FgVect3UIs &  tris,       // Indices into 'verts'
    float               binsPerTri=1.0f);

#endif

// */
