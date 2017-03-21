//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Dec 2, 2006
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

static
void
testOriginToSegmentDistSqr()
{
    FgVect3D    p0(1.0,0.0,0.0),
                p1(0.0,1.0,0.0),
                p2(2.0,-1.0,0.0);
    FgMat33D rot;
    rot.setIdentity();
    for (uint ii=0; ii<10; ++ii) {
        FgVect3D    r0 = rot * p0,
                    r1 = rot * p1,
                    r2 = rot * p2;
        FgVecMag    delta;
        // Degenerate case:
        delta = fgClosestPointInSegment(r0,r0);
        FGASSERT(fgApproxEqual(delta.mag,1.0,10));
        FGASSERT(fgApproxEqual(delta.vec,rot * FgVect3D(1.0,0.0,0.0)));
        // edge closest point (both directions):
        delta = fgClosestPointInSegment(r0,r1);
        FGASSERT(fgApproxEqual(delta.mag,0.5,10));
        FGASSERT(fgApproxEqual(delta.vec,rot * FgVect3D(0.5,0.5,0.0)));
        delta = fgClosestPointInSegment(r1,r2);
        FGASSERT(fgApproxEqual(delta.mag,0.5,10));
        FGASSERT(fgApproxEqual(delta.vec,rot * FgVect3D(0.5,0.5,0.0)));
        // vertex closest point:
        delta = fgClosestPointInSegment(r0,r2);
        FGASSERT(fgApproxEqual(delta.mag,1.0,10));
        FGASSERT(fgApproxEqual(delta.vec,rot * FgVect3D(1.0,0.0,0.0)));
        rot = fgQuaternionRand().asMatrix();
    }
}

static
void
testPointToFacetDistSqr()
{
    FgVect3D    origin,
                p0(1.0,0.0,0.0),
                p1(0.0,1.0,0.0),
                p2(0.0,0.0,1.0),
                p3(2.0,-1.0,0.0),
                p4(2.0,0.0,1.0);
    FgMat33D rot;
    rot.setIdentity();
    for (uint ii=0; ii<10; ++ii) {
        FgVect3D    r0 = rot * p0,
                    r1 = rot * p1,
                    r2 = rot * p2,
                    r3 = rot * p3,
                    r4 = rot * p4;
        FgVecMag    delta;
        // surface closest point (both orientations):
        delta = fgClosestPointInTri(origin,r0,r1,r2);
        FGASSERT(fgApproxEqual(delta.mag,1.0/3.0,10));
        FGASSERT(fgApproxEqual(delta.vec,rot * FgVect3D(1.0/3.0)));
        delta = fgClosestPointInTri(origin,r0,r2,r1);
        FGASSERT(fgApproxEqual(delta.mag,1.0/3.0,10));
        FGASSERT(fgApproxEqual(delta.vec,rot * FgVect3D(1.0/3.0)));
        // degenerate facet edge closest point:
        delta = fgClosestPointInTri(origin,r0,r1,r3);
        FGASSERT(fgApproxEqual(delta.mag,0.5,10));
        FGASSERT(fgApproxEqual(delta.vec,rot * FgVect3D(0.5,0.5,0.0)));
        // edge closest point (both orientations):
        delta = fgClosestPointInTri(origin,r0,r2,r3);
        FGASSERT(fgApproxEqual(delta.mag,0.5,10));
        FGASSERT(fgApproxEqual(delta.vec,rot * FgVect3D(0.5,0.0,0.5)));
        delta = fgClosestPointInTri(origin,r0,r3,r2);
        FGASSERT(fgApproxEqual(delta.mag,0.5,10));
        FGASSERT(fgApproxEqual(delta.vec,rot * FgVect3D(0.5,0.0,0.5)));
        // vertex closest point:
        delta = fgClosestPointInTri(origin,r0,r3,r4);
        FGASSERT(fgApproxEqual(delta.mag,1.0,10));
        FGASSERT(fgApproxEqual(delta.vec,rot * FgVect3D(1.0,0.0,0.0)));
        delta = fgClosestPointInTri(origin,r0,r0*2.0,r3);
        FGASSERT(fgApproxEqual(delta.mag,1.0,10));
        FGASSERT(fgApproxEqual(delta.vec,rot * FgVect3D(1.0,0.0,0.0)));
        rot = fgQuaternionRand().asMatrix();
    }
}

static
void
testBarycentricCoords()
{
    // Test points inside triangle:
    for (uint ii=0; ii<100; ++ii) {
        FgVect2D    v0 = fgMatRandNormal<2,1>(),
                    v1 = fgMatRandNormal<2,1>(),
                    v2 = fgMatRandNormal<2,1>(),
                    del1 = v1-v0,
                    del2 = v2-v0;
        if ((del1[0]*del2[1]-del1[1]*del2[0]) > 0.001) {
            double      c0 = fgRand(),
                        c1 = fgRand() * (1.0 - c0),
                        c2 = 1.0 - c1 - c0;
            FgVect2D    pnt = v0*c0 + v1*c1 + v2*c2;
            FgVect3D    res = fgBarycentricCoords(pnt,v0,v1,v2).val();
            FGASSERT(fgMinElem(res)>=0.0f);     // Inside.
            FgVect2D    chk = v0*res[0] + v1*res[1] + v2*res[2];
            FGASSERT(fgApproxEqual(pnt,chk,128));
            FGASSERT(fgApproxEqual(res[0]+res[1]+res[2],1.0,64));
        }
    }
    // Test points outside triangle:
    for (uint ii=0; ii<100; ++ii) {
        FgVect2D    v0 = fgMatRandNormal<2,1>(),
                    v1 = fgMatRandNormal<2,1>(),
                    v2 = fgMatRandNormal<2,1>(),
                    del1 = v1-v0,
                    del2 = v2-v0;
        if ((del1[0]*del2[1]-del1[1]*del2[0]) > 0.001) {
            FgVect3D    c;
                        c[0] = -float(fgRand()),
                        c[1] = float(fgRand()) * (1.0f - c[0]),
                        c[2] = 1.0f - c[1] - c[0];
            c = fgPermuteAxes<double>(ii%3) * c;
            FgVect2D        pnt = v0*c[0] + v1*c[1] + v2*c[2];
            FgVect3D        res = fgBarycentricCoords(pnt,v0,v1,v2).val();
            FGASSERT(fgMinElem(res)<0.0f);     // Outside
            FgVect2D        chk = v0*res[0] + v1*res[1] + v2*res[2];
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
        FgVect3D    v0 = fgMatRandNormal<3,1>(),
                    v1 = fgMatRandNormal<3,1>(),
                    v2 = fgMatRandNormal<3,1>(),
                    bc = fgMatRandNormal<3,1>();
        bc /= bc[0] + bc[1] + bc[2];
        FgVect3D    pt = bc[0]*v0 + bc[1]*v1 + bc[2]*v2;
        FgOpt<FgVect3D>    ret = fgBarycentricCoords(pt,v0,v1,v2);
        if (ret.valid()) {
            FgVect3D        res = ret.val(),
                            delta = res-bc;
            //fgout << fgnl << bc << " -> " << res << " delta: " << res-bc;
            FGASSERT(delta.mag() < fgSqr(0.000001));
        }
    }
    fgout << fgpop;
}

static
void
testPlaneH()
{
    FgVect3D    v0(0,0,0),
                v1(1,0,0),
                v2(0,1,0);
    fgRandSeedRepeatable();
    for (size_t ii=0; ii<100; ++ii) {
        FgAffine3D  s = fgSimilarityRand().asAffine();
        FgVect4D    pln = fgPlaneH(s*v0,s*v1,s*v2);
        double      a = fgRand(),
                    b = fgRand(),
                    c = 1.0 - a - b;
        FgVect3D    pt = s * (v0*a + v1*b + v2*c);
        double      r = fgDot(pt,pln.subMatrix<3,1>(0,0)),
                    mag = sqrt(pln.mag());
        FGASSERT(fgApproxEqualMag(-r,pln[3],mag));
    }
}

static
void
testRayPlaneIntersect()
{
    FgVect2D    v0(0,0),
                v1(1,0),
                v2(0,1),
                zero;
    fgRandSeedRepeatable();
    for (size_t ii=0; ii<100; ++ii) {
        FgMat22D     rot = fgMatRotate(fgRand()*2.0*fgPi());
        FgVect2D        r0 = rot * v0,
                        r1 = rot * v1,
                        r2 = rot * v2;
        FgMat33D     rot3 = fgMatRotateAxis((fgRand()*0.5-0.25)*fgPi(),fgMatRandNormal<3,1>());
        FgVect3D        p0 = rot3 * fgAsHomogVec(r0),
                        p1 = rot3 * fgAsHomogVec(r1),
                        p2 = rot3 * fgAsHomogVec(r2),
                        pt = rot3 * fgAsHomogVec(zero + fgMatRandUniform<2,1>(-0.1,0.1));
        FgVect4D        pln = fgPlaneH(p0,p1,p2);
        FgVect4D        is = fgLinePlaneIntersect(pt*exp(fgRandNormal()),pln);
        FGASSERT(fgApproxEqual(pt,fgFromHomogVec(is)));
    }
}

static
void
pit0(FgVect2D pt,FgVect2D v0,FgVect2D v1,FgVect2D v2,int res)
{
    FGASSERT(fgPointInTriangle(pt,v0,v1,v2) == res);
    FGASSERT(fgPointInTriangle(pt,v0,v2,v1) == res*-1);
}

static
void
pit1(FgVect2D pt,FgVect2D v0,FgVect2D v1,FgVect2D v2,int res)
{
    for (size_t ii=0; ii<5; ++ii) {
        FgMat22D     rot = fgMatRotate(fgRand()*2.0*fgPi());
        FgVect2D        trn(fgRand(),fgRand());
        FgAffine2D      s(rot,trn);
        pit0(s*pt,s*v0,s*v1,s*v2,res); }
}

static
void
testPointInTriangle()
{
    FgVect2D    v0(0.0,0.0),
                v1(1.0,0.0),
                v2(0.0,1.0);
    double      d = numeric_limits<double>::epsilon() * 100,
                d1 = 1.0 - d * 2.0;
    fgRandSeedRepeatable();
    // In middle:
    pit1(FgVect2D(0.25,0.25),v0,v1,v2,1);
    // Near vertices:
    pit1(FgVect2D(d,d),v0,v1,v2,1);
    pit1(FgVect2D(d1,d),v0,v1,v2,1);
    pit1(FgVect2D(d,d1),v0,v1,v2,1);
    // Near edges:
    pit1(FgVect2D(0.5,d),v0,v1,v2,1);
    pit1(FgVect2D(d,0.5),v0,v1,v2,1);
    pit1(FgVect2D(0.5-d,0.5-d),v0,v1,v2,1);
    // Miss cases:
    pit1(FgVect2D(0.5+d,0.5+d),v0,v1,v2,0);
    pit1(FgVect2D(1.0+d,0.0),v0,v1,v2,0);
    pit1(FgVect2D(0.0,1.0+d),v0,v1,v2,0);
    pit1(FgVect2D(-d,0.5),v0,v1,v2,0);
    pit1(FgVect2D(0.5,-d),v0,v1,v2,0);
    pit1(FgVect2D(-d,-d),v0,v1,v2,0);
}

static
void
testLineFacetIntersect()
{
    double      s = 0.1;
    FgVect3D    v0(0,0,0),
                v1(1,0,0),
                v2(0,1,0);
    FgOpt<FgVect3D>    ret;
    ret = fgLineTriIntersect(FgVect3D(s,s,1),FgVect3D(0,0,-1),v0,v1,v2);
    FGASSERT(ret.val() == FgVect3D(s,s,0));
    ret = fgLineTriIntersect(FgVect3D(s,s,1),FgVect3D(0,0,1),v0,v1,v2);
    FGASSERT(ret.val() == FgVect3D(s,s,0));
    ret = fgLineTriIntersect(FgVect3D(-s,-s,1),FgVect3D(0,0,1),v0,v1,v2);
    FGASSERT(!ret.valid());
    ret = fgLineTriIntersect(FgVect3D(0,0,1),FgVect3D(-s,-s,-1),v0,v1,v2);
    FGASSERT(!ret.valid());
    ret = fgLineTriIntersect(FgVect3D(0,0,1),FgVect3D(s,s,-1),v0,v1,v2);
    FGASSERT(ret.val() == FgVect3D(s,s,0));
}

void
fgGeometryTest(const FgArgs &)
{
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
fgGeometryManTest(const FgArgs &)
{
    // Give visual feedback on continuity along a line passing through all 3 cases;
    // facet, edge and vertex closest point:
    const uint      ns = 800;
    FgVect3D        v0(0.0,0.0,1.0),
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
    FgMatrixD       derivs(sz,3);
    for (uint ii=0; ii<sz; ++ii) {
        derivs.rc(ii,0) = func[ii+1];
        derivs.rc(ii,1) = func[ii+2]-func[ii];
        derivs.rc(ii,2) = func[ii+2] + func[ii] - 2.0 * func[ii+1];
    }
    fgDrawFunctions(derivs);
}
