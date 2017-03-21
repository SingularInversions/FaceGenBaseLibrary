//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     July 14, 2009
//
// Similarity transform: v' = sRv + t
//
// USE: If applying to many vectors, more efficient to transform to 'asAffine()' first.
//

#ifndef FGSIMILARITY_HPP
#define FGSIMILARITY_HPP

#include "FgQuaternion.hpp"
#include "FgAffineC.hpp"
#include "FgStdStream.hpp"

struct  FgSimilarity
{
    double          m_scale;        // Scale and rotation applied first
    FgQuaternionD   m_rot;
    FgVect3D        m_trans;        // Translation applied last

    FG_SERIALIZE3(m_scale,m_rot,m_trans);

    FgSimilarity()
    : m_scale(1.0)
    {}

    explicit
    FgSimilarity(const double & s)
    : m_scale(s)
    {}

    explicit
    FgSimilarity(const FgVect3D & t)
    : m_scale(1.0), m_trans(t)
    {}

    explicit
    FgSimilarity(const FgQuaternionD & r)
    : m_scale(1.0), m_rot(r)
    {}

    FgSimilarity(
        double                  scale,
        const FgQuaternionD &   rot,
        const FgVect3D &        trans)
    : m_scale(scale), m_rot(rot), m_trans(trans)
    {FGASSERT(m_scale > 0.0); }

    // We use xformCoord() instead of operator*() because similarity transforms work differently
    // for coordinate, covariant and contravariant vectors:
    template<uint ncols>
    FgMatrixC<double,3,ncols>
    xformCoord(const FgMatrixC<double,3,ncols> & coords);   // Columns are coordinate vectors.

    // More efficient if applying the transform to many vectors:
    FgAffine3D
    asAffine() const
    {return FgAffine3D(m_rot.asMatrix() * m_scale,m_trans); }

    // operator* in this context means composition:
    FgSimilarity
    operator*(const FgSimilarity & rhs) const;

    FgSimilarity
    inverse() const;
};

template<uint ncols>
FgMatrixC<double,3,ncols>
FgSimilarity::xformCoord(
    const FgMatrixC<double,3,ncols> & coords)
{
    FgMatrixC<double,3,ncols>   tmp = (m_rot * coords) * m_scale;
    for (uint rr=0; rr<3; rr++)
        for (uint cc=0; cc<ncols; cc++)
            tmp.rc(rr,cc) += m_trans[rr];
    return tmp;
}

std::ostream &
operator<<(std::ostream & os,const FgSimilarity & v);

FgSimilarity
fgSimilarityRand();

FgSimilarity
fgSimilarityApprox(
    const std::vector<FgVect3D> &    domainPts,
    const std::vector<FgVect3D> &    rangePts);

inline
FgSimilarity
fgSimilarityApprox(const FgVerts & d,const FgVerts & r)
{return fgSimilarityApprox(fgToDouble(d),fgToDouble(r)); }

#endif
