//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Static checking for RGBA channel definitions & ordering.
// All operators assume color channels are alpha-pre-weighted.
//

#ifndef FGRGBA_HPP
#define FGRGBA_HPP

#include "FgApproxEqual.hpp"

namespace Fg {

template<typename T>
struct      Rgba
{
    Arr<T,4>            m_c;
    FG_SER1(m_c)

    typedef T           ValueType;

    Rgba() {};
    explicit Rgba(T val) : m_c(cArr<T,4>(val)) {}
    explicit Rgba(Arr<T,4> const & arr) : m_c(arr) {}

    // Otherwise the conversion constuctor would override:
    Rgba(Rgba const &) = default;
    Rgba &          operator=(Rgba const &) = default;
    Rgba(T r,T g,T b,T a) : m_c {{r,g,b,a}} {}
    // Conversion constructor
    template<class U>
    explicit Rgba(Rgba<U> const & val) : m_c(mapCast<T,U,4>(val.m_c))  {}

    T const &       operator[](size_t idx) const {return m_c[idx]; }
    T &             operator[](size_t idx) {return m_c[idx]; }
    T &             red() {return m_c[0]; }
    T &             green() {return m_c[1]; }
    T &             blue() {return m_c[2]; }
    T &             alpha() {return m_c[3]; }
    T const &       red() const {return m_c[0]; }
    T const &       green() const {return m_c[1]; }
    T const &       blue() const {return m_c[2]; }
    T const &       alpha() const {return m_c[3]; }

    Arr<T,3>        rgb() const {return {{m_c[0],m_c[1],m_c[2]}}; }
    // only use arithmetic with alpha-weighted values !
    Rgba            operator+(Rgba const & rhs) const {return Rgba(m_c+rhs.m_c); }
    Rgba            operator-(Rgba const & rhs) const {return Rgba(m_c-rhs.m_c); }
    Rgba            operator*(Rgba const &rhs) const {return Rgba(mapMul(m_c,rhs.m_c)); }
    Rgba            operator*(T val) const {return Rgba(m_c * val); }
    Rgba            operator/(T val) const {return Rgba(m_c/val); }
    Rgba const &    operator*=(T v) {m_c*=v; return *this; }
    Rgba const &    operator/=(T v) {m_c/=v; return *this; }
    Rgba const &    operator+=(Rgba rhs) {m_c += rhs.m_c; return *this; }
    bool            operator==(Rgba const & rhs) const {return m_c == rhs.m_c; }
    bool            operator!=(Rgba const & rhs) const {return !(m_c == rhs.m_c); }
    T               rec709() const                      // Use rec.709 RGB -> CIE L
    {
        return static_cast<T>(0.213 * red() + 0.715 * green() + 0.072 * blue());
    }
    void            alphaWeight()
    {
        uint        a = uint(m_c[3]);
        m_c[0] = uchar((uint(m_c[0]) * a + 127) / 255);
        m_c[1] = uchar((uint(m_c[1]) * a + 127) / 255);
        m_c[2] = uchar((uint(m_c[2]) * a + 127) / 255);
    }

    static Rgba<T>  fromRgbaPtr(T const * v) {return Rgba<T>(v[0],v[1],v[2],v[3]); }
};

typedef Rgba<uchar>     Rgba8;
typedef Rgba<ushort>    Rgba16;
typedef Rgba<uint>      RgbaUI;
typedef Rgba<float>     RgbaF;
typedef Rgba<double>    RgbaD;

typedef Svec<Rgba8>     RgbaUCs;
typedef Svec<RgbaF>     RgbaFs;

template<typename T>
struct      Traits<Rgba<T> >
{
    typedef typename Traits<T>::Scalar             Scalar;
    typedef Rgba<typename Traits<T>::Accumulator>  Accumulator;
    typedef Rgba<typename Traits<T>::Floating>     Floating;
    typedef Rgba<typename Traits<T>::Printable>    Printable;
};

template<typename T>
Rgba<T>             operator*(Rgba<T> lhs, T rhs)
{
    lhs *= rhs;
    return lhs;
}

template<typename To,typename From>
void                mapRound_(Rgba<From> const & in,Rgba<To> & out) {mapRound_(in.m_c,out.m_c); }

template<typename T>
std::ostream &      operator<<(std::ostream & out,Rgba<T> p) {return out << p.m_c; }

template<typename T>
Mat<T,4,2>          cBounds(Svec<Rgba<T>> const & vals)    // If empty, returns [max,lowest]
{
    Mat<T,4,2>              ret = catHoriz(
        Mat<T,4,1>{std::numeric_limits<T>::max()},
        Mat<T,4,1>{std::numeric_limits<T>::lowest()}
    );
    for (Rgba<T> const & v : vals) {
        for (uint ii=0; ii<4; ++ii) {
            updateMin_(ret.rc(ii,0),v[ii]);
            updateMax_(ret.rc(ii,1),v[ii]);
        }
    }
    return ret;
}

template<class To,class From>
Rgba<To>            mapCast(Rgba<From> const & from) {return Rgba<To>{mapCast<To,From>(from.m_c)}; }

template<typename To,typename From>
inline void         mapCast_(Rgba<From> const & from,Rgba<To> & to) {mapCast_(from.m_c,to.m_c); }

template<typename T>
double              cDot(Rgba<T> const & l,Rgba<T> const & r) {return cDot(l.m_c,r.m_c); }

template<typename T>
double              cSsd(Rgba<T> const & l,Rgba<T> const & r) {return cSsd(l.m_c,r.m_c); }

template<class T>
inline bool         isApproxEqual(Rgba<T> l,Rgba<T> r,T maxDiff) {return isApproxEqual(l.m_c,r.m_c,maxDiff); }

}

#endif
