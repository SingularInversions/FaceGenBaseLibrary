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
    double      mag;    // Squared magnitude of vec

    FgVecMag() : mag(0) {}
};

// Returns closest point in given line segment from origin:
FgVecMag
fgClosestPointInSegment(FgVect3D p0,FgVect3D p1);

// Returns delta from point to tri:
FgVecMag
fgClosestPointInTri(
    FgVect3D point,
    FgVect3D vert0,
    FgVect3D vert1,
    FgVect3D vert2);

// Returns the barycentric coord of point in triangle
// (invalid if outside of triangle or triangle degenerate):
FgOpt<FgVect3D>
fgBarycentricCoords(
    FgVect2D            point,
    FgVect2D            v0,
    FgVect2D            v1,
    FgVect2D            v2);

// Returns the barycentric coord of a point relative to a triangle in 3D.
// Only works for planar points.
// Returns invalid if triangle degenerate.
FgOpt<FgVect3D>
fgBarycentricCoords(
    FgVect3D point,
    FgVect3D vert0,
    FgVect3D vert1,
    FgVect3D vert2);

inline
FgOpt<FgVect3D>
fgBarycentricCoords(FgVect2F point,FgVect2F v0,FgVect2F v1,FgVect2F v2)
{return fgBarycentricCoords(FgVect2D(point),FgVect2D(v0),FgVect2D(v1),FgVect2D(v2)); }

// Homogenous plane representation from 3 points:
FgVect4D
fgPlaneH(FgVect3D p0,FgVect3D p1,FgVect3D p2);

// A ray is a line through the origin (Homog not necessary), intersected with a homog plane:
FgVect4D
fgLinePlaneIntersect(FgVect3D ray,FgVect4D plane);

// Returns: 0: point not in triangle or degenerate triangle.
//          1: point in triangle, CC winding
//          -1: point in triangle, CW winding
int
fgPointInTriangle(FgVect2D pt,FgVect2D v0,FgVect2D v1,FgVect2D v2);

FgOpt<FgVect3D>
fgLineFacetIntersect(FgVect3D pnt,FgVect3D dir,FgVect3D v0,FgVect3D v1,FgVect3D v2);

inline
double
pointToPlaneDistSqr(FgVect3D pnt,FgVect4D planeH)
{
    FgVect3D    planeN(planeH[0],planeH[1],planeH[2]);
    return (fgSqr(fgDot(pnt,planeN) + planeH[3]) / planeN.lengthSqr());
}

#endif
