//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     July 6, 2005
//
// Keeps a normalized quaternion.
//
// If you must access members directly, keep normalized or some functions won't work.
//

#ifndef FGQUATERNION_HPP
#define FGQUATERNION_HPP

#include "FgStdLibs.hpp"

#include "FgMath.hpp"
#include "FgMatrix.hpp"
#include "FgRandom.hpp"

template<typename T>
struct  FgQuaternion
{
    // Real component, identity when 1 (with quaternion normalized):
    T                   m_real;
    // Complex components. Direction is rotation axis (RHR) and length (when quaternion is normalized)
    // is twice the rotation in radians for small values (tangent rotations):
    FgMatrixC<T,3,1>    m_comp;
    FG_SERIALIZE2(m_real,m_comp);

    // Default constructor is identity:
    FgQuaternion() : m_real(1), m_comp(FgMatrixC<T,3,1>(0)) {}

    FgQuaternion(T real,FgMatrixC<T,3,1> complex) : m_real(real), m_comp(complex) 
    {normalizeP(); }

    FgQuaternion(T real,T rotAxisX,T rotAxisY,T rotAxisZ) : m_real(real), m_comp(rotAxisX,rotAxisY,rotAxisZ)
    {normalizeP(); }

    // Create a rotation around a coordinate axis (0 - X, 1 - Y, 2 - Z):
    FgQuaternion(T radians,uint axis) :
        m_real(std::cos(radians/2))
    {
        FGASSERT(axis < 3);
        m_comp[axis] = std::sin(radians/2);
    }

    explicit 
    FgQuaternion(const FgMatrixC<T,4,1> & v) : m_real(v[0]), m_comp(v[1],v[2],v[3])
    {normalizeP(); }

    bool
    operator==(const FgQuaternion & rhs) const
    {return ((m_real == rhs.m_real) && (m_comp == rhs.m_comp)); }

    // Composition operator:
    FgQuaternion
    operator*(const FgQuaternion & rhs) const
    {
        FgQuaternion    ret;
        ret.m_real = m_real*(rhs.m_real) - fgDot(m_comp,rhs.m_comp);
        ret.m_comp = m_real*(rhs.m_comp) + (rhs.m_real)*m_comp + fgCrossProduct(m_comp,rhs.m_comp);
        return ret;
    }

    FgQuaternion
    inverse() const
    {return FgQuaternion(m_real,-m_comp); }

    FgMatrixC<T,3,3>
    asMatrix() const
    {
        FgMatrixC<T,3,3>    ret;
        T                   sq = fgSqr(m_real),
                            lq = fgSqr(m_comp[0]), 
                            mq = fgSqr(m_comp[1]),
                            nq = fgSqr(m_comp[2]);
        ret[0] = sq + lq - mq - nq;
        ret[4] = sq - lq + mq - nq;
        ret[8] = sq - lq - mq + nq;
        ret[1] = T(2) * (m_comp[0]*m_comp[1] - m_real*m_comp[2]);
        ret[2] = T(2) * (m_comp[0]*m_comp[2] + m_real*m_comp[1]);
        ret[3] = T(2) * (m_comp[0]*m_comp[1] + m_real*m_comp[2]);
        ret[5] = T(2) * (m_comp[1]*m_comp[2] - m_real*m_comp[0]);
        ret[6] = T(2) * (m_comp[0]*m_comp[2] - m_real*m_comp[1]);
        ret[7] = T(2) * (m_comp[1]*m_comp[2] + m_real*m_comp[0]);
        return ret;
    }

    FgMatrixC<T,4,1>
    asVector() const
    {return FgMatrixC<T,4,1>(m_real,m_comp[0],m_comp[1],m_comp[2]); }

    bool            // false if zero magnitude
    normalize()     // Useful for deserialization of user data
    {
        T   mag = m_real*m_real + m_comp.mag();
        if (mag == T(0))
            return false;
        T   fac = T(1) / sqrt(mag);
        m_real *= fac;
        m_comp *= fac;
        return true;
    }

    // Returns tangent distance magnitude between rotations in radians squared
    // (only accurate for small diffs):
    double
    deltaRadiansMag(const FgQuaternion & rhs) const
    {
        FgMatrixC<T,4,1>    lv = asVector(),
                            rv = rhs.asVector();
        // Account for projective identity:
        if (fgDot(lv,rv) < 0)
            rv *= -1;
        // Small differences correspond to twice the difference in radians thus 4 times the magnitude:
        return (lv-rv).mag()*4;
    }

    void
    normalizeP()
    {FGASSERT(normalize()); }
};

template<class T,uint ncols>
FgMatrixC<T,3,ncols>
operator*(const FgQuaternion<T> & lhs,const FgMatrixC<T,3,ncols> & rhs)
{ return (lhs.asMatrix() * rhs); }

template <class T>
std::ostream& operator<<(std::ostream& s,const FgQuaternion<T> & q)
{return (s << q.asVector()); }

typedef FgQuaternion<float>    FgQuaternionF;
typedef FgQuaternion<double>   FgQuaternionD;

inline
FgQuaternionD
fgQuaternionRand()      // Samples evenly from SO(3)
{return FgQuaternionD(fgVecRandNrm<4>()); }     // Use normal distros to ensure isotropy

inline
FgQuaternionD
fgRotateX(double radians)
{return FgQuaternionD(radians,0); }

inline
FgQuaternionD
fgRotateY(double radians)
{return FgQuaternionD(radians,1); }

inline
FgQuaternionD
fgRotateZ(double radians)
{return FgQuaternionD(radians,2); }

#endif
