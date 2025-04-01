//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// 2D grid spatial index

#ifndef FG_GRIDINDEX_HPP
#define FG_GRIDINDEX_HPP

#include "FgImage.hpp"

namespace Fg {

template<typename T>
struct      GridIndex
{
    AxAffine2F          clientToGridPacs;
    Img<Svec<T>>        grid;       // Bins of client objects (bins not exactly square)
    Svec<T> const       empty;

    GridIndex(AxAffine2F toGridPacs,Vec2UI gridDims) :
        clientToGridPacs{toGridPacs},
        grid{gridDims}
    {}
    // automatically determine grid dimensions by roughly equating bins with the number of lookup objects:
    GridIndex(Rect2F clientDomain,size_t approxNumBins)
    {
        FGASSERT(approxNumBins > 0);
        FGASSERT(clientDomain.volume() > 0);
        double              scaleToBins = std::sqrt(scast<double>(approxNumBins)/clientDomain.volume());
        Vec2F               gridSizef = clientDomain.dims * float(scaleToBins);
        Vec2UI              gridSize {gridSizef  + Vec2F{0.5f}};
        gridSize = mapMax(gridSize,1U);
        Rect2F              gridDomain {{0,0},Vec2F{gridSize}};
        clientToGridPacs = {clientDomain,gridDomain};
        grid.resize(gridSize);
    }

    // returns false if the bounds are entirely outside the grid and the value discarded. True otherwise.
    bool                add(T const & val,Mat22F clientBounds)
    {
        Mat22F          pacsBounds = clientToGridPacs * clientBounds;
        pacsBounds[0] = cMax(pacsBounds[0],0.0f);
        pacsBounds[2] = cMax(pacsBounds[2],0.0f);
        if ((pacsBounds[0] > pacsBounds[1]) || (pacsBounds[2] > pacsBounds[3]))
            return false;
        Mat22UI         ircsBounds = Mat22UI(pacsBounds);         // All elements now guaranteed  positive
        ircsBounds[1] = cMin(ircsBounds[1]+1,grid.width());        // Convert to exlusive upper bounds (EUB)
        ircsBounds[3] = cMin(ircsBounds[3]+1,grid.height());       // and clip to grid.
        for (uint yy=ircsBounds[2]; yy<ircsBounds[3]; ++yy) {      // Invalid bounds implicity skipped
            for (uint xx=ircsBounds[0]; xx<ircsBounds[1]; ++xx)
                grid.xy(xx,yy).push_back(val);
        }
        return true;
    }

    Svec<T> const &     operator[](Vec2F const & clientPos) const
    {
        Vec2F           posPacs = clientToGridPacs*clientPos;
        if ((posPacs[0] < 0.0f) || (posPacs[1] < 0.0f))
            return empty;
        Vec2UI          posIrcs = Vec2UI(posPacs);
        if ((posIrcs[0] < grid.width()) && (posIrcs[1] < grid.height()))
            return grid[posIrcs];
        return empty;
    }
};

struct  TriPoint
{
    uint                triInd;         // Index of triangle in list
    // Indices of vertices of triangle in winding order (redundant to above; for efficiency):
    Arr3UI              vertInds;
    // Barycentric coordinate of intersection point in projected coordinates. Note that that
    // harmonic interpolation must be used to linaerly interpolate attributes in the original space:
    Arr3F               baryCoord;
};
typedef Svec<TriPoint>  TriPoints;

struct  GridTriangles
{
    AxAffine2F          clientToGridPacs;
    Img<Uints>          grid;               // Bins of indices into 'tris'
    Vec2Fs              verts;              // in client CS
    Arr3UIs             tris;               // indices into 'verts'

    GridTriangles() {}
    GridTriangles(
        Vec2Fs const &      verts_,         // tris containing invalid verts [max,max] will not be indexed
        Arr3UIs const &     tris_,          // indices into 'verts'
        float               binsPerTri=1.0f);

    Opt<TriPoint>       nearestIntersect(
        Floats const &      inverseDepths,  // 1/Z values. Must be 1-1 with 'verts'
        Vec2F               pos)
        const;

    // Returns an unsorted list of all tris encompassing 'pos'
    void                intersects_(
        // If 'pos' lies outside the bounds specified during construction no intersections will be computed:
        Vec2F               pos,            // in client coordinate system (ie. of 'verts' used to create this)
        TriPoints &         ret)
        const;

    TriPoints           intersects(Vec2F pos) const
    {
        TriPoints         ret;
        intersects_(pos,ret);
        return ret;
    }
};

}

#endif

// */
