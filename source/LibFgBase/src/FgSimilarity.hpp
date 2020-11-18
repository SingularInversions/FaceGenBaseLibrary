//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Geometric similarity transform: v' = sRv + t
// (shape preserving: scale, rotation, translation)
//
// USE: If applying repeatedly, more efficient to transform to 'asAffine()' first.
//

#ifndef FGSIMILARITY_HPP
#define FGSIMILARITY_HPP

#include "FgQuaternion.hpp"
#include "FgAffineC.hpp"
#include "FgStdStream.hpp"

namespace Fg {

template<typename T>
struct  Similarity
{
    T               scale = 1;      // Scale and rotation applied first
    Quaternion<T>   rot;
    Mat<T,3,1>      trans;          // Translation applied last

    FG_SERIALIZE3(scale,rot,trans);

    Similarity() {}

    explicit
    Similarity(T s) : scale(s) {}

    explicit
    Similarity(Mat<T,3,1> const & t) : trans(t) {}

    explicit
    Similarity(Quaternion<T> const & r) : rot(r) {}

    Similarity(T s,Quaternion<T> const & r,const Mat<T,3,1> & t)
    : scale(s), rot(r), trans(t)
    {FGASSERT(scale > 0); }

    Mat<T,3,1>
    operator*(Mat<T,3,1> vec) const
    {return rot*vec*scale + trans; }

    // More efficient if applying the transform to many vectors:
    Affine<T,3>
    asAffine() const
    {return Affine<T,3>(rot.asMatrix() * scale,trans); }

    // operator* in this context means composition:
    Similarity operator*(const Similarity & rhs) const
    {
        // Transform:   sRv+t
        // Composition: s'R'(sRv+t)+t'
        //            = s'R'sR(v) + (s'R't+t')
        Similarity    ret;
        ret.scale = scale * rhs.scale;
        ret.rot = rot * rhs.rot;
        ret.trans = scale * (rot * rhs.trans) + trans;
        return ret;
    }

    Similarity inverse() const
    {
        // v' = sRv+t
        // sRv = v' - t
        // v = s'R'v' - s'R't
        double              s = 1.0 / scale;
        QuaternionD         r = rot.inverse();
        Vec3D               t = r * trans * s;
        return Similarity {s,r,-t};
    }

    // Be more explicit than using default constructor:
    static Similarity identity() {return Similarity(1,Quaternion<T>(),Mat<T,3,1>(0)); }
};

typedef Similarity<float>   SimilarityF;
typedef Similarity<double>  SimilarityD;

template<typename T>
Mat<T,4,4>
asHomogMat(Similarity<T> const & s)
{return asHomogMat(s.asAffine()); }

SimilarityD
similarityRand();

SimilarityD
similarityApprox(Vec3Ds const & domainPts,Vec3Ds const & rangePts);

inline
SimilarityD
similarityApprox(Vec3Fs const & d,Vec3Fs const & r)
{return similarityApprox(scast<double>(d),scast<double>(r)); }

SimilarityD
interpolateAsModelview(SimilarityD s0,SimilarityD s1,double val);  // val [0,1]

// Reverse-order similarity transform: v' = sR(v + t) = sRv + sRt
// Useful when you want the translation relative to the input shape (v) not the output (v').
// Also fewer operations and doesn't lose precision on 'trans' during 'inverse' operation.
struct  SimilarityRD
{
    Vec3D           trans;          // Translation applied first
    QuaternionD     rot;
    double          scale;          // Scale and rotation applied last

    FG_SERIALIZE3(trans,rot,scale);

    SimilarityRD() {}

    SimilarityRD(Vec3D const & t,QuaternionD const & r,double s)
    : trans(t), rot(r), scale(s)
    {FGASSERT(s > 0.0); }

    // SimilarityD: v' = sRv + t = sR(v + s^-1 R^-1 t)
    SimilarityRD(const SimilarityD & s)
        : trans(s.rot.inverse() * s.trans / s.scale), rot(s.rot), scale(s.scale)
    {}

    // More efficient if applying the transform to many vectors:
    Affine3D asAffine() const;

    // v' = sR(v+t)
    // v' = sRv + sRt
    // sRv = v' - sRt
    // v = s'R'(v' - sRt)
    SimilarityRD
    inverse() const
    {
        Vec3D           t = rot * trans * scale;
        return SimilarityRD {-t,rot.inverse(),1.0/scale};
    }

    // Return inverse of only the linear component (no translation):
    Mat33D
    linearInverse() const
    {return rot.inverse().asMatrix() / scale; }

    static SimilarityRD
    identity()
    {return SimilarityRD {Vec3D(0),QuaternionD{},1}; }
};

typedef Svec<SimilarityRD>   SimilarityRDs;

std::ostream &
operator<<(std::ostream & os,SimilarityRD const & v);

}

#endif
