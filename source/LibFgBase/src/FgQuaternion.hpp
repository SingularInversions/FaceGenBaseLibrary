//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Keeps a normalized quaternion.
//
// If you must access members directly, keep normalized or some functions won't work.
//

#ifndef FGQUATERNION_HPP
#define FGQUATERNION_HPP

#include "FgStdLibs.hpp"

#include "FgMath.hpp"
#include "FgMatrixC.hpp"
#include "FgMatrixV.hpp"
#include "FgRandom.hpp"

namespace Fg {

template<typename T>
struct  Quaternion
{
    // Real component, identity when 1 (with quaternion normalized):
    T               real;
    // Imaginary components. Direction is rotation axis (RHR) and length (when quaternion is normalized)
    // is twice the rotation in radians for small values (tangent rotations):
    Mat<T,3,1>      imag;
    FG_SERIALIZE2(real,imag);
    FG_SER2(real,imag)

    Quaternion() : real(1), imag(Mat<T,3,1>(0)) {}      // Default constructor is identity
    Quaternion(T r,Mat<T,3,1> i) : real(r), imag(i) {normalizeP(); }
    Quaternion(T r,T rotAxisX,T rotAxisY,T rotAxisZ) : real(r), imag(rotAxisX,rotAxisY,rotAxisZ) {normalizeP(); }
    // Create a rotation around a coordinate axis (0 - X, 1 - Y, 2 - Z):
    Quaternion(T radians,uint axis) : real(std::cos(radians/2))
    {
        FGASSERT(axis < 3);
        imag[axis] = std::sin(radians/2);
    }
    explicit Quaternion(Mat<T,4,1> const & v) : real(v[0]), imag(v[1],v[2],v[3]) {normalizeP(); }

    bool            operator==(const Quaternion & rhs) const {return ((real == rhs.real) && (imag == rhs.imag)); }
    // Composition operator. Retains normalization to precision but does NOT renormalize:
    Quaternion      operator*(const Quaternion & rhs) const
    {
        Quaternion    ret;
        ret.real = real*(rhs.real) - cDot(imag,rhs.imag);
        ret.imag = real*(rhs.imag) + (rhs.real)*imag + crossProduct(imag,rhs.imag);
        return ret;
    }
    Quaternion      inverse() const {return Quaternion(real,-imag); }
    Mat<T,3,3>      asMatrix() const
    {
        Mat<T,3,3>      ret;
        T               rm = sqr(real),
                        im = sqr(imag[0]), 
                        jm = sqr(imag[1]),
                        km = sqr(imag[2]);
        ret[0] = rm + im - jm - km;
        ret[4] = rm - im + jm - km;
        ret[8] = rm - im - jm + km;
        ret[1] = T(2) * (imag[0]*imag[1] - real*imag[2]);
        ret[2] = T(2) * (imag[0]*imag[2] + real*imag[1]);
        ret[3] = T(2) * (imag[0]*imag[1] + real*imag[2]);
        ret[5] = T(2) * (imag[1]*imag[2] - real*imag[0]);
        ret[6] = T(2) * (imag[0]*imag[2] - real*imag[1]);
        ret[7] = T(2) * (imag[1]*imag[2] + real*imag[0]);
        return ret;
    }
    Mat<T,4,1>      asVec4() const {return Mat<T,4,1>(real,imag[0],imag[1],imag[2]); }
    // should always be very close to 1, this is just for testing:
    T               mag() const {return sqr(real) + cMag(imag); }
    // useful for deserialization. Returns false if zero magnitude:
    bool            normalize()
    {
        T               m = mag();
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

template<class T,uint ncols>
Mat<T,3,ncols>      operator*(const Quaternion<T> & lhs,const Mat<T,3,ncols> & rhs) {return (lhs.asMatrix() * rhs); }
template <class T>
std::ostream &      operator<<(std::ostream& s,const Quaternion<T> & q) {return (s << q.asVec4()); }

typedef Quaternion<float>       QuaternionF;
typedef Quaternion<double>      QuaternionD;
typedef Svec<QuaternionD>       QuaternionDs;

inline QuaternionD  cRotateX(double radians) {return QuaternionD(radians,0); }
inline QuaternionD  cRotateY(double radians) {return QuaternionD(radians,1); }
inline QuaternionD  cRotateZ(double radians) {return QuaternionD(radians,2); }

// Return the tangent magnitude of the difference between two quaternions (in double-radians squared).
// Useful for rotation prior.
double              tanDeltaMag(QuaternionD const & lhs,QuaternionD const & rhs);
template<typename T>
Mat<T,4,4>          asHomogMat(Quaternion<T> q) {return asHomogMat(q.asMatrix()); }

// Approx exponential map interpolation. 'val' must be [0,1]:
QuaternionD         interpolate(
    QuaternionD         q0,         // Must be normalized
    QuaternionD         q1,         // Must be normalized
    double              val);       // Must be [0,1]

bool                isApproxEqual(QuaternionD const & l,QuaternionD const & r,double prec);

}

#endif
