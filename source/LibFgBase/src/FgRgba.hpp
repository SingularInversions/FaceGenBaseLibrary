//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Aug 27, 2004
//
// Templated RGBA pixel type.
//
// All operators assume color channels are pre-weighted by alpha.
//
// INVARIANTS:
//
// The 4 channels are densely packed in memory.
//
// The 3 colour channels are guaranteed contiguous.
//

#ifndef FGRGBA_HPP
#define FGRGBA_HPP

#include "FgStdLibs.hpp"

#include "FgTypes.hpp"
#include "FgDiagnostics.hpp"
#include "FgMatrix.hpp"
#include "FgOut.hpp"

template<typename T>
struct      FgRgba
{
    FgMatrixC<T,4,1>        m_c;           // Keep as array so densely packed.
    FG_SERIALIZE1(m_c)

    typedef T ValueType;

    FgRgba()
    {};

    explicit
    FgRgba(T val)
    : m_c(val)
    {}

    explicit
    FgRgba(const FgMatrixC<T,4,1> & mat)
    : m_c(mat)
    {}

    // Conversion constructor
    template<class U>
    explicit
    FgRgba(const FgRgba<U> & val)
    : m_c(FgMatrixC<T,4,1>(val.m_c)) 
    {}

    // Provide explicit CC when using templated conversion constructor to avoid compiler issues.
    FgRgba(const FgRgba & rhs)
    : m_c(rhs.m_c)
    {}

    FgRgba(T r,T g,T b,T a)
    {red()=r; green()=g; blue()=b; alpha()=a; }

    T & red() {return m_c[0]; }
    T & green() {return m_c[1]; }
    T & blue() {return m_c[2]; }
    T & alpha() {return m_c[3]; }

    const T & red() const {return m_c[0]; }
    const T & green() const {return m_c[1]; }
    const T & blue() const {return m_c[2]; }
    const T & alpha() const {return m_c[3]; }

    FgMatrixC<T,3,1>
    rgb() const
    {return FgMatrixC<T,3,1>(m_c[0],m_c[1],m_c[2]); }

    FgRgba
    operator+(const FgRgba & rhs) const
    {return FgRgba(m_c+rhs.m_c); }

    FgRgba
    operator-(const FgRgba & rhs) const
    {return FgRgba(m_c-rhs.m_c); }

    FgRgba
    operator*(const FgRgba &rhs) const
    {return FgRgba(fgMultiply(m_c,rhs.m_c)); }

    FgRgba
    operator*(T val) const
    {return FgRgba(m_c * val); }

    FgRgba
    operator/(T val) const
    {return FgRgba(m_c/val); }

    const FgRgba &
    operator*=(T v)
    {m_c*=v; return *this; }

    const FgRgba &
    operator/=(T v)
    {m_c/=v; return *this; }

    const FgRgba &
    operator+=(FgRgba rhs)
    {
        m_c += rhs.m_c;
        return *this;
    }
    
    bool
    operator==(FgRgba const & rhs) const
    {return m_c == rhs.m_c; }

    bool
    operator!=(FgRgba const & rhs) const
    {return !(m_c == rhs.m_c); }

    T       
    rec709() const                      // Use rec.709 RGB -> CIE L
    {return static_cast<T>(0.213 * red() + 0.715 * green() + 0.072 * blue()); };

    static
    FgRgba<T>
    fromRgbaPtr(const T * v)
    {return FgRgba<T>(v[0],v[1],v[2],v[3]); }

    void
    alphaWeight()
    {
        uint        a = uint(m_c[3]);
        m_c[0] = uchar((uint(m_c[0]) * a + 127) / 255);
        m_c[1] = uchar((uint(m_c[1]) * a + 127) / 255);
        m_c[2] = uchar((uint(m_c[2]) * a + 127) / 255);
    }
};

template<typename T>
FgRgba<T> operator*(FgRgba<T> lhs, T rhs)
{
    lhs *= rhs;
    return lhs;
}

template<typename T,typename U>
void
fgRound(
    const FgRgba<T> &   in,
    FgRgba<U> &         out)
{
    fgRound_(in.m_c,out.m_c);
}

template<typename T>
FgOut & operator<<(FgOut & out, const FgRgba<T> & pixel)
{
    return out << "{" 
               << int(pixel.red())   << "," 
               << int(pixel.green()) << ","
               << int(pixel.blue())  << ","
               << int(pixel.alpha()) << "}";
}

typedef FgRgba<uchar>           FgRgbaUB;
typedef FgRgba<ushort>          FgRgbaUS;
typedef FgRgba<float>           FgRgbaF;
typedef FgRgba<double>          FgRgbaD;

template<typename T>
struct  FgTraits<FgRgba<T> >
{
    typedef FgRgba<typename FgTraits<T>::Accumulator>  Accumulator;
    typedef FgRgba<typename FgTraits<T>::Floating>     Floating;
};

// NB: This function assumes pre-weighted colour values !!!
inline
FgRgbaUB
fgCompositeFragmentUnweighted(FgRgbaUB foreground,FgRgbaUB background)
{
	uint		fga = foreground.alpha(),
				bga = background.alpha(),
				tra = 255 - fga,
				aca = (tra * bga + 127) / 255;
	FgVect3UI	fgc(foreground.m_c.subMatrix<3,1>(0,0)),
				bgc(background.m_c.subMatrix<3,1>(0,0)),
				acc = fgc + (bgc * aca + FgVect3UI(127)) / 255;
	return
		FgRgbaUB(acc[0],acc[1],acc[2],fga+aca);
}

// NB: This function assumes pre-weighted colour values !!!
inline
FgRgbaUB
fgCompositeFragment(FgRgbaUB foreground,FgRgbaUB background)
{
    uint        fac = 255 - foreground.alpha();
    FgVect4UI   acc = FgVect4UI(background.m_c) * fac + FgVect4UI(127);
    return (foreground + FgRgbaUB(FgMatrixC<uchar,4,1>(acc/255)));
}
inline
FgRgbaF
fgCompositeFragment(FgRgbaF foreground,FgRgbaF background)
{
    float       fac = (255.0f - foreground.alpha()) / 255.0f;
    return (foreground + background * fac);
}

inline
FgRgbaUB
fgRoundU(FgRgbaF v)
{return FgRgbaUB(FgVect4UC(fgRoundU(v.m_c))); }

#endif
