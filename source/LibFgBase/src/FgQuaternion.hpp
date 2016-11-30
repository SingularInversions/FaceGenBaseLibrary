//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     July 6, 2005
//
// Holds a normalized quaternion.
//
// USE:
//
// Default constructor is identity.
//
// The tangential rotation described by a quaternion's 3 complex components are 
// RHR rotations around the X, Y and Z axes resp, in radians times 2.0.
//

#ifndef FGQUATERNION_HPP
#define FGQUATERNION_HPP

#include "FgStdLibs.hpp"

#include "FgMath.hpp"
#include "FgMatrix.hpp"
#include "FgRandom.hpp"

template<typename T>
class FgQuaternion
{
    T                   m_real;       // Real component
    FgMatrixC<T,3,1>    m_comp;       // Complex components

public:
    FgQuaternion() : m_real(T(1)) {}

    // Create a rotation around a coordinate axis (0 - X, 1 - Y, 2 - Z):
    FgQuaternion(T radians,uint axis) :
        m_real(std::cos(radians*T(0.5)))
    {
        FGASSERT(axis < 3);
        m_comp[axis] = std::sin(radians*T(0.5));
    }

    // The two constructors below are tangent (an approx to twice the 'r' value in radians)
    // rotations around the given axis:
    FgQuaternion(T r,FgMatrixC<T,3,1> rotAxis) :
        m_real(r),
        m_comp(rotAxis) 
    {normalizeP(); }
    FgQuaternion(T r,T rotX,T rotY,T rotZ) :
        m_real(r),
        m_comp(rotX,rotY,rotZ)
    {normalizeP(); }

    explicit 
    FgQuaternion(const FgMatrixC<T,4,1> & v) :
        m_real(v[0]),
        m_comp(v[1],v[2],v[3])
    {normalizeP(); }

    T
    real() const 
    {return m_real; }

    FgQuaternion &
    setIdentity()
    { m_real=T(1); m_comp.setConstant(0); return *this; }

    // operator* in this context means composition
    FgQuaternion
    operator*(const FgQuaternion &rhs) const
    {
        return FgQuaternion(
            m_real*(rhs.m_real) - fgDot(m_comp,rhs.m_comp),
            m_real*(rhs.m_comp) + (rhs.m_real)*m_comp + fgCrossProduct(m_comp,rhs.m_comp));
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

    FG_SERIALIZE2(m_real,m_comp);

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

private:
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
fgQuaternionRand()
{
    // Use normal distros to ensure isotropy:
    return FgQuaternionD(fgMatRandNormal<4,1>());
}

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
