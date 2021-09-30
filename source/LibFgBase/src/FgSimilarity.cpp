//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgSimilarity.hpp"
#include "FgMatrixSolver.hpp"
#include "FgBounds.hpp"
#include "FgApproxEqual.hpp"
#include "FgMain.hpp"

using namespace std;

namespace Fg {

ostream &
operator<<(ostream & os,Rigid3D const & r)
{
    return os << "rot: " << r.rot << " trans: " << r.trans;
}

SimilarityD
SimilarityD::operator*(SimilarityD const & rhs) const
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

SimilarityD
SimilarityD::inverse() const
{
    // v' = sRv+t
    // sRv = v' - t
    // v = s'R'v' - s'R't
    double              s = 1.0 / scale;
    QuaternionD         r = rot.inverse();
    Vec3D               t = r * trans * s;
    return SimilarityD {s,r,-t};
}

SimilarityD
similarityRand()
{
    return
        SimilarityD(
            std::exp(randNormal()),
            QuaternionD::rand(),
            Vec3D::randNormal());
}

SimilarityD
solveSimilarity(Vec3Ds const & domainPts,Vec3Ds const & rangePts)
{
    FGASSERT(domainPts.size() > 2);     // Not solvable with only 2 points.
    uint            numPts = uint(domainPts.size());
    FGASSERT(numPts == rangePts.size());
    // Compute the sufficient statistics for scale & rotation
    Vec3D           domMean = cMean(domainPts),
                    ranMean = cMean(rangePts);
    double          domRayMag = 0.0,
                    ranRayMag = 0.0;
    Mat33D     S(0.0);
    for (uint ii=0; ii<numPts; ii++) {
        Vec3D       domRay = domainPts[ii] - domMean,
                    ranRay = rangePts[ii] - ranMean;
        domRayMag += domRay.mag();
        ranRayMag += ranRay.mag();
        S += domRay * ranRay.transpose();
    }
    double          scale = sqrt(ranRayMag / domRayMag );
    Mat44D          N(0.0);
    double  Sxx = S.cr(0,0),   Sxy = S.cr(1,0),   Sxz = S.cr(2,0),
            Syx = S.cr(0,1),   Syy = S.cr(1,1),   Syz = S.cr(2,1),
            Szx = S.cr(0,2),   Szy = S.cr(1,2),   Szz = S.cr(2,2);
    // Set the upper triangular elements of N not including the diagonal:
    N.cr(1,0)=Syz-Szy;          N.cr(2,0)=Szx-Sxz;      N.cr(3,0)=Sxy-Syx;
                                N.cr(2,1)=Sxy+Syx;      N.cr(3,1)=Szx+Sxz;
                                                        N.cr(3,2)=Syz+Szy;
    // Since it's symmetric, set the lower triangular (not including diagonal) by:
    N += N.transpose();
    // And set the diagonal elements:
    N.cr(0,0) = Sxx+Syy+Szz;
    N.cr(1,1) = Sxx-Syy-Szz;
    N.cr(2,2) = Syy-Sxx-Szz;
    N.cr(3,3) = Szz-Sxx-Syy;
    // Calculate rotation from N per [Jain '95]. 'cEigsRsm' Leaves largest eigVal in last index:
    QuaternionD         pose(cEigsRsm(N).vecs.colVec(3));
    // Calculate the 'trans' term: The transform is given by:
    // X = SR(d-dm)+rm = SR(d)-SR(dm)+rm
    Vec3D               trans = -scale * (pose.asMatrix() * domMean) + ranMean;
    SimilarityD         ret {scale,pose,trans};
    // Measure residual:
    //double  resid = cRms(rangePts-mapMul(ret.asAffine(),domainPts)) / cMaxElem(cDims(rangePts));
    //fgout << fgnl << "SimilarityApprox() relative RMS residual: " << resid;
    return ret;
}

SimilarityD
interpolateAsModelview(SimilarityD s0,SimilarityD s1,double val)
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

Affine3D
SimilarityRD::asAffine() const
{
    // Since 'Affine' applies translation second: v' = sR(v+t) = sRv + sRt
    Affine3D      ret;
    ret.linear = rot.asMatrix() * scale;
    ret.translation = ret.linear * trans;
    return ret;
}

SimilarityRD
SimilarityRD::inverse() const
{
    // v' = sR(v+t)
    // v' = sRv + sRt
    // sRv = v' - sRt
    // v = s'R'(v' - sRt)
    Vec3D           t = rot * trans * scale;
    return SimilarityRD {-t,rot.inverse(),1.0/scale};
}

std::ostream &
operator<<(std::ostream & os,SimilarityRD const & v)
{
    return os
        << "Translation: " << v.trans
        << " Scale: " << v.scale
        << " Rotation: " << v.rot;
}

void
testSimilarity(CLArgs const &)
{
    SimilarityD     sim(expSafe(randNormal()),QuaternionD(Vec4D::randNormal()),Vec3D::randNormal());
    SimilarityD     id = sim * sim.inverse();
    Mat33D          diff = id.asAffine() * Mat33D::identity() - Mat33D::identity();
    FGASSERT(cMax(mapAbs(diff.m)) < epsilonD()*8);
}

void
testSimilaritySolve(CLArgs const &)
{
    randSeedRepeatable();
    for (uint nn=0; nn<10; nn++) {
        uint            numPts = 3;
        Vec3Ds          domain(numPts),
                        range(numPts);
        for (uint ii=0; ii<numPts; ii++)
            domain[ii] = Vec3D::randNormal();
        for (uint mm=0; mm<10; mm++) {
            SimilarityD     simRef(expSafe(3.0 * randNormal()),QuaternionD::rand(),Vec3D::randNormal());
            mapMul_(simRef.asAffine(),domain,range);
            SimilarityD     sim = solveSimilarity(domain,range);
            mapMul_(sim.asAffine(),domain);
            FGASSERT(isApproxEqualRelMag(domain,range));
        }
        numPts *= 2;
    }
}

}

// */
