//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Sept 20, 2005
//

#include "stdafx.h"

#include "FgGeometry.hpp"
#include "FgStdVector.hpp"
#include "FgMatrixSolver.hpp"

using namespace std;

FgVecMag
fgClosestPointInSegment(FgVect3D p0,FgVect3D p1)
{
    FgVecMag    ret;
    FgVect3D    segment = p1-p0;
    double      lenSqr = segment.lengthSqr();
    if (lenSqr == 0.0) {
        ret.vec = p0;
        ret.mag = p0.lengthSqr();
        return ret;
    }
    double      b0 = fgDot(segment,p0),
                b1 = fgDot(segment,p1);
    if (b0*b1 <= 0.0) {
        ret.vec = p0 - (b0*segment)/lenSqr;
        ret.mag = ret.vec.lengthSqr();
        return ret;
    }
    double      l0 = p0.lengthSqr(),
                l1 = p1.lengthSqr();
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
    FgVect3D    point,
    FgVect3D    vert0,
    FgVect3D    vert1,
    FgVect3D    vert2)
{
    FgVect3D    v0 = vert0 - point,
                v1 = vert1 - point,
                v2 = vert2 - point,
                e01 = vert1 - vert0,
                e12 = vert2 - vert1,
                e02 = vert2 - vert0,
                norm = fgCrossProduct(e01,e02);
    double      normLensqr = norm.lengthSqr();
    // Degenerate case:
    if (normLensqr == 0.0) {
        double  mag01 = e01.lengthSqr(),
                mag12 = e12.lengthSqr(),
                mag02 = e02.lengthSqr();
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
    FgVect3D    clipn0 = fgCrossProduct(norm,e01),
                clipn1 = fgCrossProduct(norm,e12),
                clipn2 = fgCrossProduct(norm,-e02);
    double      p0ls = v0.lengthSqr(),
                p1ls = v1.lengthSqr(),
                p2ls = v2.lengthSqr(),
                clipb0 = fgDot(clipn0,(p1ls > p0ls) ? v1 : v0),
                clipb1 = fgDot(clipn1,(p2ls > p1ls) ? v2 : v1),
                clipb2 = fgDot(clipn2,(p0ls > p2ls) ? v0 : v2);
    if (clipb0 > 0.0)
        return fgClosestPointInSegment(v0,v1);
    if (clipb1 > 0.0)
        return fgClosestPointInSegment(v1,v2);
    if (clipb2 > 0.0)
        return fgClosestPointInSegment(v2,v0);
    double      dot = fgDot(norm,v0),
                dmag = dot / normLensqr;
    FgVecMag    ret;
    ret.vec = norm * dmag;
    ret.mag = dot * dmag;
    return ret;
}
    
FgOpt<FgVect3D>
fgBarycentricCoords(
    FgVect2D            point,
    FgVect2D            v0,
    FgVect2D            v1,
    FgVect2D            v2)
{
    FgOpt<FgVect3D>    ret;
    FgVect2D        u0 = v0-point,
                    u1 = v1-point,
                    u2 = v2-point;
    double          c0 = u1[0]*u2[1] - u2[0]*u1[1],
                    c1 = u2[0]*u0[1] - u0[0]*u2[1],
                    c2 = u0[0]*u1[1] - u1[0]*u0[1],
                    d = c0+c1+c2;
    if (d == 0.0) {     // All segments from point to vertices are colinear
        return ret;
    }
    ret = FgVect3D(c0/d,c1/d,c2/d);
    return ret;
}

FgOpt<FgVect3D>
fgBarycentricCoords(
    FgVect3D point,
    FgVect3D v0,
    FgVect3D v1,
    FgVect3D v2)
{
    FgOpt<FgVect3D>    ret;
    // Transform into the coordinate system v0 = (0,0), v1 = (1,0), v2 = (0,1) by defining
    // the inverse homogenous transform then inverting and taking only the top 2 rows to
    // form 'xf' below. Converting to barycentric is then trivial, and barycentric coords
    // are preserved by linear xforms.
    FgVect3D    e01 = v1-v0,
                e02 = v2-v0;
    double      det =
        e01[0]*e02[1]*v0[2] + e02[0]*v0[1]*e01[2] + e01[1]*e02[2]*v0[0] -
        e01[2]*e02[1]*v0[0] - e01[1]*e02[0]*v0[2] - e02[2]*v0[1]*e01[0];
    if (det == 0)
        return ret;
    FgMat23D     xf;
    xf[0] = e02[1]*v0[2] - e02[2]*v0[1];
    xf[1] = e02[2]*v0[0] - e02[0]*v0[2];
    xf[2] = e02[0]*v0[1] - e02[1]*v0[0];
    xf[3] = e01[2]*v0[1] - e01[1]*v0[2];
    xf[4] = e01[0]*v0[2] - e01[2]*v0[0];
    xf[5] = e01[1]*v0[0] - e01[0]*v0[1];
    xf *= 1 / det;
    FgVect2D    bc = xf * point;
    return FgVect3D(1-bc[0]-bc[1],bc[0],bc[1]);
}

FgVect4D
fgPlaneH(FgVect3D p0,FgVect3D p1,FgVect3D p2)
{
    FgVect3D    nrm = fgCrossProduct(p1-p0,p2-p0);
    FGASSERT(nrm.lengthSqr() > 0.0);
    double      w = -fgDot(nrm,p0);
    FGASSERT(w != 0.0);
    FgVect4D    ph(nrm[0],nrm[1],nrm[2],w);
    return ph;
}

FgVect4D
fgLinePlaneIntersect(FgVect3D r,FgVect4D p)
{
    double  a = -p[3];
    return FgVect4D(a*r[0],a*r[1],a*r[2],r[0]*p[0]+r[1]*p[1]+r[2]*p[2]);
}

int
fgPointInTriangle(FgVect2D pt,FgVect2D v0,FgVect2D v1,FgVect2D v2)
{
    FgVect2D    s0 = v1-v0,
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

FgOpt<FgVect3D>
fgLineFacetIntersect(FgVect3D pnt,FgVect3D dir,FgVect3D v0,FgVect3D v1,FgVect3D v2)
{
    FgOpt<FgVect3D>    ret;
    // First calculate intersection with plane (in CS with 'pnt' as origin):
    FgVect3D                p0 = v0-pnt;
    FgVect4D                planeH = fgPlaneH(p0,v1-pnt,v2-pnt);
    FgVect4D                isectH = fgLinePlaneIntersect(dir,planeH);
    if (isectH[3] == 0.0)   // line parallel to facet plane
        return ret;
    FgVect3D                isect = fgFromHomogVec(isectH);
    // Now calculate barycentric coords s,t (with axes v1-v0,v2-v0) of intersection point:
    FgVect3D                u(v1-v0),
                            v(v2-v0),
                            w(isect-p0);
    double                  uv = fgDot(u,v),
                            wv = fgDot(w,v),
                            vv = fgDot(v,v),
                            wu = fgDot(w,u),
                            uu = fgDot(u,u),
                            s = (uv*wv-vv*wu) / (uv*uv-uu*vv),
                            t = (uv*wu-uu*wv) / (uv*uv-uu*vv);
    if ((s >= 0.0) && (t >= 0.0) && (s+t < 1.0))
        ret = pnt+isect;
    return ret;
}
