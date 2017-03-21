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
// * Possible optimization: use a bin container that only allocates beyond a certain bin size.
//

#ifndef FG_GRIDTRIANGLES_HPP
#define FG_GRIDTRIANGLES_HPP

#include "FgImage.hpp"
#include "FgAffineCwC.hpp"

struct  FgTriPoint
{
    uint        triInd;
    FgVect3UI   pointInds;
    FgVect3F    baryCoord;
};

struct  FgGridTriangles
{
    FgAffineCw2F            clientToGridIpcs;
    FgImage<FgUints>        grid;               // Bins of indices into client triangle array

    FgOpt<FgTriPoint>
    nearestIntersect(
        const FgVect3UIs &  tris,       // Must be same list used to initialize index
        const FgVect2Fs &   verts,      // "
        const FgFlts &      depths,     // Must be 1-1 with 'verts'
        FgVect2F            pos) const;

    // NB: If 'pos' lies outside the bounds specified during construction no intersections will be computed:
    void
    intersects(
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
        intersects(tris,verts,pos,ret);
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
