//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     July 14, 2009
//

#include "stdafx.h"

#include "FgSimilarity.hpp"
#include "FgMatrixSolver.hpp"
#include "FgBounds.hpp"
#include "FgApproxEqual.hpp"
#include "FgMain.hpp"

using namespace std;

// Transform: v' = sRv+t
// Inverse:   v  = s^R^v' + (-s^R^t)
FgSimilarity
FgSimilarity::inverse() const
{
    double          scale = 1.0 / m_scale;
    FgQuaternionD   rot = m_rot.inverse();
    FgVect3D        trans = -scale * (rot * m_trans);
    return FgSimilarity(scale,rot,trans);
}

// Transform:   sRv+t
// Composition: s'R'(sRv+t)+t'
//            = s'R'sR(v) + (s'R't+t')
FgSimilarity
FgSimilarity::operator*(const FgSimilarity & rhs)
const
{
    double          scale = m_scale * rhs.m_scale;
    FgQuaternionD   rot = m_rot * rhs.m_rot;
    FgVect3D        trans = m_scale * (m_rot * rhs.m_trans) + m_trans;
    return FgSimilarity(scale,rot,trans);
}

std::ostream &
operator<<(std::ostream & os,const FgSimilarity & v)
{
    return
        os << "Scale: " << v.m_scale << fgnl
            << "Rotation: " << v.m_rot << fgnl
            << "Translation: " << v.m_trans;
}

FgSimilarity
fgSimilarityRand()
{
    return
        FgSimilarity(
            std::exp(fgRandNormal()),
            fgQuaternionRand(),
            fgVecRandNrm<3>());
}

// Uses approach originally from [Horn '87 "Closed-Form Solution of Absolute Orientation..."
// (taken from [Jain '95 "machine vision" 12.3]) to find an approximate similarity transform
// between two sets of corresponding points, FROM the domain points TO the range points:
FgSimilarity
fgSimilarityApprox(
    const vector<FgVect3D> &    domainPts,
    const vector<FgVect3D> &    rangePts)
{
    FgSimilarity            ret;
    FGASSERT(domainPts.size() > 2);     // Not solvable with only 2 points.
    uint                    numPts = uint(domainPts.size());
    FGASSERT(numPts == rangePts.size());
    // Compute the sufficient statistics for scale & rotation
    FgVect3D        domMean = fgMean(domainPts),
                    ranMean = fgMean(rangePts);
    double          domRayMag = 0.0,
                    ranRayMag = 0.0;
    FgMat33D     S(0.0);
    for (uint ii=0; ii<numPts; ii++) {
        FgVect3D    domRay = domainPts[ii] - domMean,
                    ranRay = rangePts[ii] - ranMean;
        domRayMag += domRay.mag();
        ranRayMag += ranRay.mag();
        S += domRay * ranRay.transpose();
    }
    double          scale = sqrt(ranRayMag / domRayMag );
    FgMat44D        N(0.0);
    double  Sxx = S.cr(0,0),   Sxy = S.cr(1,0),   Sxz = S.cr(2,0),
            Syx = S.cr(0,1),   Syy = S.cr(1,1),   Syz = S.cr(2,1),
            Szx = S.cr(0,2),   Szy = S.cr(1,2),   Szz = S.cr(2,2);
    // Set the upper triangular elements of N not including the diagonal:
    N.cr(1,0)=Syz-Szy;         N.cr(2,0)=Szx-Sxz;      N.cr(3,0)=Sxy-Syx;
                                N.cr(2,1)=Sxy+Syx;      N.cr(3,1)=Szx+Sxz;
                                                         N.cr(3,2)=Syz+Szy;
    // Since it's symmetric, set the lower triangular (not including diagonal) by:
    N += N.transpose();
    // And set the diagonal elements:
    N.cr(0,0) = Sxx+Syy+Szz;
    N.cr(1,1) = Sxx-Syy-Szz;
    N.cr(2,2) = Syy-Sxx-Szz;
    N.cr(3,3) = Szz-Sxx-Syy;
    // Calculate rotation from N per [Jain '95]. 'fgEigsRsm' Leaves largest eigVal in last index:
    FgQuaternionD       pose(fgEigsRsm(N).vecs.colVec(3));
    // Calculate the 'trans' term: The transform is given by:
    // X = SR(d-dm)+rm = SR(d)-SR(dm)+rm
    FgVect3D            trans = -scale * (pose.asMatrix() * domMean) + ranMean;
    ret = FgSimilarity(scale,pose,trans);
    // Measure residual:
    double  resid = fgRms(rangePts-fgTransform(domainPts,ret.asAffine())) / fgMaxElem(fgDims(rangePts));
    fgout << fgnl << "SimilarityApprox() relative RMS residual: " << resid;
    return ret;
}

void
fgSimilarityTest(const FgArgs &)
{
    FgSimilarity    sim(fgExp(fgRandNormal()),FgQuaternionD(fgVecRandNrm<4>()),fgVecRandNrm<3>());
    FgSimilarity    id = sim * sim.inverse();
    FgMat33D        diff = id.xformCoord(FgMat33D::identity()) - FgMat33D::identity();
    FGASSERT(fgApproxEqual(1.0+diff.length(),1.0,100));
}

void
fgSimilarityApproxTest(const FgArgs &)
{
    fgRandSeedRepeatable();
    for (uint nn=0; nn<10; nn++) {
        uint                numPts = 3;
        vector<FgVect3D>    domain(numPts),
                            range(numPts);
        for (uint ii=0; ii<numPts; ii++)
            domain[ii] = fgVecRandNrm<3>();
        for (uint mm=0; mm<10; mm++) {
            FgSimilarity    simRef(fgExp(3.0 * fgRandNormal()),fgQuaternionRand(),fgVecRandNrm<3>());
            fgTransform_(domain,range,simRef.asAffine());
            FgSimilarity    sim = fgSimilarityApprox(domain,range);
            fgTransform_(domain,sim.asAffine());
            FGASSERT(fgApproxEqual(domain,range));
        }
        numPts *= 2;
    }
}

// */
