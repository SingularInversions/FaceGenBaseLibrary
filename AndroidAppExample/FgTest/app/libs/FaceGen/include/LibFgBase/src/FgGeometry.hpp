//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Sept 20, 2005
//

#ifndef FGGEOMETRY_HPP
#define FGGEOMETRY_HPP

#include "FgStdLibs.hpp"
#include "FgQuaternion.hpp"

struct  FgVecMag
{
    FgVect3D    vec;
    double      mag;    // Squared magnitude of vec. Initialized to invalid.

    FgVecMag() : mag(std::numeric_limits<double>::max()) {}

    bool valid() const
    {return (mag != std::numeric_limits<double>::max()); }
};

// Returns closest point in given line segment from origin:
FgVecMag
fgClosestPointInSegment(FgVect3D p0,FgVect3D p1);

// Returns delta from point to tri:
FgVecMag
fgClosestPointInTri(FgVect3D point,FgVect3D vert0,FgVect3D vert1,FgVect3D vert2);

// Returns the barycentric coord of point relative to triangle.
// If no valid value, triangle is degenerate.
// Point is in triangle if all coordinates are positive:
FgOpt<FgVect3D>
fgBarycentricCoords(FgVect2D point,FgVect2D v0,FgVect2D v1,FgVect2D v2);

// Returns the barycentric coord of a point relative to a triangle in 3D.
// Only works for planar points.
// Returns invalid if triangle degenerate.
FgOpt<FgVect3D>
fgBarycentricCoords(FgVect3D point,FgVect3D vert0,FgVect3D vert1,FgVect3D vert2);

inline
FgOpt<FgVect3D>
fgBarycentricCoords(FgVect2F point,FgVect2F v0,FgVect2F v1,FgVect2F v2)
{return fgBarycentricCoords(FgVect2D(point),FgVect2D(v0),FgVect2D(v1),FgVect2D(v2)); }

// Return the position of the barycentric coordinate given the triangle indices and vertex list:
inline
FgVect3F
fgBarycentricToPos(const FgVect3Fs & verts,FgVect3UI vertInds,FgVect3F baryCoord)
{return (baryCoord[0]*verts[vertInds[0]] + baryCoord[1]*verts[vertInds[1]] + baryCoord[2]*verts[vertInds[2]]); }

// Homogenous plane representation from 3 points on plane:
FgVect4D
fgPlaneH(FgVect3D p0,FgVect3D p1,FgVect3D p2);

// Returns the homogeneous coordinate of the intersection of a line through the origin with a plane.
// The homogeneous component will be zero if there is no intersection. Otherwise, the dot product
// of the intersection and the ray will determine the direction (along ray) to intersection.
FgVect4D
fgLinePlaneIntersect(
    FgVect3D        ray,        // Direction of ray emanating from origin. Does not need to be normalized
    FgVect4D        plane);     // Homogenous representation

// Returns: 0: point not in triangle or degenerate triangle.
//          1: point in triangle, CC winding
//          -1: point in triangle, CW winding
int
fgPointInTriangle(FgVect2D pt,FgVect2D v0,FgVect2D v1,FgVect2D v2);

// Returns the intersection point of a line and a triangle, if it exists, in either direction:
FgOpt<FgVect3D>
fgLineTriIntersect(
    FgVect3D        point,      // Point on line
    FgVect3D        ray,        // Direction of ray emanating from point. Does not need to be normalized
    FgVect3D        v0,         // Vertices of triangle
    FgVect3D        v1,         // "
    FgVect3D        v2);        // "

inline
double
pointToPlaneDistSqr(FgVect3D pnt,FgVect4D planeH)
{
    FgVect3D    planeN(planeH[0],planeH[1],planeH[2]);
    return (fgSqr(fgDot(pnt,planeN) + planeH[3]) / planeN.mag());
}

#endif
