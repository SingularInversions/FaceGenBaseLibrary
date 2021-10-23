//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgGeometry.hpp"
#include "FgStdVector.hpp"
#include "FgMatrixSolver.hpp"
#include "FgCommand.hpp"
#include "FgKdTree.hpp"
#include "FgBounds.hpp"
#include "FgRandom.hpp"
#include "FgSimilarity.hpp"
#include "FgMath.hpp"
#include "FgApproxEqual.hpp"
#include "FgImageDraw.hpp"
#include "FgMain.hpp"
#include "Fg3dMeshIo.hpp"
#include "Fg3dDisplay.hpp"
#include "FgArray.hpp"

using namespace std;

namespace Fg {

double
cArea(Vec2D p0,Vec2D p1,Vec2D p2)
{
    return (p1[0]-p0[0]) * (p2[1]-p0[1]) - (p1[1]-p0[1]) * (p2[0]-p0[0]);
}

VecMagD
closestPointInSegment(Vec3D const & p0,Vec3D const & p1)
{
    VecMagD    ret;
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

Opt<Vec3D>
cBarycentricCoord(Vec2D point,Vec2D v0,Vec2D v1,Vec2D v2)
{
    Opt<Vec3D>      ret;
    Vec2D           u0 = v0-point,
                    u1 = v1-point,
                    u2 = v2-point;
    double          c0 = u1[0]*u2[1] - u2[0]*u1[1],
                    c1 = u2[0]*u0[1] - u0[0]*u2[1],
                    c2 = u0[0]*u1[1] - u1[0]*u0[1],
                    d = c0+c1+c2;
    if (d == 0.0)       // All segments from point to vertices are colinear
        return ret;
    ret = Vec3D(c0/d,c1/d,c2/d);
    return ret;
}

Opt<Vec3D>
cBarycentricCoord(Vec3D point,Vec3D v0,Vec3D v1,Vec3D v2)
{
    Opt<Vec3D>      ret;
    // Transform into the coordinate system v0 = (0,0), v1 = (1,0), v2 = (0,1) by defining
    // the inverse homogeneous transform then inverting and taking only the top 2 rows to
    // form 'xf' below. Converting to barycentric is then trivial, and barycentric coords
    // are preserved by linear xforms.
    Vec3D           e01 = v1-v0,
                    e02 = v2-v0;
    double          det =
        e01[0]*e02[1]*v0[2] + e02[0]*v0[1]*e01[2] + e01[1]*e02[2]*v0[0] -
        e01[2]*e02[1]*v0[0] - e01[1]*e02[0]*v0[2] - e02[2]*v0[1]*e01[0];
    if (det == 0)
        return ret;
    Mat23D          xf;
    xf[0] = e02[1]*v0[2] - e02[2]*v0[1];
    xf[1] = e02[2]*v0[0] - e02[0]*v0[2];
    xf[2] = e02[0]*v0[1] - e02[1]*v0[0];
    xf[3] = e01[2]*v0[1] - e01[1]*v0[2];
    xf[4] = e01[0]*v0[2] - e01[2]*v0[0];
    xf[5] = e01[1]*v0[0] - e01[0]*v0[1];
    xf *= 1 / det;
    Vec2D           bc = xf * point;
    return Vec3D(1-bc[0]-bc[1],bc[0],bc[1]);
}

Plane
cPlane(Vec3D p0,Vec3D p1,Vec3D p2)
{
    Plane           ret;
    ret.norm = crossProduct(p1-p0,p2-p0);    // Surface normal, not normalized
    FGASSERT(ret.norm.mag() > 0.0);
    ret.scalar = -cDot(ret.norm,p0);
    return ret;
}

Vec4D
linePlaneIntersect(Vec3D r,Plane p)
{
    double  a = -p.scalar;
    return Vec4D(a*r[0],a*r[1],a*r[2],cDot(r,p.norm));
}

int
pointInTriangle(Vec2D pt,Vec2D v0,Vec2D v1,Vec2D v2)
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
lineTriIntersect(Vec3D pnt,Vec3D dir,Vec3D v0,Vec3D v1,Vec3D v2)
{
    Opt<Vec3D>         ret;
    // First calculate intersection with plane (in CS with 'pnt' as origin):
    Vec3D               p0 = v0-pnt;
    Plane               plane = cPlane(p0,v1-pnt,v2-pnt);
    Vec4D               isectH = linePlaneIntersect(dir,plane);
    if (isectH[3] != 0.0) {     // line can't be parallel to facet plane
        Vec3D                isect = fromHomogVec(isectH);
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

static double
xformErr(Vec3Fs const & verts,Affine3F const & mirror)
{
    KdTree              kdt(verts);
    double              err = 0.0;
    for (Vec3F vert : verts)
        err += kdt.findClosest(mirror * vert).distMag;
    return err;
}

double
findSaggitalSymmetry(Vec3Fs const & verts,Affine3F & mirror)
{
    Vec3F           centre = cMean(verts);
    Mat33F          mirrorX {-1,0,0, 0,1,0, 0,0,1};
    double          bestErr = numeric_limits<double>::max();
    Affine3F        bestXform;
    for (uint dim=0; dim<3; ++dim) {
        Affine3F        symmToXNull(-centre,permuteAxes<float>(dim)),
                        guess(symmToXNull.inverse() * (mirrorX * symmToXNull));
        double          err = xformErr(verts,guess);
        if (err < bestErr) {
            bestErr = err;
            bestXform = guess;
        }
    }
    mirror = bestXform;
    Mat32F          bounds = cBounds(verts);
    Vec3F           dims = bounds.colVec(1) - bounds.colVec(0);
    double          rmsErr = sqrt(bestErr / double(verts.size()));
    return (rmsErr / dims.len());
}

namespace {

void
testClosestPointInSegment(CLArgs const &)
{
    Vec3D    p0(1.0,0.0,0.0),
                p1(0.0,1.0,0.0),
                p2(2.0,-1.0,0.0);
    Mat33D          rot = Mat33D::identity();
    for (uint ii=0; ii<10; ++ii) {
        Vec3D    r0 = rot * p0,
                    r1 = rot * p1,
                    r2 = rot * p2;
        VecMagD    delta;
        // Degenerate case:
        delta = closestPointInSegment(r0,r0);
        FGASSERT(isApproxEqualPrec(delta.mag,1.0));
        FGASSERT(isApproxEqualRelMag(delta.vec,rot * Vec3D(1.0,0.0,0.0),30));
        // edge closest point (both directions):
        delta = closestPointInSegment(r0,r1);
        FGASSERT(isApproxEqualPrec(delta.mag,0.5));
        FGASSERT(isApproxEqualRelMag(delta.vec,rot * Vec3D(0.5,0.5,0.0),30));
        delta = closestPointInSegment(r1,r2);
        FGASSERT(isApproxEqualPrec(delta.mag,0.5));
        FGASSERT(isApproxEqualRelMag(delta.vec,rot * Vec3D(0.5,0.5,0.0),30));
        // vertex closest point:
        delta = closestPointInSegment(r0,r2);
        FGASSERT(isApproxEqualPrec(delta.mag,1.0));
        FGASSERT(isApproxEqualRelMag(delta.vec,rot * Vec3D(1.0,0.0,0.0),30));
        rot = QuaternionD::rand().asMatrix();
    }
}

void
testBarycentricCoord(CLArgs const &)
{
    randSeedRepeatable();
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
            Vec3D    res = cBarycentricCoord(pnt,v0,v1,v2).val();
            FGASSERT(cMinElem(res)>=0.0f);     // Inside.
            Vec2D    chk = v0*res[0] + v1*res[1] + v2*res[2];
            FGASSERT(isApproxEqualRelMag(pnt,chk,30));
            FGASSERT(isApproxEqualPrec(res[0]+res[1]+res[2],1.0));
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
            Vec3D        res = cBarycentricCoord(pnt,v0,v1,v2).val();
            FGASSERT(cMinElem(res)<0.0f);     // Outside
            Vec2D        chk = v0*res[0] + v1*res[1] + v2*res[2];
            FGASSERT(isApproxEqualRelMag(pnt,chk,30));
            FGASSERT(isApproxEqualPrec(res[0]+res[1]+res[2],1.0));
        }
    }
}

void
testBarycentricCoord3(CLArgs const &)
{
    randSeedRepeatable();
    fgout << fgnl << "Barycentric 3d: " << fgpush;
    for (uint ii=0; ii<50; ++ii) {
        Vec3D    v0 = Vec3D::randNormal(),
                    v1 = Vec3D::randNormal(),
                    v2 = Vec3D::randNormal(),
                    bc = Vec3D::randNormal();
        bc /= bc[0] + bc[1] + bc[2];
        Vec3D    pt = bc[0]*v0 + bc[1]*v1 + bc[2]*v2;
        Opt<Vec3D>    ret = cBarycentricCoord(pt,v0,v1,v2);
        if (ret.valid()) {
            Vec3D        res = ret.val(),
                            delta = res-bc;
            //fgout << fgnl << bc << " -> " << res << " delta: " << res-bc;
            FGASSERT(delta.mag() < sqr(0.000001));
        }
    }
    fgout << fgpop;
}

void
testPlane(CLArgs const &)
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

void
testLinePlaneIntersect(CLArgs const &)
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
        Mat33D          rot3 = matRotateAxis((randUniform()*0.5-0.25)*pi(),normalize(Vec3D::randNormal()));
        Vec3D           p0 = rot3 * asHomogVec(r0),
                        p1 = rot3 * asHomogVec(r1),
                        p2 = rot3 * asHomogVec(r2),
                        pt = rot3 * asHomogVec(zero + Vec2D::randUniform(-0.1,0.1));
        Plane           pln = cPlane(p0,p1,p2);
        Vec4D           is = linePlaneIntersect(pt*exp(randNormal()),pln);
        FGASSERT(isApproxEqualRelMag(pt,fromHomogVec(is),30));
    }
}

void
pit0(Vec2D pt,Vec2D v0,Vec2D v1,Vec2D v2,int res)
{
    FGASSERT(pointInTriangle(pt,v0,v1,v2) == res);
    FGASSERT(pointInTriangle(pt,v0,v2,v1) == res*-1);     //-V764 (PVS Studio)
}

void
pit1(Vec2D pt,Vec2D v0,Vec2D v1,Vec2D v2,int res)
{
    for (size_t ii=0; ii<5; ++ii) {
        Mat22D     rot = matRotate(randUniform()*2.0*pi());
        Vec2D        trn(randUniform(),randUniform());
        Affine2D      s(rot,trn);
        pit0(s*pt,s*v0,s*v1,s*v2,res); }
}

void
testPointInTriangle(CLArgs const &)
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

void
testLineTriIntersect(CLArgs const &)
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

// Test levi-civita tensor-based parallelogram area formula:
void
testTensorArea(CLArgs const &)
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
        FGASSERT(isApproxEqualPrec(area0,area1,30U));
    }
}

void
testmFindSaggitalSymmetry(CLArgs const &)
{
    Mesh                baseMesh = loadTri(dataDir() + "tools/internal/InternalBaseFace.tri");
    Affine3F            mirror;
    findSaggitalSymmetry(baseMesh.verts,mirror);
    Mesh                tmpMesh = baseMesh;
    mapMul_(mirror,tmpMesh.verts);
    viewMesh({baseMesh,tmpMesh},true);
}


}   // namespace

void
testGeometry(CLArgs const & args)
{
    Cmds            cmds {
        {testClosestPointInSegment,"cps","closest point in segment"},
        {testBarycentricCoord,"ptb","point to barycentric coord"},
        {testBarycentricCoord3,"ptb3","point to barycentric coord 3D"},
        {testPlane,"ptp","points to plane 3D"},
        {testLinePlaneIntersect,"lpi","line plane intersect"},
        {testPointInTriangle,"pit","point in triangle 2D"},
        {testLineTriIntersect,"lineTri","line triangle intersect"},
        {testTensorArea,"tensor","levi-civita tensor area formula"},
    };
    doMenu(args,cmds,true);
}

void
testmGeometry(CLArgs const & args)
{
    Cmds            cmds = {
        {testmFindSaggitalSymmetry,"sagg","find saggital symmetry"},
    };
    doMenu(args,cmds);
}

}
