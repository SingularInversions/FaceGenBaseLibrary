//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgGeometry.hpp"
#include "FgSerial.hpp"
#include "FgMatrixSolver.hpp"
#include "FgCommand.hpp"
#include "FgKdTree.hpp"
#include "FgBounds.hpp"
#include "FgRandom.hpp"
#include "FgTransform.hpp"
#include "FgMath.hpp"
#include "FgApproxEqual.hpp"
#include "FgImageDraw.hpp"
#include "FgMain.hpp"
#include "Fg3dMeshIo.hpp"
#include "Fg3dDisplay.hpp"
#include "FgArray.hpp"
#include "FgFileSystem.hpp"

using namespace std;

namespace Fg {

double              cArea(Vec2D p0,Vec2D p1,Vec2D p2)
{
    return (p1[0]-p0[0]) * (p2[1]-p0[1]) - (p1[1]-p0[1]) * (p2[0]-p0[0]);
}

bool                isPointLeftOfLine(Vec2D begin,Vec2D end,Vec2D pnt,double tol)
{
    Vec2D               del = end-begin;
    double              mag = cMagD(del);
    if (mag == 0)
        return false;
    Vec2D               deln = del / sqrt(mag);     // required for 'tol' units to be respected
    Vec2D               norm {-deln[1],deln[0]};    // level set function increases in this direction
    double              c = -cDot(norm,begin);      // line signed distance from origin (at closest point on line)
    return (cDot(pnt,norm)+c > tol);
}
void                testIsPointLeftOfLine(CLArgs const &)
{
    double constexpr    eps = 1.0/(1<<20);
    double constexpr    tol = eps/2;
    Arr<Vec2D,2>        line {{0,0},{1,0},};
    Vec2Ds              left  {{-eps,eps},{0,eps},{0.5,eps},{1,eps},{2,eps},},
                        right {{-eps,-eps},{0,-eps},{0.5,-eps},{1,-eps},{2,-eps},};
    for (Vec2D l : left)
        FGASSERT(isPointLeftOfLine(line[0],line[1],l,tol));
    for (Vec2D r : right)
        FGASSERT(!isPointLeftOfLine(line[0],line[1],r,tol));
    for (size_t ii=0; ii<100; ++ii) {
        double              scale = exp(randNormal());
        Mat22D              rot = matRotate(randUniform(0,tau)) * scale;
        Vec2D               trans = Vec2D::randNormal() * scale;
        Affine2D            xf {rot,trans};
        Arr<Vec2D,2>        lineXf = mapMul(xf,line);
        for (Vec2D l : left)
            FGASSERT(isPointLeftOfLine(lineXf[0],lineXf[1],xf*l,tol*scale));
        for (Vec2D r : right)
            FGASSERT(!isPointLeftOfLine(lineXf[0],lineXf[1],xf*r,tol*scale));
    }
}

void                testIsPointInTri(CLArgs const &)
{
    double constexpr    eps = 1.0/(1<<20);
    double constexpr    tol = eps/2;
    Arr<Vec2D,3>        tri {{-1,1},{-1,-1},{1,0},};
    Vec2Ds              inside {{eps-1,0},{eps-1,1-2*eps},{0,0.5-2*eps},{0,0},},
                        border {{-1,0},{-1,1},{0,0.5},{1,0},};
    for (size_t ii=0; ii<100; ++ii) {
        double              scale = exp(randNormal());
        Mat22D              rot = matRotate(randUniform(0,tau)) * scale;
        if (ii > 50)
            rot = rot * Mat22D{-1,0,0,1};       // switch to LHR CS
        Vec2D               trans = Vec2D::randNormal() * scale;
        Affine2D            xf {rot,trans};
        Arr<Vec2D,3>        triXf = mapMul(xf,tri);
        if (ii > 50)
            swap(triXf[1],triXf[2]);            // switch to CW winding for LHR CS
        for (Vec2D p : inside)
            FGASSERT(isPointInTri(xf*p,triXf,tol*scale));
        for (Vec2D p : border)
            FGASSERT(!isPointInTri(xf*p,triXf,tol*scale));
    }
}

int                 pointInTriangle(Vec2D pt,Vec2D v0,Vec2D v1,Vec2D v2)
{
    Vec2D               s0 = v1-v0,
                        s1 = v2-v1,
                        s2 = v0-v2,
                        p0 = pt-v0,
                        p1 = pt-v1,
                        p2 = pt-v2,
                        zero {0};
    double              d0 = s0[0]*p0[1]-s0[1]*p0[0],
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
void                pit0(Vec2D pt,Vec2D v0,Vec2D v1,Vec2D v2,int res)
{
    FGASSERT(pointInTriangle(pt,v0,v1,v2) == res);
    FGASSERT(pointInTriangle(pt,v0,v2,v1) == res*-1);     //-V764 (PVS Studio)
}
void                pit1(Vec2D pt,Vec2D v0,Vec2D v1,Vec2D v2,int res)
{
    for (size_t ii=0; ii<5; ++ii) {
        Mat22D          rot = matRotate(randUniform()*2.0*pi);
        Vec2D           trn(randUniform(),randUniform());
        Affine2D        s(rot,trn);
        pit0(s*pt,s*v0,s*v1,s*v2,res); }
}
void                testPointInTriangle(CLArgs const &)
{
    Vec2D    v0(0.0,0.0),
                v1(1.0,0.0),
                v2(0.0,1.0);
    double      d = lims<double>::epsilon() * 100,
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

Opt<Arr3D>          cBarycentricCoord(Vec3D point,Vec3D v0,Vec3D v1,Vec3D v2)
{
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
        return {};
    Mat23D          xf;
    xf[0] = e02[1]*v0[2] - e02[2]*v0[1];
    xf[1] = e02[2]*v0[0] - e02[0]*v0[2];
    xf[2] = e02[0]*v0[1] - e02[1]*v0[0];
    xf[3] = e01[2]*v0[1] - e01[1]*v0[2];
    xf[4] = e01[0]*v0[2] - e01[2]*v0[0];
    xf[5] = e01[1]*v0[0] - e01[0]*v0[1];
    xf *= 1 / det;
    Vec2D           bc = xf * point;
    return Arr3D{1-bc[0]-bc[1],bc[0],bc[1]};
}

Plane               cPlane(Vec3D p0,Vec3D p1,Vec3D p2)
{
    Vec3D               unorm = crossProduct(p1-p0,p2-p0);    // unnormalized surface direction (CC winding)
    double              mag = cMagD(unorm),
                        // normalizing factor will be 0 if points are degenerate, giving invalid() Plane
                        fac = (mag == 0.0) ? 0.0 : 1.0 / sqrt(mag);
    Vec3D               norm = unorm * fac;
    // desired plane equation is n . p + s = 0, hence s = - n . p
    return {norm,-cDot(norm,p0)};
}

Plane               cTangentPlane(Vec3D const & query,Vec3DMag const & delta)
{
                        // an invalid delta will result in an invalid Plane:
    double              mag = delta.valid() ? delta.mag : 0.0,
                        // and zero delta will also result in an invalid Plane:
                        fac = (mag == 0.0) ? 0.0 : 1.0 / sqrt(mag);
    Vec3D               norm = delta.vec * fac;
    // desired plane equation is n . (q+d) + s = 0, hence s = - n . (q+d)
    return {norm,-cDot(norm,query+delta.vec)};
}

Vec4D               linePlaneIntersect(Vec3D r,Plane p)
{
    double  a = -p.offset;
    return Vec4D(a*r[0],a*r[1],a*r[2],cDot(r,p.norm));
}

Opt<Vec3D>          lineTriIntersect(Vec3D pnt,Vec3D dir,Vec3D v0,Vec3D v1,Vec3D v2)
{
    Opt<Vec3D>          ret;
    // First calculate intersection with plane (in CS with 'pnt' as origin):
    Vec3D               p0 = v0-pnt;
    Plane               plane = cPlane(p0,v1-pnt,v2-pnt);
    Vec4D               isectH = linePlaneIntersect(dir,plane);
    if (isectH[3] != 0.0) {     // line can't be parallel to facet plane
        Vec3D                isect = projectHomog(isectH);
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

Vec2D               flipTri(Arr<Vec2D,3> const & tri,size_t flipIdx)
{
    FGASSERT(flipIdx < 3);
    size_t              v0Idx = (flipIdx + 1) % 3,      // opposing edge v0
                        v1Idx = (flipIdx + 2) % 3;      // opposing edge v1
    Vec2D               v0 = tri[v0Idx],
                        v1 = tri[v1Idx],
                        vf = tri[flipIdx];
    Vec2D               e01 = v1 - v0,
                        e0f = vf - v0,
                        perp = e0f - e01 * (cDot(e01,e0f) / cMag(e01));
    return v0 + e0f - perp * 2;
}
static void         testFlipTri(CLArgs const &)
{
    Vec2D               v0 {-1,0},
                        v1 {1,0};
    for (size_t ii=0; ii<1000; ++ii) {
        double              x = randUniform(-2,2),
                            y = randUniform(0.1,10.0);
        Vec2D               v2 {x,y},
                            vf {x,-y};
        Mat22D              xf = matRotate(randUniform(-pi,pi));
        Arr<Vec2D,3>        tri {v0,v1,v2};
        Vec2D               tst = flipTri(mapMul(xf,tri),2),
                            ref = xf * vf;
        FGASSERT(isApproxEqual(tst,ref,epsBits(30)));
    }
}

static double       xformErr(Vec3Fs const & verts,Affine3F const & mirror)
{
    KdTree              kdt(verts);
    double              err = 0.0;
    for (Vec3F vert : verts)
        err += kdt.findClosest(mirror * vert).distMag;
    return err;
}

double              findSaggitalSymmetry(Vec3Fs const & verts,Affine3F & mirror)
{
    Vec3F           centre = cMean(verts);
    Mat33F          mirrorX {-1,0,0, 0,1,0, 0,0,1};
    double          bestErr = numeric_limits<double>::max();
    Affine3F        bestXform;
    for (uint dim=0; dim<3; ++dim) {
        Affine3F        symmToXNull(-centre,cRotationPermuter<float>(dim)),
                        guess(symmToXNull.inverse() * (mirrorX * symmToXNull));
        double          err = xformErr(verts,guess);
        if (err < bestErr) {
            bestErr = err;
            bestXform = guess;
        }
    }
    mirror = bestXform;
    Mat32F          bounds = cBounds(verts);
    Vec3F           dims = bounds * Vec2F{-1,1};
    double          rmsErr = sqrt(bestErr / double(verts.size()));
    return (rmsErr / cLenD(dims));
}

Quadratic3D         operator+(Quadratic3D const & l,Quadratic3D const & r)
{
    MatS3D              qform = l.qform + r.qform;
    Vec3D               centre = cInverse(qform) * (l.qform*l.centre + r.qform*r.centre);
    double              minVal = l(centre) + r(centre);
    return {qform,centre,minVal};
}

bool                isApproxEqual(Quadratic const & l,Quadratic const & r,double tol)
{
    return
        isApproxEqual(l.precision,r.precision,tol) &&
        isApproxEqual(l.vertex,r.vertex,tol) &&
        isApproxEqual(l.vertVal,r.vertVal,tol);
}

QuadraticZ3D        operator*(Mat33D const & M,QuadraticZ3D const & Q)
{
    return {
        cCongruentTransform(Q.qform,cInverse(M)),
        M * Q.centre
    };
}

QuadraticZ3D        operator*(Rigid3D const & R,QuadraticZ3D const & Q)
{
    // y = Mx, want S' | x^T S x = y^T S' y = x^T M^T S' M x (for all x)
    // S = M^T S' M
    // S' = M^T^-1 S M^-1   (we know M is invertible since it's rigid motion)
    //    = M^-1^T S M^-1   (tranpose inverse == inverse transpose)
    //    = M S M^-1        (for the pure rotation part)
    Mat33D              ria = R.rot.inverse().asMatrix();
    return {
        cCongruentTransform(Q.qform,ria),
        R.asAffine() * Q.centre
    };
}
static void         testRigid3DMulQuadraticZ3D(CLArgs const &)
{
    auto                fn = []()
    {
        Rigid3D             R = Rigid3D::randNormal(1);
        QuadraticZ3D        Q {MatS3D::randSpd(1),Vec3D::randNormal(1)},
                            S = R * Q;
        for (size_t ii=0; ii<10; ++ii) {
            Vec3D               p = Vec3D::randNormal(1);
            double              tst = Q(p),
                                ref = S(R.asAffine()*p);
            FGASSERT(isApproxEqualPrec(tst,ref,30));
        }
    };
    repeat(fn,10);
}
QuadraticZ3Ds       mapTransform(Rigid3D const & r,QuadraticZ3Ds const & qs)
{
    Affine3D            ra = r.asAffine();
    Mat33D              ria = r.rot.inverse().asMatrix();
    auto                fn = [&](QuadraticZ3D const & q) -> QuadraticZ3D
    {
        return {
            cCongruentTransform(q.qform,ria),
            ra * q.centre,
        };
    };
    return mapCall(qs,fn);
}

Quadratic           isectRayQuadratic(QuadraticZ3D const & Q,Vec3D ray)
{
    // see Design -> Quadratics:
    double              lambda = applyQform(Q.qform,ray),
                        na = cDot(ray,Q.qform*Q.centre),
                        b = 0.5 * applyQform(Q.qform,Q.centre);
    return {lambda,na/lambda,b-0.5*sqr(na)/lambda};
}
static void         testIsectRayQuadratic(CLArgs const &)
{
    for (size_t ii=0; ii<4; ++ii) {
        // simple test with unit isotropic quadratic where we know what the answer should be:
        Vec2D               trans {randNormal(),randNormal()};
        Vec3D               vertex {exp(randNormal()),trans[0],trans[1]},
                            ray {1,0,0};
        QuadraticZ3D        Q {MatS3D{{1,1,1},{0,0,0}},vertex};
        Quadratic           tst = isectRayQuadratic(Q,ray),
                            ref {1,vertex[0],0.5*cMag(trans)};
        FGASSERT(isApproxEqual(tst,ref,epsBits(30)));
        // now solve in random linear transformed space and check answer:
        for (size_t jj=0; jj<4; ++jj) {
            Mat33D              xf = cRandPositiveDefinite3D();
            Quadratic           tst2 = isectRayQuadratic(xf*Q,xf*ray);
            FGASSERT(isApproxEqual(tst2,ref,epsBits(30)));
        }
    }
}

namespace {

void                testClosestPointInSegment(CLArgs const &)
{
    double constexpr    eps = epsBits(30);
    Vec2D               pB1 {-2,1},
                        pA1 {-1,1},
                        p01 {0,1},
                        p11 {1,1},
                        p21 {1,1};
    auto                fn = [](Vec2D x,Vec2D y,double ref)
    {
            FGASSERT(isApproxEqual(cMagD(closestPointOnSegmentToOrigin<double,2>({x,y})),ref,eps));
    };
    fn(p01,p01,1.0);
    fn(pB1,pA1,2.0);
    fn(pA1,p01,1.0);
    fn(p01,p11,1.0);
    fn(p11,p21,2.0);
    fn(pA1,p11,1.0);
    fn(pB1,p11,1.0);
    fn(pA1,p21,1.0);
}

void                testBarycentricCoord(CLArgs const &)
{
    randSeedRepeatable();
    // Test points inside triangle:
    for (uint ii=0; ii<100; ++ii) {
        Arr<Vec2D,3>    v;
        for (Vec2D & e : v) e = Vec2D::randNormal();    // deterministic order
        Vec2D           del1 = v[1]-v[0],
                        del2 = v[2]-v[0];
        if ((del1[0]*del2[1]-del1[1]*del2[0]) > 0.001) {
            Arr3D           c;
            c[0] = randUniform();                       // deterministic order
            c[1] = randUniform() * (1.0 - c[0]);
            c[2] = 1.0 - c[1] - c[0];
            Vec2D           pnt = multAcc(v,c);
            Arr3D           res = cBarycentricCoord(pnt,v).value();
            FGASSERT(cMinElem(res)>=0.0f);     // Inside.
            Vec2D           chk = multAcc(v,res);
            FGASSERT(isApproxEqualRelMag(pnt,chk,30));
            FGASSERT(isApproxEqualPrec(res[0]+res[1]+res[2],1.0));
        }
    }
    // Test points outside triangle:
    for (uint ii=0; ii<100; ++ii) {
        Arr<Vec2D,3>    v;
        for (Vec2D & e : v) e = Vec2D::randNormal();    // deterministic order
        Vec2D           del1 = v[1]-v[0],
                        del2 = v[2]-v[0];
        if ((del1[0]*del2[1]-del1[1]*del2[0]) > 0.001) {
            Vec3D           c;
            c[0] = -randUniform();                      // deterministic order
            c[1] = randUniform() * (1.0 - c[0]);
            c[2] = 1.0 - c[1] - c[0];
            c = cRotationPermuter<double>(ii%3) * c;
            Vec2D           pnt = multAcc(v,c.m);
            Arr3D           res = cBarycentricCoord(pnt,v).value();
            FGASSERT(cMinElem(res)<0.0f);     // Outside
            Vec2D           chk = multAcc(v,res);
            FGASSERT(isApproxEqualRelMag(pnt,chk,30));
            FGASSERT(isApproxEqualPrec(res[0]+res[1]+res[2],1.0));
        }
    }
}

void                testBarycentricCoord3(CLArgs const &)
{
    randSeedRepeatable();
    fgout << fgnl << "Barycentric 3d: " << fgpush;
    for (uint ii=0; ii<50; ++ii) {
        Vec3D       v0 = Vec3D::randNormal(),
                    v1 = Vec3D::randNormal(),
                    v2 = Vec3D::randNormal();
        Arr3D       bc = Vec3D::randNormal().m;
        bc /= bc[0] + bc[1] + bc[2];
        Vec3D    pt = bc[0]*v0 + bc[1]*v1 + bc[2]*v2;
        Opt<Arr3D>    ret = cBarycentricCoord(pt,v0,v1,v2);
        if (ret.has_value()) {
            Arr3D           res = ret.value(),
                            delta = res-bc;
            //fgout << fgnl << bc << " -> " << res << " delta: " << res-bc;
            FGASSERT(cMag(delta) < sqr(0.000001));
        }
    }
    fgout << fgpop;
}

void                testPlane(CLArgs const &)
{
    Vec3D           e0 {0,0,0},
                    e1 {1,0,0},
                    e2 {0,1,0},
                    e3 {0,0,1};
    {
        Plane               p = cPlane(e0,e1,e2);       // i,j,k
        FGASSERT((p.norm == e3) && (p.offset == 0.0));
        p = cPlane(e1,e1*2,e1*3);                       // colinear
        FGASSERT(p.invalid());
        p = cPlane(e1,e2,e2);                           // degenerate
        FGASSERT(p.invalid());
    }
    randSeedRepeatable();
    for (size_t ii=0; ii<100; ++ii) {
        SimilarityD         sim = similarityRand();
        Affine3D            xf = sim.asAffine();
        Plane               plane = cPlane(xf*e0,xf*e1,xf*e2);
        double              a = randUniform(),
                            b = randUniform(),
                            c = 1.0 - a - b,                // a,b,c are trilinear coords
                            d = randNormal();               // test *signed* distance
        Vec3D               p0 = e0*a + e1*b + e2*c,        // on plane
                            p1 = p0 + e3 * d,               // off plane
                            query0 = xf * p0,
                            query1 = xf * p1;
        FGASSERT(isApproxEqual(plane.distance(query0),0.0,epsBits(30)));
        FGASSERT(isApproxEqual(plane.distance(query1),d*sim.scale,epsBits(30)));
    }
}

void                testLinePlaneIntersect(CLArgs const &)
{
    Arr<Vec2D,3>        pts {{0,0},{1,0},{0,1},};
    randSeedRepeatable();
    for (size_t ii=0; ii<100; ++ii) {
        Mat22D              rot = matRotate(randUniform()*2.0*pi);
        Arr<Vec2D,3>        rs = mapMul(rot,pts);
        Mat33D              rot3 = QuaternionD::rand().asMatrix();
        Arr<Vec3D,3>        ps = mapCall(rs,[rot3](Vec2D p){return rot3 * append(p,1.0); });
        Vec3D               pt = rot3 * append(Vec2D::randUniform(-0.1,0.1),1.0);
        Plane               pln = cPlane(ps);
        Vec4D               is = linePlaneIntersect(pt*exp(randNormal()),pln);
        FGASSERT(isApproxEqualRelMag(pt,projectHomog(is),30));
    }
}

void                testLineTriIntersect(CLArgs const &)
{
    double          s = 0.1;
    Vec3D           v0(0,0,0),
                    v1(1,0,0),
                    v2(0,1,0);
    Opt<Vec3D>      ret;
    ret = lineTriIntersect(Vec3D(s,s,1),Vec3D(0,0,-1),v0,v1,v2);
    FGASSERT(ret.value() == Vec3D(s,s,0));
    ret = lineTriIntersect(Vec3D(s,s,1),Vec3D(0,0,1),v0,v1,v2);
    FGASSERT(ret.value() == Vec3D(s,s,0));
    ret = lineTriIntersect(Vec3D(-s,-s,1),Vec3D(0,0,1),v0,v1,v2);
    FGASSERT(!ret.has_value());
    ret = lineTriIntersect(Vec3D(0,0,1),Vec3D(-s,-s,-1),v0,v1,v2);
    FGASSERT(!ret.has_value());
    ret = lineTriIntersect(Vec3D(0,0,1),Vec3D(s,s,-1),v0,v1,v2);
    FGASSERT(ret.value() == Vec3D(s,s,0));
}

// Test levi-civita tensor-based parallelogram area formula:
void                testTensorArea(CLArgs const &)
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

void                testFindSaggSymm(CLArgs const & args)
{
    Mesh                baseMesh = loadTri(dataDir() + "tools/internal/InternalBaseFace.tri");
    Affine3F            mirror;
    findSaggitalSymmetry(baseMesh.verts,mirror);
    Mesh                tmpMesh = baseMesh;
    mapMul_(mirror,tmpMesh.verts);
    if (!isAutomated(args))
        viewMesh({baseMesh,tmpMesh},true);
}

}   // namespace

void                testGeometry(CLArgs const & args)
{
    Cmds            cmds {
        {testClosestPointInSegment,"cps","closet point in line segment"},
        {testFlipTri,"flip","flipTri()"},
        {testLineTriIntersect,"lineTri","line triangle intersect"},
        {testLinePlaneIntersect,"lpi","line plane intersect"},
        {testIsPointLeftOfLine,"pll","point left of oriented line"},
        {testBarycentricCoord,"ptb","point to barycentric coord"},
        {testBarycentricCoord3,"ptb3","point to barycentric coord 3D"},
        {testPlane,"plane","plane creation and closest point to plane in 3D"},
        {testPointInTriangle,"pit","point in triangle"},
        {testIsPointInTri,"pitt","point in triangle with tolerance"},
        {testFindSaggSymm,"sagg","find saggital symmetry"},
        {testTensorArea,"tensor","levi-civita tensor area formula"},
        {testIsectRayQuadratic,"quad","quadratics operations"},
        {testRigid3DMulQuadraticZ3D,"zqr","zero-extrema quadratic rigid transform"},
    };
    doMenu(args,cmds,true);
}

}
