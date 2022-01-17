//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// 2D grid spatial index for point-triangle intersections

#ifndef FG_GRIDTRIANGLES_HPP
#define FG_GRIDTRIANGLES_HPP

#include "FgImage.hpp"
#include "FgAffine.hpp"

namespace Fg {

struct  TriPoint
{
    uint        triInd;         // Index of triangle in list
    // Indices of vertices of triangle in winding order (redundant to above for efficiency):
    Vec3UI      vertInds;
    // Barycentric coordinate of intersection point in projected coordinates. Note that that harmonic interpolation
    // must be used to linaerly interpolate attributes in the original space:
    Vec3F       baryCoord;
};
typedef Svec<TriPoint>      TriPoints;

struct  GridTriangles
{
    AffineEw2F              clientToGridIpcs;
    Img<Uints>              grid;               // Bins of indices into client triangle array

    Opt<TriPoint>
    nearestIntersect(
        Vec3UIs const &     tris,           // Must be same list used to initialize index
        Vec2Fs const &      verts,          // "
        Floats const &      inverseDepths,  // 1/Z values. Must be 1-1 with 'verts'
        Vec2F               pos) const;

    // NB: If 'pos' lies outside the bounds specified during construction no intersections will be computed:
    void
    intersects_(
        Vec3UIs const &     tris,       // Must be same list used to initialize index
        Vec2Fs const &      verts,      // "
        Vec2F               pos,
        TriPoints &         ret) const;    // Returns an unsorted list of all tris encompassing 'pos'

    TriPoints
    intersects(Vec3UIs const & tris,Vec2Fs const & verts,Vec2F pos) const
    {
        TriPoints         ret;
        intersects_(tris,verts,pos,ret);
        return ret;
    }
};

GridTriangles
gridTriangles(
    // tris containing invalid verts [max,max] will not be indexed:
    Vec2Fs const &      verts,
    Vec3UIs const &     tris,       // Indices into 'verts'
    float               binsPerTri=1.0f);

}

#endif

// */
