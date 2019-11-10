//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
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
#include "FgMatrixC.hpp"
#include "FgMatrixV.hpp"
#include "FgOut.hpp"

namespace Fg {

template<typename T>
struct      Rgba
{
    Mat<T,4,1>        m_c;           // Keep as array so densely packed.
    FG_SERIALIZE1(m_c)

    typedef T ValueType;

    Rgba()
    {};

    explicit
    Rgba(T val)
    : m_c(val)
    {}

    explicit
    Rgba(const Mat<T,4,1> & mat)
    : m_c(mat)
    {}

    // Conversion constructor
    template<class U>
    explicit
    Rgba(const Rgba<U> & val)
    : m_c(Mat<T,4,1>(val.m_c)) 
    {}

    // Provide explicit CC when using templated conversion constructor to avoid compiler issues.
    Rgba(const Rgba & rhs)
    : m_c(rhs.m_c)
    {}

    Rgba(T r,T g,T b,T a)
    {red()=r; green()=g; blue()=b; alpha()=a; }

    T & red() {return m_c[0]; }
    T & green() {return m_c[1]; }
    T & blue() {return m_c[2]; }
    T & alpha() {return m_c[3]; }

    const T & red() const {return m_c[0]; }
    const T & green() const {return m_c[1]; }
    const T & blue() const {return m_c[2]; }
    const T & alpha() const {return m_c[3]; }

    Mat<T,3,1>
    rgb() const
    {return Mat<T,3,1>(m_c[0],m_c[1],m_c[2]); }

    Rgba
    operator+(const Rgba & rhs) const
    {return Rgba(m_c+rhs.m_c); }

    Rgba
    operator-(const Rgba & rhs) const
    {return Rgba(m_c-rhs.m_c); }

    Rgba
    operator*(const Rgba &rhs) const
    {return Rgba(fgMapMul(m_c,rhs.m_c)); }

    Rgba
    operator*(T val) const
    {return Rgba(m_c * val); }

    Rgba
    operator/(T val) const
    {return Rgba(m_c/val); }

    const Rgba &
    operator*=(T v)
    {m_c*=v; return *this; }

    const Rgba &
    operator/=(T v)
    {m_c/=v; return *this; }

    const Rgba &
    operator+=(Rgba rhs)
    {
        m_c += rhs.m_c;
        return *this;
    }
    
    bool
    operator==(Rgba const & rhs) const
    {return m_c == rhs.m_c; }

    bool
    operator!=(Rgba const & rhs) const
    {return !(m_c == rhs.m_c); }

    T       
    rec709() const                      // Use rec.709 RGB -> CIE L
    {return static_cast<T>(0.213 * red() + 0.715 * green() + 0.072 * blue()); }

    static
    Rgba<T>
    fromRgbaPtr(const T * v)
    {return Rgba<T>(v[0],v[1],v[2],v[3]); }

    void
    alphaWeight()
    {
        uint        a = uint(m_c[3]);
        m_c[0] = uchar((uint(m_c[0]) * a + 127) / 255);
        m_c[1] = uchar((uint(m_c[1]) * a + 127) / 255);
        m_c[2] = uchar((uint(m_c[2]) * a + 127) / 255);
    }
};

typedef Rgba<uchar>           RgbaUC;
typedef Rgba<ushort>          RgbaUS;
typedef Rgba<float>           RgbaF;
typedef Rgba<double>          RgbaD;

template<typename T>
struct  Traits<Rgba<T> >
{
    typedef typename Traits<T>::Scalar             Scalar;
    typedef Rgba<typename Traits<T>::Accumulator>  Accumulator;
    typedef Rgba<typename Traits<T>::Floating>     Floating;
};

template<typename T>
Rgba<T> operator*(Rgba<T> lhs, T rhs)
{
    lhs *= rhs;
    return lhs;
}

template<typename To,typename From>
void
round_(Rgba<From> const & in,Rgba<To> & out)
{round_(in.m_c,out.m_c); }

template<typename T>
FgOut & operator<<(FgOut & out, const Rgba<T> & pixel)
{
    return out << "{" 
               << int(pixel.red())   << "," 
               << int(pixel.green()) << ","
               << int(pixel.blue())  << ","
               << int(pixel.alpha()) << "}";
}

// NB: This function assumes pre-weighted colour values !!!
inline
RgbaUC
fgCompositeFragmentUnweighted(RgbaUC foreground,RgbaUC background)
{
	uint		fga = foreground.alpha(),
				bga = background.alpha(),
				tra = 255 - fga,
				aca = (tra * bga + 127) / 255;
	Vec3UI	fgc(foreground.m_c.subMatrix<3,1>(0,0)),
				bgc(background.m_c.subMatrix<3,1>(0,0)),
				acc = fgc + (bgc * aca + Vec3UI(127)) / 255;
	return
		RgbaUC(acc[0],acc[1],acc[2],fga+aca);
}

// NB: This function assumes pre-weighted colour values !!!
inline
RgbaUC
fgCompositeFragment(RgbaUC foreground,RgbaUC background)
{
    uint        fac = 255 - foreground.alpha();
    Vec4UI   acc = Vec4UI(background.m_c) * fac + Vec4UI(127);
    return (foreground + RgbaUC(Mat<uchar,4,1>(acc/255)));
}
inline
RgbaF
fgCompositeFragment(RgbaF foreground,RgbaF background)
{
    float       fac = (255.0f - foreground.alpha()) / 255.0f;
    return (foreground + background * fac);
}

template<typename To,typename From>
Rgba<To>
round(Rgba<From> const & v)
{return Rgba<To>(round<To>(v.m_c)); }

template<class T,class U>
void
scast_(const Rgba<T> & i,Rgba<U> & o)
{scast_(i.m_c,o.m_c); }

}

#endif
