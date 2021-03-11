//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGGEOMETRY_HPP
#define FGGEOMETRY_HPP

#include "FgStdLibs.hpp"
#include "FgQuaternion.hpp"

namespace Fg {

// Returns the barycentric coordinate (wrt the input points) of the closest point in the plane
// spanned by the points to the origin.
Vec3D
closestBarycentricPoint(Vec3D p0,Vec3D p1,Vec3D p2);

template<typename T>
struct  VecMag
{
    Mat<T,3,1>  vec;
    T           mag;    // Squared magnitude of vec. Initialized to invalid.

    VecMag() : mag {std::numeric_limits<T>::max()} {}
    VecMag(Mat<T,3,1> v,T m) : vec(v), mag(m) {}

    bool valid() const
    {return (mag != std::numeric_limits<T>::max()); }
};
typedef VecMag<float>   VecMagF;
typedef VecMag<double>  VecMagD;

// Returns closest point in given line segment from origin:
VecMagD
closestPointInSegment(Vec3D p0,Vec3D p1);

// Returns delta from point to tri:
VecMagD
closestPointInTri(Vec3D const & point,Vec3D const & vert0,Vec3D const & vert1,Vec3D const & vert2);

// Returns the barycentric coord of point relative to triangle.
// If no valid value, triangle is degenerate.
// Point is in triangle if all coordinates are positive:
Opt<Vec3D>
barycentricCoord(Vec2D point,Vec2D v0,Vec2D v1,Vec2D v2);

// Returns the barycentric coord of a point relative to a triangle in 3D.
// Only works for planar points.
// Returns invalid if triangle degenerate.
Opt<Vec3D>
barycentricCoord(Vec3D point,Vec3D vert0,Vec3D vert1,Vec3D vert2);

inline
Opt<Vec3D>
barycentricCoord(Vec2F point,Vec2F v0,Vec2F v1,Vec2F v2)
{return barycentricCoord(Vec2D(point),Vec2D(v0),Vec2D(v1),Vec2D(v2)); }

struct  Plane
{
    Vec3D           norm;       // Plane normal, NOT unit scaled
    double          scalar;     // s such that: N * x + s = 0
};

// Plane from 3 points. Throws if points are colinear or coincident:
Plane
cPlane(Vec3D p0,Vec3D p1,Vec3D p2);

// Returns the homogeneous coordinate of the intersection of a line through the origin with a plane.
// The homogeneous component will be zero if there is no intersection. Otherwise, the dot product
// of the intersection and the ray will determine the direction (along ray) to intersection.
Vec4D
linePlaneIntersect(
    Vec3D           ray,        // Direction of ray emanating from origin. Does not need to be normalized
    Plane           plane);

// Returns: 0: point not in triangle or degenerate triangle.
//          1: point in triangle, CC winding
//          -1: point in triangle, CW winding
int
pointInTriangle(Vec2D pt,Vec2D v0,Vec2D v1,Vec2D v2);

// Returns the intersection point of a line and a triangle, if it exists, in either direction:
Opt<Vec3D>
lineTriIntersect(
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
