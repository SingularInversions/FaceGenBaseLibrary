//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#ifndef FGGEOMETRY_HPP
#define FGGEOMETRY_HPP

#include "FgTransform.hpp"

namespace Fg {

// Returns the signed area of the parallelogram defined by the points (RHR)
// which is twice the signed area of the triangle defined by the points (CC winding):
double              cArea(Vec2D p0,Vec2D p1,Vec2D p2);
inline double       cArea(Arr<Vec2D,3> const & t) {return cArea(t[0],t[1],t[2]); }

// Returns the vector area of the parallelogram defined by the points (RHR)
// which is twice the vector area of the triangle defined by the points (CC winding):
inline Vec3D        cArea(Vec3D p0,Vec3D p1,Vec3D p2) {return crossProduct(p1-p0,p2-p0); }

// create a (unnormalized) vector perpendicular to the given vector:
Vec3D               cPerp(Vec3D const & v);

// hold a vector along with its magnitude
template<class T,size_t D>
struct      VecMag
{
    Mat<T,D,1>      vec;
    T               mag;
    VecMag() : vec{lims<T>::max()}, mag{lims<T>::max()} {}              // set to invalid values
    explicit VecMag(Mat<T,D,1> const & v) : vec{v}, mag{cMagD(v)} {}
    VecMag(Mat<T,D,1> const & v,T m) : vec{v}, mag{m} {}                // trust passed values

    bool            valid() const {return (mag != lims<T>::max()); }
};
typedef VecMag<float,3>     Vec3FMag;
typedef VecMag<double,3>    Vec3DMag;

// Returns the interpolation coefficient from P0 to P1 for the closest point on the line running through
// P0 and P1 to the origin. The coefficient is in [0,1] when the closest point is between P0 and P1.
// Returns 0 if P0==P1.
template<class T,size_t D>
T                   closestCoeffOnSegmentToOrigin(Arr<Mat<T,D,1>,2> const & seg)
{
    Mat<T,D,1>          delta = seg[1]-seg[0];
    T                   lenSqr = cMagD(delta);
    if (lenSqr == 0)
        return 0;
    // the solution below is obtained by defining alpha [0,1] along the segment and minimizing distance:
    T                   alpha = -cDot(seg[0],delta) / lenSqr;
    if (alpha < 0) alpha = 0;
    if (alpha > 1) alpha = 1;
    return alpha;
}
// closest point to origin on given line segment:
template<class T,size_t D>
Mat<T,D,1>          closestPointOnSegmentToOrigin(Arr<Mat<T,D,1>,2> const & seg)
{
    T                   alpha = closestCoeffOnSegmentToOrigin(seg);
    return seg[0] * (1-alpha) + seg[1] * alpha;
}
template<class T,size_t D>
inline VecMag<double,D> closestPointOnSegment(Mat<T,D,1> query,Arr<Mat<T,D,1>,2> seg)
{
    return VecMag<double,D>{closestPointOnSegmentToOrigin(mapSub(seg,query))};
}
// closest point to origin on contiguous series of line segments defined by points
// (just returns point if segments.size()==1):
template<class T,size_t D,size_t S>
VecMag<T,D>         closestPointOnSegmentsToOrigin(Arr<Mat<T,D,1>,S> const & segs)
{
    VecMag<T,D>         ret {segs[0]};
    for (size_t ii=1; ii<segs.size(); ++ii) {
        VecMag<T,D>         vm {closestPointOnSegmentToOrigin<T,D>({segs[ii-1],segs[ii]})};
        if (vm.mag < ret.mag)
            ret = vm;
    }
    return ret;
}
// closest point to origin on contiguous series of line segments defined by segs
// (just returns point if segments.size()==1):
template<class T,size_t D>
VecMag<T,D>         closestPointOnSegmentsToOrigin(Svec<Mat<T,D,1>> const & segs)
{
    FGASSERT(!segs.empty());
    VecMag<T,D>         ret {segs[0]};
    for (size_t ii=1; ii<segs.size(); ++ii) {
        VecMag<T,D>         vm {closestPointOnSegmentToOrigin<T,D>({segs[ii-1],segs[ii]})};
        if (vm.mag < ret.mag)
            ret = vm;
    }
    return ret;
}
// returns the delta from the query point to the closest point:
template<class T,size_t D,size_t S>
inline VecMag<T,D>  closestPointOnSegments(Mat<T,D,1> query,Arr<Mat<T,D,1>,S> const & segs)
{
    return closestPointOnSegmentsToOrigin(mapSub(segs,query));
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
// Point is in triangle if all coordinates are non-negative:
template<class T>
Opt<Arr<T,3>>       cBarycentricCoord(Mat<T,2,1> point,Arr<Mat<T,2,1>,3> tri)
{
    Arr<Mat<T,2,1>,3>   u = mapSub(tri,point);      // tri relative to point
    T                   c0 = u[1][0]*u[2][1] - u[2][0]*u[1][1],
                        c1 = u[2][0]*u[0][1] - u[0][0]*u[2][1],
                        c2 = u[0][0]*u[1][1] - u[1][0]*u[0][1],
                        d = c0+c1+c2;
    if (d == 0)         // degenerate triangle
        return {};
    return Arr<T,3>{c0/d,c1/d,c2/d};
}

// Returns the barycentric coord of a point relative to a triangle in 3D.
// Only works for planar points.
// Returns invalid if triangle degenerate.
Opt<Arr3D>          cBarycentricCoord(Vec3D point,Vec3D vert0,Vec3D vert1,Vec3D vert2);

struct      Plane               // oriented plane
{
    Vec3D               norm;       // Plane normal, unit length. Zero if invalid.
    double              offset;     // s such that: n . x + o = 0 when x is in the plane.
                                    // zero if invalid, but can also be zero when valid.
    Plane() : norm{0}, offset{0} {}
    Plane(Vec3D const & n,double o) : norm{n}, offset{o} {}

    inline bool         invalid() const {return (cMagD(norm) == 0); }
    // query point to plane signed distance:
    inline double       distance(Vec3D const & q) const {return cDot(norm,q) + offset; }
};
typedef Svec<Plane>     Planes;

// oriented plane from 3 points, CC winding norm. Plane invalid if points are colinear or coincident:
Plane               cPlane(Vec3D p0,Vec3D p1,Vec3D p2);
inline Plane        cPlane(Arr<Vec3D,3> const & pts) {return cPlane(pts[0],pts[1],pts[2]); }

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

// flip a triangle around an edge. Returns the flipped location of the specified vertex around its opposing edge:
Vec2D               flipTri(Arr<Vec2D,3> const & tri,size_t flipVertIdx);

inline double       pointToPlaneDistSqr(Vec3D pnt,Vec4D planeH)
{
    Vec3D    planeN(planeH[0],planeH[1],planeH[2]);
    return (sqr(cDot(pnt,planeN) + planeH[3]) / planeN.magD());
}

// Find axial-aligned mirror symmetry in mesh shape (not topology) to nearest mirror vertex.
// Returns RMS vertex deltas to nearest mirrored vertex as a ratio of bounding box diagonal.
double              findSaggitalSymmetry(
    Vec3Fs const &      verts,
    Affine3F &          mirror);    // RETURNED: mirror transform

struct      Quadratic               // 1D quadratic in vertex form
{
    double          precision;      // second order coefficient
    double          vertex;         // aka centre
    double          vertVal;        // value at vertex

    inline double   operator()(double x) const {return 0.5 * precision * sqr(x-vertex) + vertVal; }
};
bool                isApproxEqual(Quadratic const &,Quadratic const &,double tol);

struct      QuadPd2D                // positive definite 2D quadratic in vertex form (ie no linear terms)
{
    Vec2D           centre;
    MatUT2D         qfcut;          // quadratic form cholesky upper triangular

    QuadPd2D() : centre{0,0}, qfcut{0,0,0} {}                // default init to zero quadratic
    QuadPd2D(Vec2D c,MatUT2D q) : centre{c}, qfcut{q} {}

    // note that we do not have a 1/2 factor here, so that should be added as required:
    inline double   operator()(Vec2D p) const {return cMagD(qfcut*(p-centre)); }
    inline bool     valid() const {return (qfcut.determinant() != 0.0); }
};
typedef Svec<QuadPd2D>   QuadPd2Ds;
typedef Svec<QuadPd2Ds>  QuadPd2Dss;

struct      QuadD               // quadratic in vertex form
{
    Doubles         centre;
    MatSD           qform;

    QuadD(Doubles const & c,MatSD const & q) : centre{c}, qform{q} {FGASSERT(c.size()==q.dim); }

    // note that we do not have a 1/2 factor here, so that should be added as required:
    double          operator()(Doubles const & x) const
    {
        Doubles        p = x - centre;
        return cDot(p,qform*p);
    }
};

inline double       applyQform(MatS3D const & qform,Vec3D const & relPos) {return cDot(relPos,qform*relPos); }

// quadratic, vertex-form, zero-extrema (value at centre is zero), 3D, type double:
struct      QuadVZ3D
{
    MatS3D              qform;      // quadratic form
    Vec3D               centre;

    QuadVZ3D(MatS3D const & q,Vec3D const & c) : qform{q}, centre{c} {}
    QuadVZ3D(double invVar,Vec3D const & c) : qform{Arr3D{invVar},Arr3D{0}}, centre{c} {}   // isotropic ctor

    inline double       at(Vec3D const & x) const {return applyQform(qform,x-centre); }     // no 0.5 factor
    inline double       operator()(Vec3D const & x) const {return 0.5 * applyQform(qform,x-centre); }
};
typedef Svec<QuadVZ3D>  QuadVZ3Ds;

// return the quadratic that gives identical results when its inputs have been linearly transformed (transform must be PD):
QuadVZ3D            operator*(Mat33D const &,QuadVZ3D const &);
// return the quadratic that gives identical results when its inputs have been rigidly transformed:
QuadVZ3D            operator*(Rigid3D const &,QuadVZ3D const &);
// efficiently apply transforms to an array of QuadVZ3Ds:
QuadVZ3Ds           mapTransform(Rigid3D const &,QuadVZ3Ds const &);
QuadVZ3Ds           mapTransform(SimilarityD const &,QuadVZ3Ds const &);
// calculate the univariate quadratic parameterized by the given ray from the origin through the given quadratic:
Quadratic           isectRayQuadratic(QuadVZ3D const & Q,Vec3D ray);

// 3D quadratic in vertex form general case
struct      Quadratic3D
{
    MatS3D              qform;          // Quadratic form
    Vec3D               centre;
    double              centreVal;      // Value at vertex

    Quadratic3D(MatS3D const & q,Vec3D const & c) : qform{q}, centre{c}, centreVal{0} {}
    Quadratic3D(MatS3D const & q,Vec3D const & c,double m) : qform{q}, centre{c}, centreVal{m} {}

    inline double       operator()(Vec3D x) const {return 0.5 * applyQform(qform,x-centre) + centreVal; }
};
typedef Svec<Quadratic3D>   Quadratic3Ds;
// sum of two quadratic functions as a quadratic function:
Quadratic3D         operator+(Quadratic3D const & l,Quadratic3D const & r);

// given the principal axes of a desired positive definite quadratic form, scaled *inversely* to their length,
// return the precision cholesky upper triangular matrix (pcut) of the quadratic form.
// Axes must be perpendicular. Invariant to order and sign of axes.
MatUT3D             axesToPcut(Vec3D a0,Vec3D a1,Vec3D a2);

MatUT3D             cSpheroid(Vec3D axis,double radius);

}

#endif
