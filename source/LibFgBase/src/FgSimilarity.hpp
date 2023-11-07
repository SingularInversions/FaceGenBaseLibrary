//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Geometric similarity transform: v' = sRv + t
// (shape preserving: scale, rotation, translation)
// Geometric rigid tranform
//
// USE: If applying repeatedly, more efficient to transform to 'asAffine()' first.
//

#ifndef FGSIMILARITY_HPP
#define FGSIMILARITY_HPP

#include "FgQuaternion.hpp"
#include "FgAffine.hpp"

namespace Fg {

struct  Rigid3D
{
    QuaternionD         rot;            // Applied first
    Vec3D               trans {0};      // Applied last
    FG_SER2(rot,trans)

    Rigid3D() {}
    explicit Rigid3D(QuaternionD const & r) : rot{r} {}
    explicit Rigid3D(Vec3D const & t) : trans{t} {}
    Rigid3D(QuaternionD const & r,Vec3D const & t) : rot{r}, trans{t} {}
    // construct from translation followed by rotation: y = R(x+t) = Rx + Rt
    Rigid3D(Vec3D const & t,QuaternionD const & r) : rot{r}, trans{r*t} {}

    Affine3D        asAffine() const {return Affine3D {rot.asMatrix(),trans}; }
    Rigid3D         inverse() const {return Rigid3D {-trans,rot.inverse()}; }
    // composition operator:
    Rigid3D         operator*(Rigid3D const & r) const {return {rot*r.rot,trans+rot*r.trans}; }

    static Rigid3D  randNormal(double transStdev)
    {return Rigid3D{QuaternionD::rand(),Vec3D::randNormal(transStdev)}; }
};
typedef Svec<Rigid3D>   Rigid3Ds;
std::ostream &      operator<<(std::ostream &,Rigid3D const &);

// rotate around given point:
inline Rigid3D      cRotateAround(Vec3D const & pnt,QuaternionD const & rot)
{
    // y = R(x-p)+p = Rx + (p-Rp)
    return Rigid3D{rot,pnt-rot*pnt};
}
// composition operator:
inline Rigid3D      operator*(QuaternionD const & lhs,Rigid3D const & rhs) {return {lhs*rhs.rot,lhs*rhs.trans}; }

struct  SimilarityRD;

struct  SimilarityD
{
    double              scale {1};      // Scale and rotation applied first
    QuaternionD         rot;
    Vec3D               trans {0};      // Translation applied last
    FG_SER3(scale,rot,trans)

    SimilarityD() {}
    explicit SimilarityD(double s) : scale(s) {}
    explicit SimilarityD(Vec3D const & t) : trans(t) {}
    explicit SimilarityD(QuaternionD const & r) : rot(r) {}
    SimilarityD(ScaleTrans3D const & s) : scale(s.scale), trans(s.trans) {}
    SimilarityD(double s,QuaternionD const & r,const Vec3D & t) : scale(s), rot(r), trans(t) {FGASSERT(scale > 0.0); }
    SimilarityD(Rigid3D const & r) : rot{r.rot}, trans{r.trans} {}
    SimilarityD(SimilarityRD const &);

    Vec3D           operator*(Vec3D const & v) const {return rot * v * scale + trans; }
    Vec3F           operator*(Vec3F const & v) const {return Vec3F(operator*(Vec3D(v))); }
    // More efficient if applying the transform to many vectors:
    Affine3D        asAffine() const {return Affine3D {rot.asMatrix() * scale,trans}; }
    Mat33D          linearComponent() const {return rot.asMatrix() * scale; }
    Rigid3D         rigidComponent() const {return {rot,trans}; }
    // operator* in this context means composition:
    SimilarityD     operator*(SimilarityD const & rhs) const;
    SimilarityD     operator*(QuaternionD const & rhs) const {return {scale,rot*rhs,trans}; }
    SimilarityD     inverse() const;

    // Be more explicit than using default constructor:
    static SimilarityD identity() {return SimilarityD(1.0,QuaternionD{},Vec3D{0}); }
};

inline Mat44D       asHomogMat(SimilarityD const & s) {return asHomogMat(s.asAffine()); }
SimilarityD         similarityRand();
// Uses Horn '87 "Closed-Form Solution of Absolute Orientation..." to quickly find an accurate approximation
// of the least squares similarity transform for a 1-1 mapping from the domain to the range points.
// at least 3 non-degenerate non-colinear points must be provided:
SimilarityD         solveSimilarity(Vec3Ds const & domain,Vec3Ds const & range);
SimilarityD         solveSimilarity(Vec3Fs const & domain,Vec3Fs const & range);
// as above but for count-weighted samples (eg. by inverse variance):
SimilarityD         solveSimilarity(Vec3Ds const & domain,Vec3Ds const & range,Doubles weights);
SimilarityD         interpolateAsModelview(SimilarityD s0,SimilarityD s1,double val);  // val [0,1]

// Reverse-order similarity transform: v' = sR(v + t) = sRv + sRt
// Useful when you want the translation relative to the input shape (v) not the output (v').
// Also fewer operations and doesn't lose precision on 'trans' during 'inverse' operation.
struct  SimilarityRD
{
    Vec3D           trans {0};          // Translation applied first
    QuaternionD     rot;
    double          scale {1};          // Scale and rotation applied last
    FG_SER3(trans,rot,scale)

    SimilarityRD() {}
    SimilarityRD(Vec3D const & t,QuaternionD const & r,double s) : trans {t}, rot {r}, scale {s} {FGASSERT(s > 0.0); }
    // SimilarityD: v' = sRv + t = sR(v + s^-1 R^-1 t)
    SimilarityRD(SimilarityD const & s) : trans {s.rot.inverse()*s.trans/s.scale}, rot{s.rot}, scale {s.scale} {}

    // More efficient if applying the transform to many vectors:
    Affine3D                asAffine() const;
    SimilarityRD            inverse() const;
    Mat33D                  linearComponent() const {return rot.asMatrix() * scale; }
    // SimilarityR: v' = sR(v + t) = sRv + sRt
    SimilarityD             asSimilarityD() const {return SimilarityD{scale,rot,scale*(rot*trans)}; }

    static SimilarityRD     identity() {return SimilarityRD {Vec3D(0),QuaternionD{},1}; }
    static size_t constexpr dof() {return 7; }
};
typedef Svec<SimilarityRD>   SimilarityRDs;

std::ostream &      operator<<(std::ostream & os,SimilarityRD const & v);

inline Vec3Ds       transform(SimilarityD const & sim,Vec3Ds const & poss) {return mapMul(sim.asAffine(),poss); }

}

#endif
