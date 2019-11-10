//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGGEOMETRY_HPP
#define FGGEOMETRY_HPP

#include "FgStdLibs.hpp"
#include "FgQuaternion.hpp"

namespace Fg {

struct  FgVecMag
{
    Vec3D    vec;
    double      mag;    // Squared magnitude of vec. Initialized to invalid.

    FgVecMag() : mag(std::numeric_limits<double>::max()) {}

    bool valid() const
    {return (mag != std::numeric_limits<double>::max()); }
};

// Returns closest point in given line segment from origin:
FgVecMag
fgClosestPointInSegment(Vec3D p0,Vec3D p1);

// Returns delta from point to tri:
FgVecMag
fgClosestPointInTri(Vec3D point,Vec3D vert0,Vec3D vert1,Vec3D vert2);

// Returns the barycentric coord of point relative to triangle.
// If no valid value, triangle is degenerate.
// Point is in triangle if all coordinates are positive:
Opt<Vec3D>
fgBarycentricCoords(Vec2D point,Vec2D v0,Vec2D v1,Vec2D v2);

// Returns the barycentric coord of a point relative to a triangle in 3D.
// Only works for planar points.
// Returns invalid if triangle degenerate.
Opt<Vec3D>
fgBarycentricCoords(Vec3D point,Vec3D vert0,Vec3D vert1,Vec3D vert2);

inline
Opt<Vec3D>
fgBarycentricCoords(Vec2F point,Vec2F v0,Vec2F v1,Vec2F v2)
{return fgBarycentricCoords(Vec2D(point),Vec2D(v0),Vec2D(v1),Vec2D(v2)); }

// Homogenous plane representation from 3 points on plane:
Vec4D
fgPlaneH(Vec3D p0,Vec3D p1,Vec3D p2);

// Returns the homogeneous coordinate of the intersection of a line through the origin with a plane.
// The homogeneous component will be zero if there is no intersection. Otherwise, the dot product
// of the intersection and the ray will determine the direction (along ray) to intersection.
Vec4D
fgLinePlaneIntersect(
    Vec3D        ray,        // Direction of ray emanating from origin. Does not need to be normalized
    Vec4D        plane);     // Homogenous representation

// Returns: 0: point not in triangle or degenerate triangle.
//          1: point in triangle, CC winding
//          -1: point in triangle, CW winding
int
fgPointInTriangle(Vec2D pt,Vec2D v0,Vec2D v1,Vec2D v2);

// Returns the intersection point of a line and a triangle, if it exists, in either direction:
Opt<Vec3D>
fgLineTriIntersect(
    Vec3D        point,      // Point on line
    Vec3D        ray,        // Direction of ray emanating from point. Does not need to be normalized
    Vec3D        v0,         // Vertices of triangle
    Vec3D        v1,         // "
    Vec3D        v2);        // "

inline
double
pointToPlaneDistSqr(Vec3D pnt,Vec4D planeH)
{
    Vec3D    planeN(planeH[0],planeH[1],planeH[2]);
    return (sqr(cDot(pnt,planeN) + planeH[3]) / planeN.mag());
}

}

#endif
