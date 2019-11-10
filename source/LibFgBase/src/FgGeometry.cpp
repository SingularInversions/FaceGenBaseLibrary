//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgGeometry.hpp"
#include "FgStdVector.hpp"
#include "FgMatrixSolver.hpp"

using namespace std;

namespace Fg {

FgVecMag
fgClosestPointInSegment(Vec3D p0,Vec3D p1)
{
    FgVecMag    ret;
    Vec3D    segment = p1-p0;
    double      lenSqr = segment.mag();
    if (lenSqr == 0.0) {
        ret.vec = p0;
        ret.mag = p0.mag();
        return ret;
    }
    double      b0 = cDot(segment,p0),
                b1 = cDot(segment,p1);
    if (b0*b1 <= 0.0) {
        ret.vec = p0 - (b0*segment)/lenSqr;
        ret.mag = ret.vec.mag();
        return ret;
    }
    double      l0 = p0.mag(),
                l1 = p1.mag();
    if (l0 < l1) {
        ret.vec = p0;
        ret.mag = l0;
        return ret;
    }
    ret.vec = p1;
    ret.mag = l1;
    return ret;
}

FgVecMag
fgClosestPointInTri(
    Vec3D    point,
    Vec3D    vert0,
    Vec3D    vert1,
    Vec3D    vert2)
{
    Vec3D    v0 = vert0 - point,
                v1 = vert1 - point,
                v2 = vert2 - point,
                e01 = vert1 - vert0,
                e12 = vert2 - vert1,
                e02 = vert2 - vert0,
                norm = crossProduct(e01,e02);
    double      normLensqr = norm.mag();
    // Degenerate case:
    if (normLensqr == 0.0) {
        double  mag01 = e01.mag(),
                mag12 = e12.mag(),
                mag02 = e02.mag();
        if (mag01 > mag12) {
            if (mag01 > mag02)
                return fgClosestPointInSegment(v0,v1);
            else
                return fgClosestPointInSegment(v0,v2);
        }
        else {
            if (mag12 > mag02)
                return fgClosestPointInSegment(v1,v2);
            else
                return fgClosestPointInSegment(v0,v2);
        }
    }
    // Normal case:
    Vec3D    clipn0 = crossProduct(norm,e01),
                clipn1 = crossProduct(norm,e12),
                clipn2 = crossProduct(norm,-e02);
    double      p0ls = v0.mag(),
                p1ls = v1.mag(),
                p2ls = v2.mag(),
                clipb0 = cDot(clipn0,(p1ls > p0ls) ? v1 : v0),
                clipb1 = cDot(clipn1,(p2ls > p1ls) ? v2 : v1),
                clipb2 = cDot(clipn2,(p0ls > p2ls) ? v0 : v2);
    if (clipb0 > 0.0)
        return fgClosestPointInSegment(v0,v1);
    if (clipb1 > 0.0)
        return fgClosestPointInSegment(v1,v2);
    if (clipb2 > 0.0)
        return fgClosestPointInSegment(v2,v0);
    double      dot = cDot(norm,v0),
                dmag = dot / normLensqr;
    FgVecMag    ret;
    ret.vec = norm * dmag;
    ret.mag = dot * dmag;
    return ret;
}
    
Opt<Vec3D>
fgBarycentricCoords(
    Vec2D            point,
    Vec2D            v0,
    Vec2D            v1,
    Vec2D            v2)
{
    Opt<Vec3D>    ret;
    Vec2D        u0 = v0-point,
                    u1 = v1-point,
                    u2 = v2-point;
    double          c0 = u1[0]*u2[1] - u2[0]*u1[1],
                    c1 = u2[0]*u0[1] - u0[0]*u2[1],
                    c2 = u0[0]*u1[1] - u1[0]*u0[1],
                    d = c0+c1+c2;
    if (d == 0.0) {     // All segments from point to vertices are colinear
        return ret;
    }
    ret = Vec3D(c0/d,c1/d,c2/d);
    return ret;
}

Opt<Vec3D>
fgBarycentricCoords(
    Vec3D point,
    Vec3D v0,
    Vec3D v1,
    Vec3D v2)
{
    Opt<Vec3D>    ret;
    // Transform into the coordinate system v0 = (0,0), v1 = (1,0), v2 = (0,1) by defining
    // the inverse homogenous transform then inverting and taking only the top 2 rows to
    // form 'xf' below. Converting to barycentric is then trivial, and barycentric coords
    // are preserved by linear xforms.
    Vec3D    e01 = v1-v0,
                e02 = v2-v0;
    double      det =
        e01[0]*e02[1]*v0[2] + e02[0]*v0[1]*e01[2] + e01[1]*e02[2]*v0[0] -
        e01[2]*e02[1]*v0[0] - e01[1]*e02[0]*v0[2] - e02[2]*v0[1]*e01[0];
    if (det == 0)
        return ret;
    Mat23D     xf;
    xf[0] = e02[1]*v0[2] - e02[2]*v0[1];
    xf[1] = e02[2]*v0[0] - e02[0]*v0[2];
    xf[2] = e02[0]*v0[1] - e02[1]*v0[0];
    xf[3] = e01[2]*v0[1] - e01[1]*v0[2];
    xf[4] = e01[0]*v0[2] - e01[2]*v0[0];
    xf[5] = e01[1]*v0[0] - e01[0]*v0[1];
    xf *= 1 / det;
    Vec2D    bc = xf * point;
    return Vec3D(1-bc[0]-bc[1],bc[0],bc[1]);
}

Vec4D
fgPlaneH(Vec3D p0,Vec3D p1,Vec3D p2)
{
    Vec3D    nrm = crossProduct(p1-p0,p2-p0);
    FGASSERT(nrm.mag() > 0.0);
    double      w = -cDot(nrm,p0);
    FGASSERT(w != 0.0);
    Vec4D    ph(nrm[0],nrm[1],nrm[2],w);
    return ph;
}

Vec4D
fgLinePlaneIntersect(Vec3D r,Vec4D p)
{
    double  a = -p[3];
    return Vec4D(a*r[0],a*r[1],a*r[2],r[0]*p[0]+r[1]*p[1]+r[2]*p[2]);
}

int
fgPointInTriangle(Vec2D pt,Vec2D v0,Vec2D v1,Vec2D v2)
{
    Vec2D    s0 = v1-v0,
                s1 = v2-v1,
                s2 = v0-v2,
                p0 = pt-v0,
                p1 = pt-v1,
                p2 = pt-v2,
                zero;
    double      d0 = s0[0]*p0[1]-s0[1]*p0[0],
                d1 = s1[0]*p1[1]-s1[1]*p1[0],
                d2 = s2[0]*p2[1]-s2[1]*p2[0];
    // Degenerate triangle check necessary or any point will appear as in triangle:
    if (s0 == zero || s1 == zero || s2 == zero)
        return 0;
    if (d0 >= 0 && d1 >= 0 && d2 >= 0)
        return 1;
    if (d0 <= 0 && d1 <= 0 && d2 <= 0)
        return -1;
    return 0;
}

Opt<Vec3D>
fgLineTriIntersect(Vec3D pnt,Vec3D dir,Vec3D v0,Vec3D v1,Vec3D v2)
{
    Opt<Vec3D>         ret;
    // First calculate intersection with plane (in CS with 'pnt' as origin):
    Vec3D                p0 = v0-pnt;
    Vec4D                planeH = fgPlaneH(p0,v1-pnt,v2-pnt);
    Vec4D                isectH = fgLinePlaneIntersect(dir,planeH);
    if (isectH[3] != 0.0) {     // line can't be parallel to facet plane
        Vec3D                isect = fgFromHomogVec(isectH);
        // Now calculate barycentric coords s,t (with axes v1-v0,v2-v0) of intersection point:
        Vec3D                u(v1-v0),
                                v(v2-v0),
                                w(isect-p0);
        double                  uv = cDot(u,v),
                                wv = cDot(w,v),
                                vv = cDot(v,v),
                                wu = cDot(w,u),
                                uu = cDot(u,u),
                                s = (uv*wv-vv*wu) / (uv*uv-uu*vv),
                                t = (uv*wu-uu*wv) / (uv*uv-uu*vv);
        if ((s >= 0.0) && (t >= 0.0) && (s+t < 1.0))
            ret = pnt+isect;
    }
    return ret;
}

}
