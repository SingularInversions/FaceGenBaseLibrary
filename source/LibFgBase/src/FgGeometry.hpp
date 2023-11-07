//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGGEOMETRY_HPP
#define FGGEOMETRY_HPP

#include "FgAffine.hpp"

namespace Fg {

// Returns the signed area of the parallelogram defined by the points (RHR)
// which is twice the signed area of the triangle defined by the points (CC winding):
double              cArea(Vec2D p0,Vec2D p1,Vec2D p2);
inline double       cArea(Arr<Vec2D,3> const & t) {return cArea(t[0],t[1],t[2]); }

// Returns the vector area of the parallelogram defined by the points (RHR)
// which is twice the vector area of the triangle defined by the points (CC winding):
inline Vec3D        cArea(Vec3D p0,Vec3D p1,Vec3D p2) {return crossProduct(p1-p0,p2-p0); }

// hold a vector along with its magnitude
template<typename T,uint D>
struct      VecMag
{
    Mat<T,D,1>      vec;
    T               mag {lims<T>::max()};       // invalid value if not set
    VecMag() {}
    explicit VecMag(Mat<T,D,1> const & v) : vec{v}, mag{cMag(v)} {}
    VecMag(Mat<T,D,1> const & v,T m) : vec{v}, mag{m} {}                // trust passed values

    bool            valid() const {return (mag != lims<T>::max()); }
};
typedef VecMag<float,3>     Vec3FMag;
typedef VecMag<double,3>    Vec3DMag;

// closest point to origin on given line segment:
template<typename T,uint D>
Mat<T,D,1>          closestPointOnSegment(Mat<T,D,1> const & begin,Mat<T,D,1> const & end)
{
    Mat<T,D,1>          segment = end - begin;
    T                   lenSqr = cMag(segment);
    if (lenSqr == 0)
        return begin;
    // the solution below is obtained by defining alpha [0,1] along the segment and minimizing distance:
    T                   alpha = -cDot(begin,segment) / lenSqr;
    if (alpha < 0) alpha = 0;
    if (alpha > 1) alpha = 1;
    return begin + alpha*segment;
}
// closest point to origin on contiguous series of line segments defined by points
// (just returns point if segments.size()==1):
template<typename T,uint D>
VecMag<T,D>         closestPointOnSegments(Svec<Mat<T,D,1>> const & points)
{
    FGASSERT(!points.empty());
    VecMag<T,D>         ret {points[0]};
    for (size_t ii=1; ii<points.size(); ++ii) {
        VecMag<T,D>         vm {closestPointOnSegment(points[ii-1],points[ii])};
        if (vm.mag < ret.mag)
            ret = vm;
    }
    return ret;
}

// oriented line to point comparison. For RHR coordinate system (eg. X increases right and Y increases up).
// Reverse for LHR coordinate system (eg. X increases right and Y increases down).
// returns false if 'begin' == 'end' (ie. degenerate line specification).
bool                isPointLeftOfLine(
    Vec2D               begin,      // arbitrary point on the line
    Vec2D               end,        // different arbitrary point on the line in the oriented direction
    Vec2D               pnt,
    // tolerance for rejecting the point as on the line in same units as above. Zero if to machine precision
    // (not including precisely on the line). Negative values allow for a tolerance on the other side of the line.
    double              tol);

// can handle degenerate tris, in which case it will return false.
inline bool         isPointInTri(
    Vec2D               pnt,
    Arr<Vec2D,3> const & tri,       // must be CC winding in RHR coordinate system, OR CW winding in LHR CS
    double              tol)        // point must be this far inside the tri in same units as above. >=0
{
    return
        isPointLeftOfLine(tri[0],tri[1],pnt,tol) &&
        isPointLeftOfLine(tri[1],tri[2],pnt,tol) &&
        isPointLeftOfLine(tri[2],tri[0],pnt,tol);
}

// Returns: 0: point not in triangle or degenerate triangle.
//           1: point in triangle, CC winding
//          -1: point in triangle, CW winding
int                 pointInTriangle(Vec2D pt,Vec2D v0,Vec2D v1,Vec2D v2);

// Returns the barycentric coord of point relative to triangle.
// If no valid value, triangle is degenerate.
// Point is in triangle if all coordinates are positive:
Opt<Vec3D>          cBarycentricCoord(Vec2D point,Vec2D v0,Vec2D v1,Vec2D v2);

// Returns the barycentric coord of a point relative to a triangle in 3D.
// Only works for planar points.
// Returns invalid if triangle degenerate.
Opt<Vec3D>          cBarycentricCoord(Vec3D point,Vec3D vert0,Vec3D vert1,Vec3D vert2);
inline Opt<Vec3D>   cBarycentricCoord(Vec2F point,Vec2F v0,Vec2F v1,Vec2F v2)
{
    return cBarycentricCoord(Vec2D(point),Vec2D(v0),Vec2D(v1),Vec2D(v2));
}

struct      Plane               // oriented plane
{
    Vec3D               norm;       // Plane normal, unit length. Zero if invalid.
    double              offset;     // s such that: n . x + o = 0 when x is in the plane.
                                    // zero if invalid, but can also be zero when valid.
    Plane() : norm{0}, offset{0} {}
    Plane(Vec3D const & n,double o) : norm{n}, offset{o} {}

    inline bool         invalid() const {return (cMag(norm) == 0); }
    // query point to plane signed distance:
    inline double       distance(Vec3D const & q) const {return cDot(norm,q) + offset; }
};
typedef Svec<Plane>     Planes;

// oriented plane from 3 points, CC winding norm. Plane invalid if points are colinear or coincident:
Plane               cPlane(Vec3D p0,Vec3D p1,Vec3D p2);

// tangent surface from query point-to-surface result, oriented in -ve 'delta' direction:
// (there is no absolute orientation as the surface normal is just the delta and there is no explicit surface)
// if delta is not valid, or if delta is zero, the resulting Plane is not valid:
Plane               cTangentPlane(Vec3D const & query,Vec3DMag const & deltaToSurface);

// Returns the homogeneous coordinate of the intersection of a line through the origin with a plane.
// The homogeneous component will be zero if there is no intersection. Otherwise, the dot product
// of the intersection and the ray will determine the direction (along ray) to intersection.
Vec4D               linePlaneIntersect(
    Vec3D           ray,        // Direction of ray emanating from origin. Does not need to be normalized
    Plane           plane);

// Returns the intersection point of a line and a triangle, if it exists, in either direction:
Opt<Vec3D>          lineTriIntersect(
    Vec3D        point,      // Point on line
    Vec3D        ray,        // Direction of ray emanating from point. Does not need to be normalized
    Vec3D        v0,         // Vertices of triangle
    Vec3D        v1,         // "
    Vec3D        v2);        // "

inline double       pointToPlaneDistSqr(Vec3D pnt,Vec4D planeH)
{
    Vec3D    planeN(planeH[0],planeH[1],planeH[2]);
    return (sqr(cDot(pnt,planeN) + planeH[3]) / planeN.mag());
}

// Find axial-aligned mirror symmetry in mesh shape (not topology) to nearest mirror vertex.
// Returns RMS vertex deltas to nearest mirrored vertex as a ratio of bounding box diagonal.
double              findSaggitalSymmetry(
    Vec3Fs const &      verts,
    Affine3F &          mirror);    // RETURNED: mirror transform

struct      QuadPd2D                // positive definite 2D quadratic in vertex form (ie no linear terms)
{
    Vec2D           centre;
    MatUT2D         qfcut;          // quadratic form cholesky upper triangular

    QuadPd2D() : centre{0,0}, qfcut{0,0,0} {}                // default init to zero quadratic
    QuadPd2D(Vec2D c,MatUT2D q) : centre{c}, qfcut{q} {}

    // note that we do not have a 1/2 factor here, so that should be added as required:
    inline double   operator()(Vec2D p) const {return cMag(qfcut*(p-centre)); }
    inline bool     valid() const {return (qfcut.determinant() != 0.0); }
};
typedef Svec<QuadPd2D>   QuadPd2Ds;
typedef Svec<QuadPd2Ds>  QuadPd2Dss;

struct      Quad2D                  // quadratic in vertex form (ie. no linear terms)
{
    Vec2D           centre;
    MatS2D          qform;          // quadratic form matrix

    Quad2D(Vec2D const & c,MatS2D const & q) : centre{c}, qform{q} {}

    // note that we do not have a 1/2 factor here, so that should be added as required:
    double          operator()(Vec2D const & p) const
    {
        Vec2D           d = p - centre;
        return cDot(d,qform*d);
    }
};
typedef Svec<Quad2D>    Quad2Ds;

struct      Quad3D                  // quadratic in vertex form (ie. no linear terms)
{
    Vec3D           centre;
    MatS3D          qform;          // quadratic form matrix

    Quad3D(Vec3D const & c,MatS3D const & q) : centre{c}, qform{q} {}

    // note that we do not have a 1/2 factor here, so that should be added as required:
    double          operator()(Vec3D const & p) const
    {
        Vec3D           d = p - centre;
        return cDot(d,qform*d);
    }
};
typedef Svec<Quad3D>    Quad3Ds;

// map evaluation of quadratics at positions:
inline Doubles      mapQuad(Quad3Ds const & quads,Vec3Ds const & poss)
{
    return mapCallT<double>(quads,poss,[](Quad3D const & q,Vec3D const & p){return q(p); });
}

}

#endif
