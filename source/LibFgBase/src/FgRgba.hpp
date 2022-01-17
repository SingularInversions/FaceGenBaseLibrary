//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Templated RGBA pixel type.
//
// All operators assume color channels are pre-weighted by alpha.
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
    Arr<T,4>            m_c;
    FG_SERIALIZE1(m_c)

    typedef T ValueType;

    Rgba() {};
    explicit Rgba(T val) : m_c(cArr<T,4>(val)) {}
    explicit Rgba(Arr<T,4> const & arr) : m_c(arr) {}

    // Conversion constructor
    template<class U>
    explicit
    Rgba(Rgba<U> const & val)
    : m_c(mapCast<T,U,4>(val.m_c)) 
    {}

    // Otherwise the conversion constuctor above would override:
    Rgba(Rgba const & rhs) = default;

    Rgba(T r,T g,T b,T a) : m_c {{r,g,b,a}} {}

    T const &   operator[](size_t idx) const {return m_c[idx]; }
    T &         operator[](size_t idx) {return m_c[idx]; }

    T & red() {return m_c[0]; }
    T & green() {return m_c[1]; }
    T & blue() {return m_c[2]; }
    T & alpha() {return m_c[3]; }

    T const & red() const {return m_c[0]; }
    T const & green() const {return m_c[1]; }
    T const & blue() const {return m_c[2]; }
    T const & alpha() const {return m_c[3]; }

    Arr<T,3>
    rgb() const
    {return {{m_c[0],m_c[1],m_c[2]}}; }

    Rgba
    operator+(Rgba const & rhs) const
    {return Rgba(m_c+rhs.m_c); }

    Rgba
    operator-(Rgba const & rhs) const
    {return Rgba(m_c-rhs.m_c); }

    Rgba
    operator*(Rgba const &rhs) const
    {return Rgba(mapMul(m_c,rhs.m_c)); }

    Rgba
    operator*(T val) const
    {return Rgba(m_c * val); }

    Rgba
    operator/(T val) const
    {return Rgba(m_c/val); }

    Rgba const &
    operator*=(T v)
    {m_c*=v; return *this; }

    Rgba const &
    operator/=(T v)
    {m_c/=v; return *this; }

    Rgba const &
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
    fromRgbaPtr(T const * v)
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

typedef Rgba<uchar>             Rgba8;
typedef Rgba<ushort>            RgbaUS;
typedef Rgba<float>             RgbaF;
typedef Rgba<double>            RgbaD;

typedef Svec<Rgba8>            RgbaUCs;
typedef Svec<RgbaF>             RgbaFs;

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
std::ostream &
operator<<(std::ostream & out,Rgba<T> p)
{
    return out << p.m_c;
}

// Normal unweighted encoding:
// r_c = f_c * f_a + b_c * b_a * (1-f_a)
// r_a = f_a + b_a * (1-f_a)
inline
Rgba8
compositeFragmentUnweighted(Rgba8 foreground,Rgba8 background)
{
    uint        f_a = foreground.alpha(),
                b_a = background.alpha(),
                omfa = 255 - f_a,
                tmp = (b_a * omfa + 127U) / 255U;
    Arr3UI      f_c = mapCast<uint>(cHead<3>(foreground.m_c)),
                b_c = mapCast<uint>(cHead<3>(background.m_c)),
                acc = f_c * f_a + b_c * tmp,
                r_c = (acc + cArr<uint,3>(127)) / 255U;
    return      Rgba8(r_c[0],r_c[1],r_c[2],f_a+tmp);
}

// Assumes alpha-weighted colour values:
// r_c = f_c + b_c * (1-f_a)
// r_a = f_a + b_a * (1-f_a)
inline
Rgba8
compositeFragmentPreWeighted(Rgba8 foreground,Rgba8 background)
{
    uint            omfa = 255 - foreground.alpha();
    Arr4UI          acc = mapCast<uint>(background.m_c) * omfa + cArr<uint,4>(127);
    return (foreground + Rgba8(mapCast<uchar>(acc/255U)));
}
inline
RgbaF
compositeFragment(RgbaF foreground,RgbaF background)
{
    float       fac = (255.0f - foreground.alpha()) / 255.0f;
    return (foreground + background * fac);
}

template<typename To,typename From>
Rgba<To>
round(Rgba<From> const & v)
{return Rgba<To>(round<To>(v.m_c)); }

template<class To,class From>
Rgba<To>
mapCast(Rgba<From> const & from)
{return Rgba<To>{mapCast<To,From>(from.m_c)}; }

template<typename To,typename From>
inline void
deepCast_(Rgba<From> const & from,Rgba<To> & to)
{deepCast_(from.m_c,to.m_c); }

template<typename T>
double
cDot(Rgba<T> const & l,Rgba<T> const & r)
{
    return cDot(l.m_c,r.m_c);
}

template<typename T>
double
cSsd(Rgba<T> const & l,Rgba<T> const & r)
{
    return cSsd(l.m_c,r.m_c);
}

}

#endif
