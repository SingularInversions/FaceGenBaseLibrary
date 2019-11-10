//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgGeometry.hpp"
#include "FgRandom.hpp"
#include "FgSimilarity.hpp"
#include "FgMath.hpp"
#include "FgApproxEqual.hpp"
#include "FgDraw.hpp"
#include "FgBounds.hpp"
#include "FgMath.hpp"
#include "FgMain.hpp"

using namespace std;

namespace Fg {

static
void
testOriginToSegmentDistSqr()
{
    Vec3D    p0(1.0,0.0,0.0),
                p1(0.0,1.0,0.0),
                p2(2.0,-1.0,0.0);
    Mat33D rot;
    rot.setIdentity();
    for (uint ii=0; ii<10; ++ii) {
        Vec3D    r0 = rot * p0,
                    r1 = rot * p1,
                    r2 = rot * p2;
        FgVecMag    delta;
        // Degenerate case:
        delta = fgClosestPointInSegment(r0,r0);
        FGASSERT(fgApproxEqual(delta.mag,1.0,10));
        FGASSERT(fgApproxEqual(delta.vec,rot * Vec3D(1.0,0.0,0.0)));
        // edge closest point (both directions):
        delta = fgClosestPointInSegment(r0,r1);
        FGASSERT(fgApproxEqual(delta.mag,0.5,10));
        FGASSERT(fgApproxEqual(delta.vec,rot * Vec3D(0.5,0.5,0.0)));
        delta = fgClosestPointInSegment(r1,r2);
        FGASSERT(fgApproxEqual(delta.mag,0.5,10));
        FGASSERT(fgApproxEqual(delta.vec,rot * Vec3D(0.5,0.5,0.0)));
        // vertex closest point:
        delta = fgClosestPointInSegment(r0,r2);
        FGASSERT(fgApproxEqual(delta.mag,1.0,10));
        FGASSERT(fgApproxEqual(delta.vec,rot * Vec3D(1.0,0.0,0.0)));
        rot = QuaternionD::rand().asMatrix();
    }
}

static
void
testPointToFacetDistSqr()
{
    Vec3D       origin {0},
                p0(1.0,0.0,0.0),
                p1(0.0,1.0,0.0),
                p2(0.0,0.0,1.0),
                p3(2.0,-1.0,0.0),
                p4(2.0,0.0,1.0);
    Mat33D rot;
    rot.setIdentity();
    for (uint ii=0; ii<10; ++ii) {
        Vec3D    r0 = rot * p0,
                    r1 = rot * p1,
                    r2 = rot * p2,
                    r3 = rot * p3,
                    r4 = rot * p4;
        FgVecMag    delta;
        // surface closest point (both orientations):
        delta = fgClosestPointInTri(origin,r0,r1,r2);
        FGASSERT(fgApproxEqual(delta.mag,1.0/3.0,10));
        FGASSERT(fgApproxEqual(delta.vec,rot * Vec3D(1.0/3.0)));
        delta = fgClosestPointInTri(origin,r0,r2,r1);
        FGASSERT(fgApproxEqual(delta.mag,1.0/3.0,10));
        FGASSERT(fgApproxEqual(delta.vec,rot * Vec3D(1.0/3.0)));
        // degenerate facet edge closest point:
        delta = fgClosestPointInTri(origin,r0,r1,r3);
        FGASSERT(fgApproxEqual(delta.mag,0.5,10));
        FGASSERT(fgApproxEqual(delta.vec,rot * Vec3D(0.5,0.5,0.0)));
        // edge closest point (both orientations):
        delta = fgClosestPointInTri(origin,r0,r2,r3);
        FGASSERT(fgApproxEqual(delta.mag,0.5,10));
        FGASSERT(fgApproxEqual(delta.vec,rot * Vec3D(0.5,0.0,0.5)));
        delta = fgClosestPointInTri(origin,r0,r3,r2);
        FGASSERT(fgApproxEqual(delta.mag,0.5,10));
        FGASSERT(fgApproxEqual(delta.vec,rot * Vec3D(0.5,0.0,0.5)));
        // vertex closest point:
        delta = fgClosestPointInTri(origin,r0,r3,r4);
        FGASSERT(fgApproxEqual(delta.mag,1.0,10));
        FGASSERT(fgApproxEqual(delta.vec,rot * Vec3D(1.0,0.0,0.0)));
        delta = fgClosestPointInTri(origin,r0,r0*2.0,r3);
        FGASSERT(fgApproxEqual(delta.mag,1.0,10));
        FGASSERT(fgApproxEqual(delta.vec,rot * Vec3D(1.0,0.0,0.0)));
        rot = QuaternionD::rand().asMatrix();
    }
}

static
void
testBarycentricCoords()
{
    // Test points inside triangle:
    for (uint ii=0; ii<100; ++ii) {
        Vec2D    v0 = Vec2D::randNormal(),
                    v1 = Vec2D::randNormal(),
                    v2 = Vec2D::randNormal(),
                    del1 = v1-v0,
                    del2 = v2-v0;
        if ((del1[0]*del2[1]-del1[1]*del2[0]) > 0.001) {
            double      c0 = randUniform(),
                        c1 = randUniform() * (1.0 - c0),
                        c2 = 1.0 - c1 - c0;
            Vec2D    pnt = v0*c0 + v1*c1 + v2*c2;
            Vec3D    res = fgBarycentricCoords(pnt,v0,v1,v2).val();
            FGASSERT(fgMinElem(res)>=0.0f);     // Inside.
            Vec2D    chk = v0*res[0] + v1*res[1] + v2*res[2];
            FGASSERT(fgApproxEqual(pnt,chk,256));
            FGASSERT(fgApproxEqual(res[0]+res[1]+res[2],1.0,64));
        }
    }
    // Test points outside triangle:
    for (uint ii=0; ii<100; ++ii) {
        Vec2D    v0 = Vec2D::randNormal(),
                    v1 = Vec2D::randNormal(),
                    v2 = Vec2D::randNormal(),
                    del1 = v1-v0,
                    del2 = v2-v0;
        if ((del1[0]*del2[1]-del1[1]*del2[0]) > 0.001) {
            Vec3D    c;
                        c[0] = -float(randUniform()),
                        c[1] = float(randUniform()) * (1.0f - c[0]),
                        c[2] = 1.0f - c[1] - c[0];
            c = fgPermuteAxes<double>(ii%3) * c;
            Vec2D        pnt = v0*c[0] + v1*c[1] + v2*c[2];
            Vec3D        res = fgBarycentricCoords(pnt,v0,v1,v2).val();
            FGASSERT(fgMinElem(res)<0.0f);     // Outside
            Vec2D        chk = v0*res[0] + v1*res[1] + v2*res[2];
            FGASSERT(fgApproxEqual(pnt,chk,128));
            FGASSERT(fgApproxEqual(res[0]+res[1]+res[2],1.0,64));
        }
    }
}

static
void
testBarycentricCoords3D()
{
    fgout << fgnl << "Barycentric 3d: " << fgpush;
    for (uint ii=0; ii<50; ++ii) {
        Vec3D    v0 = Vec3D::randNormal(),
                    v1 = Vec3D::randNormal(),
                    v2 = Vec3D::randNormal(),
                    bc = Vec3D::randNormal();
        bc /= bc[0] + bc[1] + bc[2];
        Vec3D    pt = bc[0]*v0 + bc[1]*v1 + bc[2]*v2;
        Opt<Vec3D>    ret = fgBarycentricCoords(pt,v0,v1,v2);
        if (ret.valid()) {
            Vec3D        res = ret.val(),
                            delta = res-bc;
            //fgout << fgnl << bc << " -> " << res << " delta: " << res-bc;
            FGASSERT(delta.mag() < sqr(0.000001));
        }
    }
    fgout << fgpop;
}

static
void
testPlaneH()
{
    Vec3D    v0(0,0,0),
                v1(1,0,0),
                v2(0,1,0);
    randSeedRepeatable();
    for (size_t ii=0; ii<100; ++ii) {
        Affine3D  s = similarityRand().asAffine();
        Vec4D    pln = fgPlaneH(s*v0,s*v1,s*v2);
        double      a = randUniform(),
                    b = randUniform(),
                    c = 1.0 - a - b;
        Vec3D    pt = s * (v0*a + v1*b + v2*c);
        double      r = cDot(pt,pln.subMatrix<3,1>(0,0)),
                    mag = sqrt(pln.mag());
        FGASSERT(fgApproxEqualMag(-r,pln[3],mag));
    }
}

static
void
testRayPlaneIntersect()
{
    Vec2D    v0(0,0),
                v1(1,0),
                v2(0,1),
                zero;
    randSeedRepeatable();
    for (size_t ii=0; ii<100; ++ii) {
        Mat22D     rot = matRotate(randUniform()*2.0*pi());
        Vec2D        r0 = rot * v0,
                        r1 = rot * v1,
                        r2 = rot * v2;
        Mat33D     rot3 = matRotateAxis((randUniform()*0.5-0.25)*pi(),Vec3D::randNormal());
        Vec3D        p0 = rot3 * fgAsHomogVec(r0),
                        p1 = rot3 * fgAsHomogVec(r1),
                        p2 = rot3 * fgAsHomogVec(r2),
                        pt = rot3 * fgAsHomogVec(zero + Vec2D::randUniform(-0.1,0.1));
        Vec4D        pln = fgPlaneH(p0,p1,p2);
        Vec4D        is = fgLinePlaneIntersect(pt*exp(randNormal()),pln);
        FGASSERT(fgApproxEqual(pt,fgFromHomogVec(is)));
    }
}

static
void
pit0(Vec2D pt,Vec2D v0,Vec2D v1,Vec2D v2,int res)
{
    FGASSERT(fgPointInTriangle(pt,v0,v1,v2) == res);
    FGASSERT(fgPointInTriangle(pt,v0,v2,v1) == res*-1);     //-V764 (PVS Studio)
}

static
void
pit1(Vec2D pt,Vec2D v0,Vec2D v1,Vec2D v2,int res)
{
    for (size_t ii=0; ii<5; ++ii) {
        Mat22D     rot = matRotate(randUniform()*2.0*pi());
        Vec2D        trn(randUniform(),randUniform());
        Affine2D      s(rot,trn);
        pit0(s*pt,s*v0,s*v1,s*v2,res); }
}

static
void
testPointInTriangle()
{
    Vec2D    v0(0.0,0.0),
                v1(1.0,0.0),
                v2(0.0,1.0);
    double      d = numeric_limits<double>::epsilon() * 100,
                d1 = 1.0 - d * 2.0;
    randSeedRepeatable();
    // In middle:
    pit1(Vec2D(0.25,0.25),v0,v1,v2,1);
    // Near vertices:
    pit1(Vec2D(d,d),v0,v1,v2,1);
    pit1(Vec2D(d1,d),v0,v1,v2,1);
    pit1(Vec2D(d,d1),v0,v1,v2,1);
    // Near edges:
    pit1(Vec2D(0.5,d),v0,v1,v2,1);
    pit1(Vec2D(d,0.5),v0,v1,v2,1);
    pit1(Vec2D(0.5-d,0.5-d),v0,v1,v2,1);
    // Miss cases:
    pit1(Vec2D(0.5+d,0.5+d),v0,v1,v2,0);
    pit1(Vec2D(1.0+d,0.0),v0,v1,v2,0);
    pit1(Vec2D(0.0,1.0+d),v0,v1,v2,0);
    pit1(Vec2D(-d,0.5),v0,v1,v2,0);
    pit1(Vec2D(0.5,-d),v0,v1,v2,0);
    pit1(Vec2D(-d,-d),v0,v1,v2,0);
}

static
void
testLineFacetIntersect()
{
    double      s = 0.1;
    Vec3D    v0(0,0,0),
                v1(1,0,0),
                v2(0,1,0);
    Opt<Vec3D>    ret;
    ret = fgLineTriIntersect(Vec3D(s,s,1),Vec3D(0,0,-1),v0,v1,v2);
    FGASSERT(ret.val() == Vec3D(s,s,0));
    ret = fgLineTriIntersect(Vec3D(s,s,1),Vec3D(0,0,1),v0,v1,v2);
    FGASSERT(ret.val() == Vec3D(s,s,0));
    ret = fgLineTriIntersect(Vec3D(-s,-s,1),Vec3D(0,0,1),v0,v1,v2);
    FGASSERT(!ret.valid());
    ret = fgLineTriIntersect(Vec3D(0,0,1),Vec3D(-s,-s,-1),v0,v1,v2);
    FGASSERT(!ret.valid());
    ret = fgLineTriIntersect(Vec3D(0,0,1),Vec3D(s,s,-1),v0,v1,v2);
    FGASSERT(ret.val() == Vec3D(s,s,0));
}

void
fgGeometryTest(const CLArgs &)
{
    randSeedRepeatable();
    testOriginToSegmentDistSqr();
    testPointToFacetDistSqr();
    testBarycentricCoords();
    testBarycentricCoords3D();
    testPlaneH();
    testRayPlaneIntersect();
    testPointInTriangle();
    testLineFacetIntersect();
}

void
fgGeometryManTest(const CLArgs &)
{
    // Give visual feedback on continuity along a line passing through all 3 cases;
    // facet, edge and vertex closest point:
    const uint      ns = 800;
    Vec3D        v0(0.0,0.0,1.0),
                    v1(1.0,1.0,1.0),
                    v2(2.0,-1.0,1.0),
                    pnt(-1.0,0.0,0.0);
    vector<double>  func;
    double          step = 6.0 / double(ns-1);
    for (uint ii=0; ii<ns; ++ii) {
        pnt[0] += step;
        func.push_back(fgClosestPointInTri(pnt,v0,v1,v2).mag);
    }
    uint            sz = ns-2;
    MatD       derivs(sz,3);
    for (uint ii=0; ii<sz; ++ii) {
        derivs.rc(ii,0) = func[ii+1];
        derivs.rc(ii,1) = func[ii+2]-func[ii];
        derivs.rc(ii,2) = func[ii+2] + func[ii] - 2.0 * func[ii+1];
    }
    fgDrawFunctions(derivs);
}

}
