//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgTransform.hpp"
#include "FgMatrixSolver.hpp"
#include "FgCommand.hpp"

using namespace std;

namespace Fg {

Affine3D
solveAffine(Vec3Ds const & base,Vec3Ds const & targ)
{
    size_t          V = base.size();
    FGASSERT(targ.size() == V);
    FGASSERT(V > 3);
    // shifting the origin to the base mean before least squares minimization uncouples the
    // translation from the linear transform, making for an easy solution:
    Vec3D           bmean = cMean(base),
                    tmean = cMean(targ);
    Mat33D          bb {0},
                    tb {0};
    for (size_t vv=0; vv<V; ++vv) {
        Vec3D           b = base[vv]-bmean,
                        t = targ[vv]-tmean;
        bb += b * b.transpose();
        tb += t * b.transpose();
    }
    return Affine3D{tmean} * Affine3D{tb*cInverse(bb)} * Affine3D{-bmean};
}

double              tanDeltaMag(QuaternionD const & lhs,QuaternionD const & rhs)
{
    Vec4D    lv = lhs.asVec4(),
                rv = rhs.asVec4();
    // Ensure we compare appropriate relative signs of the vectors:
    if (cDot(lv,rv) < 0.0)
        rv *= -1.0;
    return cMagD(rv-lv);
}

QuaternionD         interpolate(QuaternionD q0, QuaternionD q1,double val)
{
    Vec4D       v0 = q0.asVec4(),
                v1 = q1.asVec4();
    double      dot = cDot(v0,v1);
    // Get closest representation from projective space. The degenerate case is when the dot
    // product is exactly zero so the inequality choice breaks that symmetry:
    if (dot < 0.0)
        v1 *= -1.0;
    // Just lerp then normalize - not a real exponential map but a passable approx:
    Vec4D       vi = v0 * (1.0-val) + v1 * val;
    return QuaternionD {vi}; // normalizes
}

bool                isApproxEqual(QuaternionD const & l,QuaternionD const & r,double prec)
{
    return (
        // must check the 2 equivalent representations:
        (isApproxEqual(l.real,r.real,prec) && isApproxEqual(l.imag,r.imag,prec)) ||
        (isApproxEqual(l.real,-r.real,prec) && isApproxEqual(l.imag,-r.imag,prec))
    );
}

void                testQuaternion(CLArgs const &)
{
    randSeedRepeatable();
    double constexpr    prec = lims<double>::epsilon()*8;
    QuaternionD const   id;
    // Axis rotations:
    for (size_t ii=0; ii<5; ++ii) {
        double          r = randNormal();
        Mat33D          qx = cRotateX(r).asMatrix(),
                        mx = matRotateX(r);
        FGASSERT(isApproxEqual(qx,mx,prec));
        Mat33D          qy = cRotateY(r).asMatrix(),
                        my = matRotateY(r);
        FGASSERT(isApproxEqual(qy,my,prec));
        Mat33D          qz = cRotateZ(r).asMatrix(),
                        mz = matRotateZ(r);
        FGASSERT(isApproxEqual(qz,mz,prec));
    }
    // asMatrix is orthonormal:
    for (size_t ii=0; ii<5; ++ii) {
        QuaternionD     q = QuaternionD::rand();
        Mat33D          m = q.asMatrix(),
                        del = m * m.transpose() - cDiagMat<double,3>(1);
        FGASSERT(cMaxElem(mapAbs(del.m)) < lims<double>::epsilon()*8);
    }
    // Composition:
    for (size_t ii=0; ii<5; ++ii) {
        QuaternionD     q0 = QuaternionD::rand(),
                        q1 = QuaternionD::rand(),
                        q2 = q1 * q0;
        Mat33D          m0 = q0.asMatrix(),
                        m1 = q1.asMatrix(),
                        m2 = m1 * m0,
                        m2q = q2.asMatrix(),
                        del = m2q-m2;
        FGASSERT(cMaxElem(mapAbs(del.m)) < lims<double>::epsilon()*8);
    }
    // Inverse:
    for (size_t ii=0; ii<5; ++ii) {
        QuaternionD     q0 = QuaternionD::rand(),
                        q1 = q0.inverse(),
                        q2 = q1 * q0;
        FGASSERT(sqrt(tanDeltaMag(q2,id)) < lims<double>::epsilon()*8);
    }
    // multiplication keeps values normalized:
    for (size_t ii=0; ii<10; ++ii) {
        QuaternionD         q0 = QuaternionD::rand(),
                            q1 = QuaternionD::rand(),
                            q2 = q0 * q1;
        FGASSERT(isApproxEqual(q2.magD(),1.0,prec));
    }
}

ostream &           operator<<(ostream & os,Rigid3D const & r)
{
    return os << "rot: " << r.rot << " trans: " << r.trans;
}

// SimilarityRD: v' = sR(v + t) = sRv + sRt
SimilarityD::SimilarityD(SimilarityRD const & s) : scale{s.scale}, rot{s.rot}, trans{s.rot*s.trans*s.scale}
{}

SimilarityD         SimilarityD::operator*(SimilarityD const & rhs) const
{
    // Transform:   sRv+t
    // Composition: s'R'(sRv+t)+t'
    //            = s'R'sR(v) + (s'R't+t')
    SimilarityD    ret;
    ret.scale = scale * rhs.scale;
    ret.rot = rot * rhs.rot;
    ret.trans = scale * (rot * rhs.trans) + trans;
    return ret;
}

SimilarityD         SimilarityD::inverse() const
{
    // v' = sRv+t
    // sRv = v' - t
    // v = s'R'v' - s'R't
    double              s = 1.0 / scale;
    QuaternionD         r = rot.inverse();
    Vec3D               t = r * trans * s;
    return SimilarityD {s,r,-t};
}

SimilarityD         similarityRand()
{
    return
        SimilarityD(
            std::exp(randNormal()),
            QuaternionD::rand(),
            Vec3D::randNormal());
}

SimilarityD         solveSimilarity(Vec3Ds const & domain,Vec3Ds const & range)
{
    size_t              V = domain.size();
    FGASSERT(V > 2);                                // not solvable with only 2 points.
    FGASSERT(range.size() == V);                    // must be 1-1
    Vec3D               domMean = cMean(domain),
                        ranMean = cMean(range);
    double              domRayMag {0},
                        ranRayMag {0};
    Mat33D              S {0};
    for (size_t ii=0; ii<V; ii++) {
        Vec3D               domRay = domain[ii] - domMean,
                            ranRay = range[ii] - ranMean;
        domRayMag += domRay.magD();
        ranRayMag += ranRay.magD();
        S += domRay * ranRay.transpose();
    }
    double              scale = sqrt(ranRayMag / domRayMag );
    Mat44D              N {0};
    double              Sxx = S.rc(0,0),   Sxy = S.rc(0,1),   Sxz = S.rc(0,2),
                        Syx = S.rc(1,0),   Syy = S.rc(1,1),   Syz = S.rc(1,2),
                        Szx = S.rc(2,0),   Szy = S.rc(2,1),   Szz = S.rc(2,2);
    // Set the upper triangular elements of N not including the diagonal:
    N.rc(0,1)=Syz-Szy;          N.rc(0,2)=Szx-Sxz;      N.rc(0,3)=Sxy-Syx;
                                N.rc(1,2)=Sxy+Syx;      N.rc(1,3)=Szx+Sxz;
                                                        N.rc(2,3)=Syz+Szy;
    // Since it's symmetric, set the lower triangular (not including diagonal) by:
    N += N.transpose();
    // And set the diagonal elements:
    N.rc(0,0) = Sxx+Syy+Szz;
    N.rc(1,1) = Sxx-Syy-Szz;
    N.rc(2,2) = Syy-Sxx-Szz;
    N.rc(3,3) = Szz-Sxx-Syy;
    // Calculate rotation from N per [Jain '95]. 'cRsmEigs' Leaves largest eigVal in last index:
    QuaternionD         pose {cRsmEigs(N).vecs.colVec(3)};
    // Calculate the 'trans' term: The transform is given by:
    // X = SR(d-dm)+rm = SR(d)-SR(dm)+rm
    Vec3D               trans = -scale * (pose.asMatrix() * domMean) + ranMean;
    return {scale,pose,trans};
}
SimilarityD         solveSimilarity(Vec3Fs const & d,Vec3Fs const & r)
{
    return solveSimilarity(mapCast<Vec3D>(d),mapCast<Vec3D>(r));
}

// as above but for count-weighted samples:
SimilarityD         solveSimilarity(Vec3Ds const & domain,Vec3Ds const & range,Doubles weights)
{
    size_t              V = domain.size();
    FGASSERT(V > 2);                                // not solvable with only 2 points.
    FGASSERT(range.size() == V);                    // must be 1-1
    FGASSERT(weights.size() == V);                  // must be 1-1
    double              wgtTot = cSum(weights);
    Vec3D               domAcc {0},
                        ranAcc {0};
    for (size_t ii=0; ii<V; ++ii) {
        domAcc += domain[ii] * weights[ii];
        ranAcc += range[ii] * weights[ii];
    }
    Vec3D               domMean = domAcc / wgtTot,
                        ranMean = ranAcc / wgtTot;
    double              domRayMag {0},
                        ranRayMag {0};
    Mat33D              S {0};
    for (size_t ii=0; ii<V; ii++) {
        Vec3D               domRay = domain[ii] - domMean,
                            ranRay = range[ii] - ranMean;
        double              wgt = weights[ii];
        domRayMag += domRay.magD() * wgt;
        ranRayMag += ranRay.magD() * wgt;
        S += domRay * ranRay.transpose() * wgt;
    }
    double              scale = sqrt(ranRayMag / domRayMag );
    Mat44D              N {0};
    double              Sxx = S.rc(0,0),   Sxy = S.rc(0,1),   Sxz = S.rc(0,2),
                        Syx = S.rc(1,0),   Syy = S.rc(1,1),   Syz = S.rc(1,2),
                        Szx = S.rc(2,0),   Szy = S.rc(2,1),   Szz = S.rc(2,2);
    N.rc(0,1)=Syz-Szy;          N.rc(0,2)=Szx-Sxz;      N.rc(0,3)=Sxy-Syx;
                                N.rc(1,2)=Sxy+Syx;      N.rc(1,3)=Szx+Sxz;
                                                        N.rc(2,3)=Syz+Szy;
    N += N.transpose();
    N.rc(0,0) = Sxx+Syy+Szz;
    N.rc(1,1) = Sxx-Syy-Szz;
    N.rc(2,2) = Syy-Sxx-Szz;
    N.rc(3,3) = Szz-Sxx-Syy;
    QuaternionD         pose {cRsmEigs(N).vecs.colVec(3)};
    Vec3D               trans = -scale * (pose.asMatrix() * domMean) + ranMean;
    return {scale,pose,trans};
}

SimilarityD         interpolateAsModelview(SimilarityD s0,SimilarityD s1,double val)
{
    // Scale is also affected by depth so we must interpolate that in log space, and it turns out
    // we need to use that same profile for the other translations to keep linear looking motion:
    double          z0 = s0.trans[2],
                    z1 = s1.trans[2],
                    sign = z0 / abs(z0);
    FGASSERT(z0*z1>0.0);        // Ensure same sign and no zero - can't interpolate behind camera
    double          zl0 = log(abs(s0.trans[2])),
                    zl1 = log(abs(s1.trans[2])),
                    zval = exp(interpolate(zl0,zl1,val)) * sign,
                    lval = (zval-z0) / (z1-z0);
    return SimilarityD {
        exp(interpolate(log(s0.scale),log(s1.scale),val)),
        interpolate(s0.rot,s1.rot,val),
        interpolate(s0.trans,s1.trans,lval)
    };
}

Affine3D            SimilarityRD::asAffine() const
{
    // Since 'Affine' applies translation second: v' = sR(v+t) = sRv + sRt
    Affine3D      ret;
    ret.linear = rot.asMatrix() * scale;
    ret.translation = ret.linear * trans;
    return ret;
}

SimilarityRD        SimilarityRD::inverse() const
{
    // v' = sR(v+t)
    // v' = sRv + sRt
    // sRv = v' - sRt
    // v = s'R'(v' - sRt)
    Vec3D           t = rot * trans * scale;
    return SimilarityRD {-t,rot.inverse(),1.0/scale};
}

SimilarityRD        SimilarityRD::operator*(SimilarityRD const & rhs) const
{
    // Transform:   sR(v+t)
    // Composition: s'R'(sR(v+t)+t')
    //            = s'R'sR(v+t) + s'R't'
    //            = s'R'sR(v + t + s"R"t')
    SimilarityRD        ret;
    ret.scale = scale * rhs.scale;
    ret.rot = rot * rhs.rot;
    ret.trans = rhs.trans + rhs.rot.inverse() * trans / rhs.scale;
    return ret;
}

std::ostream &      operator<<(std::ostream & os,SimilarityRD const & v)
{
    return os
        << "Translation: " << v.trans
        << " Scale: " << v.scale
        << " Rotation: " << v.rot;
}

bool                isApproxEqual(SimilarityD const & l,SimilarityD const & r,double prec)
{
    return (
        isApproxEqual(l.scale,r.scale,prec) &&
        isApproxEqual(l.rot,r.rot,prec) &&
        isApproxEqual(l.trans,r.trans,prec)
    );
}

namespace {

SimilarityD         randSim()
{
    double              scale = exp(randNormal());
    QuaternionD         rot = QuaternionD::rand();
    Vec3D               trans = Vec3D::randNormal();
    return {scale,rot,trans};
}

void                testSim(CLArgs const &)
{
    randSeedRepeatable();
    double constexpr    prec = lims<double>::epsilon() * 16;
    for (size_t ii=0; ii<10; ++ii) {
        SimilarityD         sim = randSim(),
                            inv = sim.inverse(),
                            id = sim * inv;
        FGASSERT(isApproxEqual(id.scale,1.0,prec));
        FGASSERT(isApproxEqual(id.rot,QuaternionD{},prec));
        FGASSERT(isApproxEqual(id.trans,Vec3D{0},prec));
    }
    for (size_t ii=0; ii<10; ++ii) {
        double              scale = exp(randNormal());
        QuaternionD         rot = QuaternionD::rand();
        Vec3D               trans = Vec3D::randNormal();
        SimilarityRD        sim = {trans,rot,scale},
                            inv = sim.inverse(),
                            id = sim * inv;
        FGASSERT(isApproxEqual(id.scale,1.0,prec));
        FGASSERT(isApproxEqual(id.rot,QuaternionD{},prec));
        FGASSERT(isApproxEqual(id.trans,Vec3D{0},prec));
    }
}

void                testSolve(CLArgs const &)
{
    randSeedRepeatable();
    // exact test:
    for (size_t ii=0; ii<5; ++ii) {
        double const        prec = epsBits(40);
        size_t              V = 3;
        for (size_t jj=0; jj<10; ++jj) {
            Vec3Ds              domain = randVecNormals<double,3>(V,1.0);
            SimilarityD         simRef = randSim();
            Vec3Ds              range = mapMul(simRef.asAffine(),domain);
            {   // unweighted:
                SimilarityD         simTst = solveSimilarity(domain,range);
                FGASSERT(isApproxEqual(simTst,simRef,prec));
            }
            if (V > 3) {       // weighted can have low precision with only 3 points:
                Doubles             weights = genSvec<double>(V,[](size_t){return sqr(randNormal()); });
                SimilarityD         simTst = solveSimilarity(domain,range,weights);
                FGASSERT(isApproxEqual(simTst,simRef,prec));
            }
        }
        V *= 2;
    }
    // test w/ noise:
    for (size_t ii=0; ii<5; ++ii) {
        double const        prec = epsBits(20);
        size_t              V = 2ULL << 16;
        SimilarityD         simRef = randSim();
        Vec3Ds              domain = randVecNormals<double,3>(V,1.0),
                            noise = randVecNormals<double,3>(V,0.01),
                            range = mapMul(simRef,domain);
        {   // unweighted:
            SimilarityD         simTst = solveSimilarity(domain,range);
            FGASSERT(isApproxEqual(simTst,simRef,prec));
        }
        {   // weighted:
            Doubles             weights = genSvec<double>(V,[](size_t){return sqr(randNormal()); });
            SimilarityD         simTst = solveSimilarity(domain,range,weights);
            FGASSERT(isApproxEqual(simTst,simRef,prec));
        }
    }
}

}

void                testSimilarity(CLArgs const & args)
{
    Cmds            cmds {
        {testSim,"sim","similarity composition and inverse"},
        {testSolve,"solve","similarity solver"},
    };
    doMenu(args,cmds,true);
}

}

// */
