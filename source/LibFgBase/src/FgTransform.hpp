//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Transformation operators
//
// operator*(transform,vector) is overloaded to application of the transform; v' = T(v)
// operator*(transform,transform) is overloaded to composition of transforms from right to left
// Note that composition of non-linear transformations is not commutative.
//

#ifndef FGTRANSFORM_HPP
#define FGTRANSFORM_HPP

#include "FgApproxEqual.hpp"

namespace Fg {

// axial scale operator. Separate scale for each axis. Equivalent to diagonal matrix.
// aka anisotropic or nonuniform scaling.
template<class T,size_t D>
struct      AxScale
{
    Arr<T,D>            scales;
    explicit AxScale(Arr<T,D> const & s) : scales{s} {}
};
typedef AxScale<double,3>   AxScale3D;

// translation operator is trivial but useful in specifying behaviour in operator application and composition
template<class T,size_t D>
struct      Trans
{
    Mat<T,D,1>          delta;
    explicit Trans(Mat<T,D,1> const & d) : delta{d} {}
    explicit Trans(T v) : delta{v} {}   // fill ctor
    Trans(T x,T y) : delta{x,y} {}
    Trans(T x,T y,T z) : delta{x,y,z} {}

    Mat<T,D,1>          operator*(Mat<T,D,1> v) const {return v + delta; }
};

typedef Trans<float,2>      Trans2F;
typedef Trans<double,2>     Trans2D;
typedef Trans<float,3>      Trans3F;
typedef Trans<double,3>     Trans3D;

template<class T,class U,size_t D>
Trans<T,D>          mapCast(Trans<U,D> t) {return Trans<T,D>{mapCast<T>(t.delta)}; }

template<class T,size_t D>
PosSize<T,D>        operator*(Trans<T,D> t,PosSize<T,D> ps) {return {t*ps.loPos,ps.size}; }

template<typename T,size_t D>
struct      ScaleTrans
{
    T               scale {1};          // applied first
    Mat<T,D,1>      trans {0};          // applied second

    ScaleTrans() {}
    explicit ScaleTrans(T s) : scale(s) {}
    explicit ScaleTrans(Mat<T,D,1> t) : trans(t) {}
    ScaleTrans(T s,Mat<T,D,1> t) : scale(s), trans(t) {}
    // construct as if given translation was done first, then scale applied
    // y = S(x + t) = Sx + St
    ScaleTrans(Mat<T,D,1> t,T s) : scale{s}, trans{t*s} {}

    // explicitly enable default constructor to avoid disabling by conversion constructor:
    ScaleTrans(ScaleTrans const &) = default;
    ScaleTrans &        operator=(ScaleTrans const &) = default;

    // Conversion constructor
    template<typename U>
    explicit ScaleTrans(ScaleTrans<U,D> const & v) : scale(v.scale), trans(v.trans) {}

    // Operator application:
    Mat<T,D,1>          operator*(Mat<T,D,1> rhs) const {return rhs * scale + trans; }
    PosSize<T,D>        operator*(PosSize<T,D> rhs) const {return {rhs.loPos*scale+trans,rhs.size*scale}; }

    // Operator composition:
    ScaleTrans          operator*(Trans<T,D> tr) const
    {
        // RHS: y = x + u
        // LHS: z = Sy + t = S(x+u) + t = Sx + (Su + t)
        return ScaleTrans{scale,scale*tr.delta + trans};
    }
    ScaleTrans          operator*(ScaleTrans rhs) const
    {
        // RHS: y = Sx+t
        // LHS: z = S'y+t' = S'(Sx+t)+t' = S'Sx + (S't+t')
        return ScaleTrans{scale*rhs.scale,scale*rhs.trans+trans};
    }

    ScaleTrans          inverse() const
    {
        // y = Sx + t, x = (y-t)/S = y/S - t/S
        FGASSERT(scale != 0);
        T           invScale = 1 / scale;
        return ScaleTrans{invScale,-invScale*trans};
    }
};
typedef ScaleTrans<float,2>     ScaleTrans2F;
typedef ScaleTrans<double,2>    ScaleTrans2D;
typedef ScaleTrans<float,3>     ScaleTrans3F;
typedef ScaleTrans<double,3>    ScaleTrans3D;
typedef Svec<ScaleTrans3D>      ScaleTrans3Ds;

template<class T,size_t D>
ScaleTrans<T,D>     operator*(Trans<T,D> t,ScaleTrans<T,D> st) {return {st.scale,t.delta+st.trans}; }

template<class T,size_t D>
ScaleTrans<T,D>     cScaleAroundPoint(T scale,Mat<T,D,1> const & pnt)
{
    // (x-pnt)*scale + pnt = x*scale - pnt*scale + pnt = x*scale + (pnt-pnt*scale)
    return {scale,pnt-pnt*scale};
}

// least squares scale and translation relative to means, for 1-1 corresponding points:
template<typename T,size_t D>
ScaleTrans<T,D>     solveScaleTrans(Svec<Mat<T,D,1>> const & src,Svec<Mat<T,D,1>> const & dst)
{
    size_t              S = src.size();
    FGASSERT(S > 1);
    FGASSERT(dst.size() == S);
    Mat<T,D,1>          meanS = cMean(src),
                        meanD = cMean(dst),
                        trans = meanD-meanS;
    Svec<Mat<T,D,1>>    srcMC = mapSub(src,meanS),
                        dstMC = mapSub(dst,meanD);
    T                   scale = cDot(srcMC,dstMC) / cMagD(srcMC);
    return ScaleTrans<T,D>{meanS} * ScaleTrans<T,D>{scale,trans} * ScaleTrans<T,D>{-meanS};
}

// 1D version with scalar members; scale (can be negative) and translate transform: x' = sx + t
template <class T>
struct      Affine1
{
    T               m_scale;            // applied first. Cannot be zero.
    T               m_trans;            // applied second
    FG_SER(m_scale,m_trans)

    Affine1() : m_scale{1}, m_trans{0} {}
    Affine1(T scale,T trans) : m_scale{scale}, m_trans{trans} {FGASSERT(m_scale != 0); }
    // Construct from a domain and its map; conceptually easier and less likely to make a mistake,
    // but somewhat redundant; you could use any non-zero domain within the intented usage.
    // Note that the map must be the mapped values of the chosen domain which is not necessarily a
    // range, for instance if the transform reverses the values:
    // s = r.s / d.s
    // s * r.l + t = d.l -> t = d.l - s * r.l
    Affine1(ValRange<T> domain,ValRange<T> dmap) :
        m_scale {dmap.size / domain.size},
        m_trans {dmap.loPos - domain.loPos*m_scale}
    {FGASSERT(domain.size * dmap.size != 0); }
    Affine1(T domainLo,T domainHi,T dmapLo,T dmapHi)
        : Affine1{ValRange<T>{domainLo,domainHi-domainLo},ValRange<T>{dmapLo,dmapHi-dmapLo}}
    {}
    // Don't let conversion constructor override defaults:
    Affine1(Affine1 const &) = default;
    Affine1 &       operator=(Affine1 const &) = default;
    // conversion constructor:
    template<class U>
    Affine1(Affine1<U> aff) : m_scale{T(aff.m_scale)}, m_trans{T(aff.m_trans)} {}

    T               operator*(T domainVal) const {return (m_scale * domainVal + m_trans); }
    // multiplication of Affine1's is composition. Note that it does not commute:
    // y = Sx + t
    // z = S'y + t' = S'(Sx+t) + t' = (S'S)x + (S't + t')
    // rhs below is 'y' above:
    Affine1         operator*(Affine1 r) const
    {
        return {m_scale*r.m_scale, m_scale*r.m_trans + m_trans};
    }
    // inverse: x = (x'-t)/s = (1/s)x' + (-t/s)
    Affine1         inverse() const {return Affine1(T(1)/m_scale,-m_trans/m_scale); }
    T               invert(T rangeVal) const {return ((rangeVal - m_trans) / m_scale); }
};
typedef Affine1<float>       Affine1F;
typedef Affine1<double>      Affine1D;

template<class T>
std::ostream &      operator<<(std::ostream & os,const Affine1<T> & v)
{
    os  << fgnl << "Scale: " << v.m_scale << " Translation: " << v.m_trans;
    return os;
}

template<class T>
Affine1<T>          interpolate(Affine1<T> lhs,Affine1<T> rhs,T val)    // val 0: lhs, 1: rhs
{
    return {
        std::exp(interpolate(std::log(lhs.m_scale),std::log(rhs.m_scale),val)),
        interpolate(lhs.m_trans,rhs.m_trans,val)
    };
}

template<class T>
bool                isApproxEqual(Affine1<T> const & l,Affine1<T> const & r,T maxDiff)
{
    return
        isApproxEqual(l.m_scale,r.m_scale,maxDiff) &&
        isApproxEqual(l.m_trans,r.m_trans,maxDiff);
}

// Axial (element-wise) affine (aka Bounding Box, Rectilinear, Axially separable affine, diagonal affine) transform:
// per-axis scaling and translation: x'_i = s * x_i + t
template <class T,size_t D>
struct      AxAffine
{
    Mat<T,D,1>          scales;         // applied first
    Mat<T,D,1>          trans;          // applied second
    FG_SER(scales,trans)

    AxAffine() : scales{1}, trans{0} {}
    explicit AxAffine(ScaleTrans<T,D> const & st) : scales{st.scale}, trans{st.trans} {}
    AxAffine(Arr<T,D> const & s,Arr<T,D> const & t)             // scale then translate
        : scales{s}, trans{t} {}
    AxAffine(Mat<T,D,1> const & s,Mat<T,D,1> const & t)         // scale then translate
        : scales{s}, trans{t} {}
    AxAffine(Trans<T,D> const & t,AxScale<T,D> const & s) :       // translate then scale
        // y = s(x+t) = sx + st
        scales{s.scales},
        trans{mapMul(s.scales,t.delta.m)}
    {}
    AxAffine(Rect<T,D> domain,Rect<T,D> range) :
        scales{mapDiv(range.dims,domain.dims)}, trans{0}
    {
        // Construct from a domain and its map (range but inverted for negative scale);
        // conceptually easier and less likely to make a mistake,
        // but somewhat redundant; you could use any non-zero domain within the intented usage.
        // Note that the map must be the mapped values of the chosen domain which is not necessarily a
        // range, for instance if the transform reverses the values:
        // s = r.s / d.s
        // s * d.l + t = r.l -> t = r.l - s * d.l
        trans = range.loPos - mapMul(domain.loPos,scales);
    }
    AxAffine(Mat<T,D,2> domain,Mat<T,D,2> range)        // only works for non-negative scales
        : AxAffine(Rect<T,D>{domain},Rect<T,D>{range}) {}
    // Don't let conversion constructor override defaults:
    AxAffine(AxAffine const &) = default;
    AxAffine &       operator=(AxAffine const &) = default;
    // Conversion constructor:
    template<class U>
    AxAffine(AxAffine<U,D> const & r) : scales(r.scales), trans(r.trans) {}

    // Application; matrices are treated as collated column vectors:
    template<size_t V>
    Mat<T,D,V>  operator*(Mat<T,D,V> const & vec) const
    {
        Mat<T,D,V>        ret;
        for (size_t rr=0; rr<D; ++rr)
            for (size_t cc=0; cc<V; ++cc)
                ret.rc(rr,cc) = scales[rr] * vec.rc(rr,cc) + trans[rr];
        return ret;
    }

    // operator composition:
    AxAffine<T,D>       operator*(Trans<T,D> r) const
    {
        // y = x + t
        // z = Sy + u = S(x+t) + u = Sx + (St+u)
        return {scales,mapMul(scales,r.delta)+trans};
    }
    AxAffine<T,D>       operator*(AxAffine<T,D> rhs) const
    {
        // y = Sx + t
        // z = Ty + u = T(Sx+t) + u = TSx + (Tt+u)
        return {mapMul(scales,rhs.scales),mapMul(scales,rhs.trans)+trans};
    }

    AxAffine<T,D>       inverse() const
    {
        // y = Sx + t
        // x = (y-t)/S = y/S - t/S
        return {mapDiv(T(1),scales),-mapDiv(trans,scales)};
    }
};

typedef AxAffine<float,2>       AxAffine2F;
typedef AxAffine<double,2>      AxAffine2D;
typedef AxAffine<float,3>       AxAffine3F;
typedef AxAffine<double,3>      AxAffine3D;
typedef AxAffine<float,4>       AxAffine4F;

typedef Svec<AxAffine2F>        AxAffine2Fs;
typedef Svec<AxAffine3F>        AxAffine3Fs;
typedef Svec<AxAffine3D>        AxAffine3Ds;

template<class T,size_t D>
inline std::ostream &   operator<<(std::ostream & os,const AxAffine<T,D> & v) {return os  << v.aff; }

template<class T,size_t D>
inline AxAffine<T,D>    operator*(AxScale<T,D> l,Trans<T,D> r) {return {r,l}; }

template<class T,size_t D>
AxAffine<T,D>           operator*(Trans<T,D> l,AxAffine<T,D> const & r) {return {r.scales,r.trans+l.delta}; }

template<size_t D>
AxAffine<double,D>      interpolate(AxAffine<double,D> lhs,AxAffine<double,D> rhs,double val)
{
    return {
        mapExp(interpolate(mapLog(lhs.scales),mapLog(rhs.scales),val)),
        interpolate(lhs.trans,rhs.trans,val)
    };
}

template <class T,size_t D>
bool                isApproxEqual(AxAffine<T,D> const & lhs,AxAffine<T,D> const & rhs,T maxDiff)
{
    return
        isApproxEqual(lhs.scales,rhs.scales,maxDiff) &&
        isApproxEqual(lhs.trans,rhs.trans,maxDiff);
}

// Full N-dimensional version with matrix/vector members
template <class T,size_t D>
struct      Affine
{
    Mat<T,D,D>          linear;           // Applied first
    Mat<T,D,1>          translation;      // Applied second
    FG_SER(linear,translation)

    Affine() : linear{cMatDiag<T,D>(1)}, translation{0} {}
    // Construct from translation: f(x) = x + b
    explicit Affine(Mat<T,D,1> const & trans) : linear {cMatDiag<T,D>(1)}, translation(trans) {}
    explicit Affine(const Mat<T,D,D> & lin) : linear(lin), translation{0} {}
    explicit Affine(ScaleTrans<T,D> const & st) : linear{cMatDiag<T,D>(st.scale)}, translation{st.trans} {}
    explicit Affine(AxAffine<T,D> const & a) : linear{cMatDiag(a.scales)}, translation{a.trans} {}
    // Construct from native form: f(x) = Mx + b
    Affine(Mat<T,D,D> const & lin,Mat<T,D,1> const & trans) : linear(lin), translation(trans) {}
    // Construct from opposite order form: f(x) = M(x+b) = Mx + Mb
    Affine(Mat<T,D,1> const & trans,const Mat<T,D,D> & lin) : linear(lin), translation(lin * trans) {}

    // Don't let conversion constructor override default copy constructor:
    Affine(Affine const &) = default;
    Affine &            operator=(Affine const &) = default;
    // Conversion constructor:
    template<typename U>
    Affine(Affine<U,D> const & v) : linear(v.linear), translation(v.translation) {}

    // If 'vec' is a matrix, its columns are transformed as vectors into a new matrix:
    template<size_t C>
    Mat<T,D,C>      operator*(const Mat<T,D,C> & vec) const
    {
        Mat<T,D,C> ret = linear * vec;
        for (size_t col=0; col<C; col++)
            for (size_t row=0; row<D; row++)
                ret.rc(row,col) += translation[row];
        return ret;
    }

    // Operator composition: L*R -> L(Rx+r) + l = LRx + Lr + l = (LR)x + (Lr+l)
    Affine              operator*(const Affine & rhs) const
    {
        Affine       ret;
        ret.linear = linear * rhs.linear;
        ret.translation = linear * rhs.translation + translation;
        return ret;
    }

    // new = scalar * old:
    void                postScale(T val) {linear *= val; translation *= val; }

    // Ax + a = y -> x = A^-1(y - a) = (A^-1)y - (A^-1a)
    Affine              inverse() const
    {
        Affine       ret;
        ret.linear = cInverse(linear);
        ret.translation = - ret.linear * translation;
        return ret;
    }
};

typedef Affine<float,2>        Affine2F;
typedef Affine<double,2>       Affine2D;
typedef Affine<float,3>        Affine3F;
typedef Affine<double,3>       Affine3D;

// Operator composition: N(Mx+b) = (NM)x + Nb
template<class T,size_t D>
Affine<T,D>       operator*(const Mat<T,D,D> & lhs,const Affine<T,D> & rhs)
{
    return Affine<T,D>(lhs*rhs.linear,lhs*rhs.translation);
}
// operator composition: Affine * ScaleTrans:
template<class T,size_t D>
Affine<T,D>         operator*(Affine<T,D> const & l,ScaleTrans<T,D> const & r)
{
    return l * Affine<T,D>{r};
}

template<class T,size_t D>
std::ostream &      operator<<(std::ostream & os,const Affine<T,D> & v)
{
    return os  << fgnl << "Linear: " << v.linear << fgnl << " Translation: " << v.translation;
}

template<typename T,size_t D>
Mat<T,D+1,D+1>  asHomogMat(Affine<T,D> a)
{
    Mat<T,D+1,D+1>      ret = asHomogMat(a.linear);
    for (size_t ii=0; ii<D; ++ii)
        ret.rc(ii,D) = a.translation[ii];
    return ret;
}

// Returns least squares affine transform for sets of points in 1-1 correspondence.
// Must be at least 4 non-degenerate point pairs.
Affine3D            solveAffine(Vec3Ds const & base,Vec3Ds const & targ);

// Keeps a normalized quaternion.
// If you must access members directly, keep normalized or some functions won't work.
template<typename T>
struct      Quaternion
{
    // Real component, identity when 1 (with quaternion normalized):
    T                   real;
    // Imaginary components. Direction is rotation axis (RHR) and length (when quaternion is normalized)
    // is twice the rotation in radians for small values (tangent rotations):
    Mat<T,3,1>          imag;
    FG_SER(real,imag)

    Quaternion() : real{1}, imag{0} {}      // Default constructor is identity
    Quaternion(T r,Mat<T,3,1> i) : real{r}, imag{i} {normalizeP(); }
    // RHR tangent rotation around an arbitrary axis:
    Quaternion(T r,T rotAxisX,T rotAxisY,T rotAxisZ) : real{r}, imag{rotAxisX,rotAxisY,rotAxisZ} {normalizeP(); }
    // RHR rotation around a coordinate axis (0: X, 1: Y, 2: Z):
    Quaternion(T radians,size_t axis) : real{std::cos(radians/2)}, imag{0}
    {
        FGASSERT(axis < 3);
        imag[axis] = std::sin(radians/2);
    }
    explicit Quaternion(Mat<T,4,1> const & v) : real{v[0]}, imag{v[1],v[2],v[3]} {normalizeP(); }

    bool                operator==(Quaternion const & rhs) const {return ((real == rhs.real) && (imag == rhs.imag)); }
    // Composition operator. Retains normalization to precision but does NOT renormalize:
    Quaternion          operator*(Quaternion const & rhs) const
    {
        Quaternion      ret;
        ret.real = real*(rhs.real) - cDot(imag,rhs.imag);
        ret.imag = real*(rhs.imag) + (rhs.real)*imag + crossProduct(imag,rhs.imag);
        return ret;
    }
    Quaternion          inverse() const {return {real,-imag}; }
    Mat<T,3,3>          asMatrix() const
    {
        T               rm = sqr(real),
                        im = sqr(imag[0]), 
                        jm = sqr(imag[1]),
                        km = sqr(imag[2]),
                        ii01 = imag[0] * imag[1],
                        ii02 = imag[0] * imag[2],
                        ii12 = imag[1] * imag[2],
                        ri0 = real * imag[0],
                        ri1 = real * imag[1],
                        ri2 = real * imag[2];
        return {
            rm+im-jm-km,    (ii01-ri2)*2,   (ii02+ri1)*2,
            (ii01+ri2)*2,   rm-im+jm-km,    (ii12-ri0)*2,
            (ii02-ri1)*2,   (ii12+ri0)*2,   rm-im-jm+km,
        };
    }
    // application to vectors. Not efficient for repeated use; use 'asMatrix' explicitly in this case:
    template<size_t C>
    Mat<T,3,C>          operator*(Mat<T,3,C> const & r) const {return asMatrix() * r; }
    Mat<T,4,1>          asVec4() const {return Mat<T,4,1>(real,imag[0],imag[1],imag[2]); }
    // should always be very close to 1, this is just for testing:
    T                   magD() const {return sqr(real) + cMagD(imag); }
    // useful for deserialization. Returns false if zero magnitude:
    bool                normalize()
    {
        T               m = magD();
        if (m == T(0))
            return false;
        T               len = sqrt(m);
        real /= len;
        imag /= len;
        return true;
    }
    void            normalizeP() {FGASSERT(normalize()); }
    // Samples evenly from SO(3):
    static Quaternion rand() {return Quaternion(Mat<T,4,1>::randNormal()); } // Use normal distros to ensure isotropy
};

template <class T>
std::ostream &      operator<<(std::ostream& s,Quaternion<T> const & q) {return (s << q.asVec4()); }

typedef Quaternion<float>       QuaternionF;
typedef Quaternion<double>      QuaternionD;
typedef Svec<QuaternionD>       QuaternionDs;

inline QuaternionD  cRotateX(double radians) {return {radians,0}; }
inline QuaternionD  cRotateY(double radians) {return {radians,1}; }
inline QuaternionD  cRotateZ(double radians) {return {radians,2}; }

// if the distrubution is well clustered, this gives a good mean. If not, results will be unstable:
QuaternionD         cMean(QuaternionDs const &);

// Return the tangent magnitude of the difference between two quaternions (in double-radians squared).
// Useful for rotation prior.
double              tanDeltaMag(QuaternionD const & lhs,QuaternionD const & rhs);

// Approximates exponential map interpolation
QuaternionD         interpolate(
    QuaternionD         q0,         // Must be normalized
    QuaternionD         q1,         // Must be normalized
    double              val);       // 0: q0, 1: q1

bool                isApproxEqual(QuaternionD const & l,QuaternionD const & r,double prec);

struct  Rigid3D
{
    QuaternionD         rot;            // Applied first
    Vec3D               trans {0};      // Applied last
    FG_SER(rot,trans)

    Rigid3D() {}
    explicit Rigid3D(QuaternionD const & r) : rot{r} {}
    explicit Rigid3D(Vec3D const & t) : trans{t} {}
    Rigid3D(QuaternionD const & r,Vec3D const & t) : rot{r}, trans{t} {}
    // construct from translation followed by rotation: y = R(x+t) = Rx + Rt
    Rigid3D(Vec3D const & t,QuaternionD const & r) : rot{r}, trans{r*t} {}

    Affine3D        asAffine() const {return Affine3D {rot.asMatrix(),trans}; }
    Rigid3D         inverse() const {return Rigid3D {-trans,rot.inverse()}; }
    // composition operators:
    Rigid3D         operator*(Rigid3D const & r) const {return {rot*r.rot,trans+rot*r.trans}; }
    // R(Qx) + t = RQx + t
    Rigid3D         operator*(QuaternionD const & q) const {return {rot*q,trans}; }
    // R(x + t') + t = Rx + (Rt' + t)
    Rigid3D         operator*(Trans3D const & t) const {return {rot,rot*t.delta+trans}; }

    static Rigid3D  randNormal(double transStdev)
    {return Rigid3D{QuaternionD::rand(),Vec3D::randNormal(transStdev)}; }
};
typedef Svec<Rigid3D>   Rigid3Ds;
std::ostream &      operator<<(std::ostream &,Rigid3D const &);

// composition:
inline Rigid3D      operator*(Trans3D const & l,QuaternionD const & r) {return {r,l.delta}; }
inline Rigid3D      operator*(QuaternionD const & l,Trans3D const & r) {return {l,l*r.delta}; }
inline Rigid3D      operator*(Trans3D const & l,Rigid3D const & r) {return {r.rot,r.trans+l.delta}; }

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
    FG_SER(scale,rot,trans)

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
// Keeps translation relative to the input shape (v) not the output (v')
// BUT it couples the rotation and translation parameters.
struct  SimilarityRD
{
    Vec3D           trans {0};          // Translation applied first
    QuaternionD     rot;
    double          scale {1};          // Scale and rotation applied last
    FG_SER(trans,rot,scale)

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
    // operator* in this context means composition:
    SimilarityRD            operator*(SimilarityRD const & rhs) const;

    static SimilarityRD     identity() {return SimilarityRD {Vec3D(0),QuaternionD{},1}; }
    static size_t constexpr dof() {return 7; }
};
typedef Svec<SimilarityRD>   SimilarityRDs;

std::ostream &      operator<<(std::ostream & os,SimilarityRD const & v);

inline Vec3Ds       transform(SimilarityD const & sim,Vec3Ds const & poss) {return mapMulR(sim.asAffine(),poss); }

Uints               cHistogram(Doubles const & data,size_t numBuckets);

}

#endif
