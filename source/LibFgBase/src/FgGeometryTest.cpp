//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgGeometry.hpp"
#include "FgRandom.hpp"
#include "FgSimilarity.hpp"
#include "FgMath.hpp"
#include "FgApproxEqual.hpp"
#include "FgImageDraw.hpp"
#include "FgBounds.hpp"
#include "FgMath.hpp"
#include "FgMain.hpp"

using namespace std;

namespace Fg {

namespace {

static
void
closestBarycentricPoint()
{
    for (size_t ii=0; ii<100; ++ii) {
        Vec3D           p0 = Vec3D::randNormal(),
                        p1 = Vec3D::randNormal(),
                        p2 = Vec3D::randNormal();
        Plane           plane = cPlane(p0,p1,p2);
        // Closest point in plane to origin equation derived using Lagrange's method:
        Vec3D           closest0 = -plane.scalar * plane.norm / cMag(plane.norm),
                        bary = closestBarycentricPoint(p0,p1,p2),
                        closest1 = bary[0]*p0 + bary[1]*p1 + bary[2]*p2;
        FGASSERT(isApproxEqualRelPrec(closest0,closest1));
        Opt<Vec3D>      bary1 = barycentricCoord(closest1,p0,p1,p2);
        FGASSERT(isApproxEqualRelPrec(bary,bary1.val()));
    }
}

static
void
originToSegmentDistSqr()
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
        VecMagD    delta;
        // Degenerate case:
        delta = closestPointInSegment(r0,r0);
        FGASSERT(isApproxEqualRelPrec(delta.mag,1.0));
        FGASSERT(isApproxEqualRelMag(delta.vec,rot * Vec3D(1.0,0.0,0.0),30));
        // edge closest point (both directions):
        delta = closestPointInSegment(r0,r1);
        FGASSERT(isApproxEqualRelPrec(delta.mag,0.5));
        FGASSERT(isApproxEqualRelMag(delta.vec,rot * Vec3D(0.5,0.5,0.0),30));
        delta = closestPointInSegment(r1,r2);
        FGASSERT(isApproxEqualRelPrec(delta.mag,0.5));
        FGASSERT(isApproxEqualRelMag(delta.vec,rot * Vec3D(0.5,0.5,0.0),30));
        // vertex closest point:
        delta = closestPointInSegment(r0,r2);
        FGASSERT(isApproxEqualRelPrec(delta.mag,1.0));
        FGASSERT(isApproxEqualRelMag(delta.vec,rot * Vec3D(1.0,0.0,0.0),30));
        rot = QuaternionD::rand().asMatrix();
    }
}

static
void
pointToFacetDistSqr()
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
        VecMagD    delta;
        // surface closest point (both orientations):
        delta = closestPointInTri(origin,r0,r1,r2);
        FGASSERT(isApproxEqualRelPrec(delta.mag,1.0/3.0));
        FGASSERT(isApproxEqualRelMag(delta.vec,rot * Vec3D(1.0/3.0),30));
        delta = closestPointInTri(origin,r0,r2,r1);
        FGASSERT(isApproxEqualRelPrec(delta.mag,1.0/3.0));
        FGASSERT(isApproxEqualRelMag(delta.vec,rot * Vec3D(1.0/3.0),30));
        // degenerate facet edge closest point:
        delta = closestPointInTri(origin,r0,r1,r3);
        FGASSERT(isApproxEqualRelPrec(delta.mag,0.5));
        FGASSERT(isApproxEqualRelMag(delta.vec,rot * Vec3D(0.5,0.5,0.0),30));
        // edge closest point (both orientations):
        delta = closestPointInTri(origin,r0,r2,r3);
        FGASSERT(isApproxEqualRelPrec(delta.mag,0.5));
        FGASSERT(isApproxEqualRelMag(delta.vec,rot * Vec3D(0.5,0.0,0.5),30));
        delta = closestPointInTri(origin,r0,r3,r2);
        FGASSERT(isApproxEqualRelPrec(delta.mag,0.5));
        FGASSERT(isApproxEqualRelMag(delta.vec,rot * Vec3D(0.5,0.0,0.5),30));
        // vertex closest point:
        delta = closestPointInTri(origin,r0,r3,r4);
        FGASSERT(isApproxEqualRelPrec(delta.mag,1.0));
        FGASSERT(isApproxEqualRelMag(delta.vec,rot * Vec3D(1.0,0.0,0.0),30));
        delta = closestPointInTri(origin,r0,r0*2.0,r3);
        FGASSERT(isApproxEqualRelPrec(delta.mag,1.0));
        FGASSERT(isApproxEqualRelMag(delta.vec,rot * Vec3D(1.0,0.0,0.0),30));
        rot = QuaternionD::rand().asMatrix();
    }
}

static
void
barycentricCoords()
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
            Vec3D    res = barycentricCoord(pnt,v0,v1,v2).val();
            FGASSERT(cMinElem(res)>=0.0f);     // Inside.
            Vec2D    chk = v0*res[0] + v1*res[1] + v2*res[2];
            FGASSERT(isApproxEqualRelMag(pnt,chk,30));
            FGASSERT(isApproxEqualRelPrec(res[0]+res[1]+res[2],1.0));
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
            c = permuteAxes<double>(ii%3) * c;
            Vec2D        pnt = v0*c[0] + v1*c[1] + v2*c[2];
            Vec3D        res = barycentricCoord(pnt,v0,v1,v2).val();
            FGASSERT(cMinElem(res)<0.0f);     // Outside
            Vec2D        chk = v0*res[0] + v1*res[1] + v2*res[2];
            FGASSERT(isApproxEqualRelMag(pnt,chk,30));
            FGASSERT(isApproxEqualRelPrec(res[0]+res[1]+res[2],1.0));
        }
    }
}

static
void
barycentricCoords3D()
{
    fgout << fgnl << "Barycentric 3d: " << fgpush;
    for (uint ii=0; ii<50; ++ii) {
        Vec3D    v0 = Vec3D::randNormal(),
                    v1 = Vec3D::randNormal(),
                    v2 = Vec3D::randNormal(),
                    bc = Vec3D::randNormal();
        bc /= bc[0] + bc[1] + bc[2];
        Vec3D    pt = bc[0]*v0 + bc[1]*v1 + bc[2]*v2;
        Opt<Vec3D>    ret = barycentricCoord(pt,v0,v1,v2);
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
planeH()
{
    Vec3D       v0(0,0,0),
                v1(1,0,0),
                v2(0,1,0);
    randSeedRepeatable();
    for (size_t ii=0; ii<100; ++ii) {
        Affine3D  s = similarityRand().asAffine();
        Plane       pln = cPlane(s*v0,s*v1,s*v2);
        double      a = randUniform(),
                    b = randUniform(),
                    c = 1.0 - a - b;
        Vec3D       pt = s * (v0*a + v1*b + v2*c);
        double      r = cDot(pt,pln.norm),
                    mag = sqrt(pln.norm.mag()+sqr(pln.scalar));
        FGASSERT(isApproxEqualAbsPrec(-r,pln.scalar,mag));
    }
}

static
void
rayPlaneIntersect()
{
    Vec2D       v0(0,0),
                v1(1,0),
                v2(0,1),
                zero;
    randSeedRepeatable();
    for (size_t ii=0; ii<100; ++ii) {
        Mat22D          rot = matRotate(randUniform()*2.0*pi());
        Vec2D           r0 = rot * v0,
                        r1 = rot * v1,
                        r2 = rot * v2;
        Mat33D          rot3 = matRotateAxis((randUniform()*0.5-0.25)*pi(),Vec3D::randNormal());
        Vec3D           p0 = rot3 * asHomogVec(r0),
                        p1 = rot3 * asHomogVec(r1),
                        p2 = rot3 * asHomogVec(r2),
                        pt = rot3 * asHomogVec(zero + Vec2D::randUniform(-0.1,0.1));
        Plane           pln = cPlane(p0,p1,p2);
        Vec4D           is = linePlaneIntersect(pt*exp(randNormal()),pln);
        FGASSERT(isApproxEqualRelMag(pt,fromHomogVec(is),30));
    }
}

static
void
pit0(Vec2D pt,Vec2D v0,Vec2D v1,Vec2D v2,int res)
{
    FGASSERT(pointInTriangle(pt,v0,v1,v2) == res);
    FGASSERT(pointInTriangle(pt,v0,v2,v1) == res*-1);     //-V764 (PVS Studio)
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
pointInTriangle()
{
    Vec2D    v0(0.0,0.0),
                v1(1.0,0.0),
                v2(0.0,1.0);
    double      d = epsilonD() * 100,
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
lineFacetIntersect()
{
    double          s = 0.1;
    Vec3D           v0(0,0,0),
                    v1(1,0,0),
                    v2(0,1,0);
    Opt<Vec3D>      ret;
    ret = lineTriIntersect(Vec3D(s,s,1),Vec3D(0,0,-1),v0,v1,v2);
    FGASSERT(ret.val() == Vec3D(s,s,0));
    ret = lineTriIntersect(Vec3D(s,s,1),Vec3D(0,0,1),v0,v1,v2);
    FGASSERT(ret.val() == Vec3D(s,s,0));
    ret = lineTriIntersect(Vec3D(-s,-s,1),Vec3D(0,0,1),v0,v1,v2);
    FGASSERT(!ret.valid());
    ret = lineTriIntersect(Vec3D(0,0,1),Vec3D(-s,-s,-1),v0,v1,v2);
    FGASSERT(!ret.valid());
    ret = lineTriIntersect(Vec3D(0,0,1),Vec3D(s,s,-1),v0,v1,v2);
    FGASSERT(ret.val() == Vec3D(s,s,0));
}

// Test tensor-based 3D triangle / parallelogram area formula:
static void
triTensorArea()
{
    randSeedRepeatable();
    for (size_t tt=0; tt<10; ++tt) {
        // tri verts in CC winding order:
        Vec3D           pnts[3] {Vec3D::randNormal(),Vec3D::randNormal(),Vec3D::randNormal()},
        // parallelogram area vectors:
                        area0 = crossProduct(pnts[1]-pnts[0],pnts[2]-pnts[0]),  // traditional formula
                        area1 {0};                                              // tensor formula
        for (uint ii=0; ii<3; ++ii) {               // even permutation tensor
            uint            jj = (ii+1)%3,
                            kk = (ii+2)%3;
            for (uint xx=0; xx<3; ++xx) {               // alternating tensor even permutes
                uint            yy = (xx+1)%3,
                                zz = (xx+2)%3;
                area1[xx] += pnts[jj][yy] * pnts[kk][zz];   // even permutation of alternating tensor
                area1[xx] -= pnts[jj][zz] * pnts[kk][yy];   // odd "
            }
        }
        FGASSERT(isApproxEqualRelPrec(area0,area1,30U));
    }
}

}   // namespace

void
testGeometry(CLArgs const &)
{
    randSeedRepeatable();
    closestBarycentricPoint();
    originToSegmentDistSqr();
    pointToFacetDistSqr();
    barycentricCoords();
    barycentricCoords3D();
    planeH();
    rayPlaneIntersect();
    pointInTriangle();
    lineFacetIntersect();
    triTensorArea();
}

void
testmGeometry(CLArgs const &)
{
    // Give visual feedback on continuity along a line passing through all 3 cases;
    // facet, edge and vertex closest point:
    const uint      ns = 800;
    Vec3D           v0(0.0,0.0,1.0),
                    v1(1.0,1.0,1.0),
                    v2(2.0,-1.0,1.0),
                    pnt(-1.0,0.0,0.0);
    vector<double>  func;
    double          step = 6.0 / double(ns-1);
    for (uint ii=0; ii<ns; ++ii) {
        pnt[0] += step;
        func.push_back(closestPointInTri(pnt,v0,v1,v2).mag);
    }
    uint            sz = ns-2;
    MatD            derivs(sz,3);
    for (size_t ii=0; ii<sz; ++ii) {
        derivs.rc(ii,0) = func[ii+1];
        derivs.rc(ii,1) = func[ii+2]-func[ii];
        derivs.rc(ii,2) = func[ii+2] + func[ii] - 2.0 * func[ii+1];
    }
    fgDrawFunctions(derivs);
}

}   // namespace Fg
