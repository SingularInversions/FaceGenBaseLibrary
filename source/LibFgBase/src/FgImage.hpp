//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Simple left-to-right, top-to-bottom (row major), tightly-packed, unaligned image templated by pixel type.
// 
// COORDINATE SYSTEMS:
//
// PACS := Pixel Area CS
//    Origin at lower corner (top left corner of image), units in pixels.
//    X - viewer’s right
//    Y - viewer’s down 
//
// IRCS := Image Raster CS
//    Origin at centre of first pixel in memory (top left), units are pixels, storage ordered by:
//    X - viewer’s right
//    Y - viewer’s down 
//
// IUCS := Image Unit CS
//    Origin at top left corner of image, (1,1) at bottom right corner.
//    X - viewer’s right
//    Y - viewer’s down 
//
// OTCS := OpenGL Texture CS
//    Origin at bottom left corner of image, (1,1) at top right
//    X - viewer’s right    [0,1]
//    Y - viewer’s up       [0,1]
//
// INVARIANTS:
//
// m_data.size() == m_dims[0] * m_dims[1];
//
// NOTES:
//
// * posIrcs = posPacs - 0.5 (thus floor(posPacs) rounds to nearest int posPacs)
// * posPacs = mapMul(posIucs,image.dims())
// * All pixel averaging / resampling operations on images with an alpha channel require the color
//   channels to be alpha-premultiplied (APM) for correct results.
// * Images loaded from formats supporting alpha (ie. PNG) are not APM and must be converted.
// * floating point channel images are linear APM (unless otherwise noted)
// * 8 bit channel images should always be gamma-encoded (but may or may not be APM)
// * 3 channel images are RGB and 4 channel images are RGBA

#ifndef FGIMAGE_HPP
#define FGIMAGE_HPP

#include "FgGeometry.hpp"

namespace Fg {

template<class T,size_t D>
Trans<T,D>          cPacsToIrcs() {return Trans<T,D>{-0.5}; }
template<class T,size_t D>
Trans<T,D>          cIrcsToPacs() {return Trans<T,D>{0.5}; }
template<class T>
AxAffine<T,2>       cIucsToOtcs() {return {Mat<T,2,1>{1,-1},Mat<T,2,1>{0,1}}; }
template<class T>
AxAffine<T,2>       cOtcsToIucs() {return {Mat<T,2,1>{1,-1},Mat<T,2,1>{0,1}}; }
template<class T,size_t D>
AxAffine<T,D>       cIucsToPacs(Mat<uint,D,1> dims) {return {mapCast<T>(dims),Mat<T,D,1>{0}}; }
template<class T,size_t D>
AxAffine<T,D>       cPacsToIucs(Mat<uint,D,1> dims) {return cIucsToPacs<T>(dims).inverse(); }
template<class T,size_t D>
AxAffine<T,D>       cIrcsToIucs(Mat<uint,D,1> dims) {return cPacsToIucs<T>(dims) * cIrcsToPacs<T,D>(); }
template<class T>
AxAffine<T,2>       cIrcsToOtcs(Mat<uint,2,1> dims) {return cIucsToOtcs<T>() * cPacsToIucs<T>(dims) * cIrcsToPacs<T,2>(); }
template<class T,size_t D>
AxAffine<T,D>       cIucsToIrcs(Mat<uint,D,1> dims) {return cPacsToIrcs<T,D>() * cIucsToPacs<T>(dims); }
template<class T>
AxAffine<T,2>       cOicsToIucs() {return {Mat<T,2,1>{0.5,-0.5},Mat<T,2,1>{0.5,0.5}}; }
template<class T>
AxAffine<T,2>       cOtcsToPacs(Mat<uint,2,1> dims) {return cIucsToPacs<T>(dims) * cOtcsToIucs<T>(); }
template<class T>
AxAffine<T,2>       cOtcsToIrcs(Mat<uint,2,1> dims) {return cPacsToIrcs<T,2>() * cOtcsToPacs<T>(dims); }

// needed because (uchar*uchar),(uchar+uchar),(ushort*ushort),(ushort+ushort) return (int):
template<class T>
Arr<T,4>    mapAddCast(Arr<T,4> l,Arr<T,4> r) {return mapCall(l,r,[](T e,T f){return scast<T>(e+f); }); }
template<class T>
Arr<T,4>    mapSubCast(Arr<T,4> l,Arr<T,4> r) {return mapCall(l,r,[](T e,T f){return scast<T>(e-f); }); }
template<class T>
Arr<T,4>    mapMulCast(Arr<T,4> l,Arr<T,4> r) {return mapCall(l,r,[](T e,T f){return scast<T>(e*f); }); }
template<class T>
Arr<T,4>    mapMulCast(Arr<T,4> a,T v) {return mapCall(a,[v](T e){return scast<T>(e*v); }); }
template<class T>
Arr<T,4>    mapDivCast(Arr<T,4> a,T v) {return mapCall(a,[v](T e){return scast<T>(e/v); }); }

template<typename T>
struct      Rgba
{
    Arr<T,4>            m_c;
    FG_SER(m_c)

    typedef T           ValueType;

    Rgba() {};
    explicit Rgba(T val) : m_c{val} {}
    explicit Rgba(Arr<T,4> const & arr) : m_c(arr) {}

    // Otherwise the conversion constuctor would override:
    Rgba(Rgba const &) = default;
    Rgba &          operator=(Rgba const &) = default;
    Rgba(T r,T g,T b,T a) : m_c {r,g,b,a} {}
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

    Arr<T,3>        rgb() const {return {m_c[0],m_c[1],m_c[2]}; }
    // only use arithmetic with alpha-weighted values !
    // note that we need to recast the results since arithmetic operations on 8 and 16 bit numbers return 'int':
    Rgba            operator+(Rgba rhs) const {return Rgba{mapAddCast(m_c,rhs.m_c)}; }
    Rgba            operator-(Rgba rhs) const {return Rgba{mapSubCast(m_c,rhs.m_c)}; }
    Rgba            operator*(T val) const {return Rgba{mapMulCast(m_c,val)}; }
    Rgba            operator/(T val) const {return Rgba{mapDivCast(m_c,val)}; }
    Rgba const &    operator*=(T v) {m_c*=v; return *this; }
    Rgba const &    operator/=(T v) {m_c/=v; return *this; }
    Rgba const &    operator+=(Rgba rhs) {m_c += rhs.m_c; return *this; }
    bool            operator==(Rgba rhs) const {return m_c == rhs.m_c; }
    bool            operator!=(Rgba rhs) const {return !(m_c == rhs.m_c); }
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

typedef Svec<Rgba8>     Rgba8s;
typedef Svec<RgbaF>     RgbaFs;

template<typename T>
struct      Traits<Rgba<T> >
{
    typedef typename Traits<T>::Scalar             Scalar;
    typedef Rgba<typename Traits<T>::Floating>     Floating;
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

template<class T>
inline void         updateMin_(Rgba<T> & mvs,Rgba<T> vs)
{
    for (size_t ii=0; ii<4; ++ii)
        updateMin_(mvs[ii],vs[ii]);
}
template<class T>
inline void         updateMax_(Rgba<T> & mvs,Rgba<T> vs)
{
    for (size_t ii=0; ii<4; ++ii)
        updateMax_(mvs[ii],vs[ii]);
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

template<typename T>
struct      Img
{
    Vec2UI          m_dims;         // [width,height]
    Svec<T>         m_data;         // Pixels stored left to right, top to bottom. size() == m_dims[0]*m_dims[1]
    FG_SER(m_dims,m_data)

    typedef T PixelType;

    Img() : m_dims(0) {}
    explicit Img(Vec2UI dims) : m_dims{dims}, m_data(dims[0]*dims[1]) {}
    Img(size_t wid,size_t hgt) : m_dims{uint(wid),uint(hgt)}, m_data(wid*hgt) {}
    Img(size_t wid,size_t hgt,T fill) : m_dims{uint(wid),uint(hgt)}, m_data(wid*hgt,fill) {}
    Img(size_t wid,size_t hgt,Svec<T> const & data) : m_dims{uint(wid),uint(hgt)}, m_data(data)
        {FGASSERT(data.size() == m_dims.elemsProduct()); }
    Img(Vec2UI dims,T fillVal) : m_dims{dims}, m_data(dims.elemsProduct(),fillVal) {}
    Img(Vec2UI dims,Svec<T> const & imgData) : m_dims{dims}, m_data{imgData}
        {FGASSERT(m_data.size() == m_dims.elemsProduct()); }
    Img(Vec2UI dims,T const * pixels) : m_dims{dims}, m_data(pixels,pixels+dims.elemsProduct()) {}

    uint            width() const {return m_dims[0]; }
    uint            height() const {return m_dims[1]; }
    Vec2UI          dims() const {return m_dims; }
    size_t          numPixels() const {return m_data.size(); }
    bool            empty() const {return (m_data.empty()); }

    void            clear() {m_data.clear(); m_dims = Vec2UI{0}; }
    // WARNING: This does not initialize image data, just sets dimensions and resizes memory:
    void            resize(uint wid,uint hgt) {m_dims = Vec2UI{wid,hgt}; m_data.resize(wid*hgt); }
    void            resize(Vec2UI dims) {m_dims = dims; m_data.resize(dims[0]*dims[1]); }
    void            resize(Vec2UI dims,T fillVal)  {m_dims = dims; m_data.resize(dims[0]*dims[1],fillVal); }

    // Element access by (X,Y) / (column,row):
    T &             xy(size_t ircs_x,size_t ircs_y)
    {
        FGASSERT_FAST((ircs_x < m_dims[0]) && (ircs_y < m_dims[1]));
        return m_data[ircs_y*m_dims[0]+ircs_x];
    }
    T const &       xy(size_t ircs_x,size_t ircs_y) const
    {
        FGASSERT_FAST((ircs_x < m_dims[0]) && (ircs_y < m_dims[1]));
        return m_data[ircs_y*m_dims[0]+ircs_x];
    }
    template<class U,FG_ENABLE_IF(U,is_integral)>
    T const &       operator[](Arr<U,2> ircs) const {return xy(ircs[0],ircs[1]); }
    template<class U,FG_ENABLE_IF(U,is_integral)>
    T &             operator[](Arr<U,2> ircs) {return xy(ircs[0],ircs[1]); }
    template<class U,FG_ENABLE_IF(U,is_integral)>
    T const &       operator[](Mat<U,2,1> ircs) const {return xy(ircs[0],ircs[1]); }
    template<class U,FG_ENABLE_IF(U,is_integral)>
    T &             operator[](Mat<U,2,1> ircs) {return xy(ircs[0],ircs[1]); }
    T const *       dataPtr() const {return (!m_data.empty() ? &m_data[0] : nullptr); }
    T *             dataPtr() {return (!m_data.empty() ? &m_data[0] : nullptr); }
    Svec<T> const & dataVec() const {return m_data; }
    T *             rowPtr(size_t row) {return &m_data[row*m_dims[0]]; }
    T const *       rowPtr(size_t row) const {return &m_data[row*m_dims[0]]; }

    bool            operator==(Img const & rhs) const {return ((m_dims == rhs.m_dims) && (m_data == rhs.m_data)); }
    Img             operator+(Img const & r) const {FGASSERT(m_dims==r.m_dims); return Img{m_dims,m_data+r.m_data}; }
    Img             operator-(Img const & r) const {FGASSERT(m_dims==r.m_dims); return Img{m_dims,m_data-r.m_data}; }

    // 'paint' access is bounds checked and out of bounds paints are ignored:
    void            paint(uint ircs_x,uint ircs_y,T val)
    {
        if ((ircs_x < m_dims[0]) && (ircs_y < m_dims[1]))
            m_data[ircs_y*m_dims[0]+ircs_x] = val;
    }
    void            paint(Vec2UI ircs,T val) {paint(ircs[0],ircs[1],val); }
    void            paint(Vec2I ircs,T val)
    {
        if ((ircs[0] >= 0) && (ircs[1] >= 0))
            paint(ircs[0],ircs[1],val);
    }
};

typedef Img<uchar>          ImgUC;
typedef Img<ushort>         ImgUS;
typedef Img<uint>           ImgUI;
typedef Img<float>          ImgF;
typedef Img<double>         ImgD;

typedef Img<Vec2F>          Img2F;

typedef Img<Arr3SC>         Img3SC;
typedef Img<Arr3I>          Img3I;
typedef Img<Arr3F>          Img3F;
typedef Svec<Img3F>         Img3Fs;
typedef Img<Vec3F>          ImgV3F;      // RGB [0,1] unless otherwise noted
typedef Svec<ImgV3F>        ImgV3Fs;

typedef Img<Arr4UC>         Img4UC;
typedef Svec<Img4UC>        Img4UCs;
typedef Img<Arr4F>          Img4F;
typedef Img<Arr4D>          Img4D;
typedef Img<Vec4F>          ImgV4F;

typedef Img<Rgba8>          ImgRgba8;
typedef Svec<ImgRgba8>      ImgRgba8s;
typedef Svec<ImgRgba8s>     ImgRgba8ss;
typedef Img<Rgba16>         ImgRgba16;
typedef Svec<ImgRgba16>     ImgRgba16s;
typedef Img<RgbaF>          ImgC4F;

// if the dimensions of 2 images are colinear, they have the same aspect ratio:
inline bool         areColinear(Vec2UI dims0,Vec2UI dims1) {return (dims0[0]*dims1[1] == dims0[1]*dims1[0]); }

template<class T>
std::ostream &      operator<<(std::ostream & os,Img<T> const & img)
{
    return
        os << "dimensions: " << img.dims()
            << " bounds: " << cBounds(img.m_data);
}
std::ostream &      operator<<(std::ostream &,ImgRgba8 const &);
std::ostream &      operator<<(std::ostream &,ImgC4F const &);

template<typename T,typename C>
Img<T>              genImg(
    Vec2UI              dims,           // [width,height]
    C const &           callable,       // (size_t x,size_t y) -> T over [x,y] < dims. Must be re-entrant if multithreaded.
    bool                multithread)    // set to false if calling from multithreaded context
{
    Img<T>              ret {dims};
    size_t              X = dims[0],
                        Y = dims[1];
    auto                fn = [&callable,&ret,X](size_t ylo,size_t yeub)
    {
        for (size_t yy=ylo; yy<yeub; ++yy)
            for (size_t xx=0; xx<X; ++xx)
                ret.xy(xx,yy) = callable(xx,yy);
    };
    if (multithread) {
        size_t              nt = cMin(std::thread::hardware_concurrency(),Y);
        Svec<std::thread>   threads; threads.reserve(nt);
        for (size_t tt=0; tt<nt; ++tt) {
            size_t              ylo = (tt * Y) / nt,
                                yeub = ((tt+1) * Y) / nt;
            threads.emplace_back(fn,ylo,yeub);
        }
        for (std::thread & thread : threads)
            thread.join();
    }
    else
        fn(0,Y);
    return ret;
}

// SAMPLING & INTERPOLATION:
 
// linear interpolation of equispaced values with zero-weighting outside bounds
struct      ImgLerp
{
    int             lo;             // lower tap index. May be out of bounds. Upper tap is lo+1. May be out of bounds.
    Arr2F           wgts {0};       // Always >= 0. Only non-zero for in-bounds taps.

    // The coordinate must be given in image raster coordinates (IRCS) since this is the natural unit for interpolation
    // (the sample point being considered as the pixel centre). For PACS coordinates, just convert by adding 0.5 to each:
    ImgLerp(float ircs,size_t numPixels);
};

template<typename T>
struct      ValWgt
{
    T               wval;       // weighted value
    float           wgt;        // [0,1] sampling weight of val
};

// bilinear interpolation of an image with bounds-aware weighting:
struct      ImgBlerp
{
    ImgLerp         xLerp,
                    yLerp;

    ImgBlerp(Vec2F ircs,Vec2UI dims) : xLerp{ircs[0],dims[0]}, yLerp{ircs[1],dims[1]} {}

    // sample with zero boundary policy (ie all pixels outside image implicitly 0):
    template<typename T>
    T               sampleZero(Img<T> const & img) const
    {
        T                   acc {0};
        for (int yy=0; yy<2; ++yy) {
            for (int xx=0; xx<2; ++xx) {
                float           w = xLerp.wgts[xx] * yLerp.wgts[yy];
                if (w > 0)
                    acc += img.xy(xLerp.lo+xx,yLerp.lo+yy) * w;
            }
        }
        return acc;
    }
    // fixed-point version of above:
    template<typename T>
    T               sampleZeroFixed(Img<T> const & img) const
    {
        typedef typename Traits<T>::Floating F;
        F                   acc {0};
        for (int yy=0; yy<2; ++yy) {
            for (int xx=0; xx<2; ++xx) {
                float           w = xLerp.wgts[xx] * yLerp.wgts[yy];
                if (w > 0)
                    acc += F(img.xy(xLerp.lo+xx,yLerp.lo+yy)) * w;
            }
        }
        return T(acc);
    }
};

template<typename T>
T                   sampleBlerpZero(Img<T> const & img,Vec2F ircs) {return ImgBlerp{ircs,img.dims()}.sampleZero(img); }

struct      CoordWgt
{
    Vec2UI      coordIrcs;  // Image coordinate
    double      wgt;        // Respective weight
};

// Calcualte bilinear interpolation coefficients and coordinates clamped within image boundaries.
// Returned matrix weights sum to 1. Cols are X [lo,hi] and rows are Y [lo,hi].
Mat<CoordWgt,2,2>   cBlerpClampIrcs(Vec2UI dims,Vec2D ircs);
Mat<CoordWgt,2,2>   cBlerpClampIucs(Vec2UI dims,Vec2D iucs);

// Sample an image with given matrix of coordinates (must be in image bounds) and weights
template<typename T>
typename Traits<T>::Floating sampleClip(Img<T> const & img,Mat<CoordWgt,2,2> const & cws)
{
    typedef typename Traits<T>::Floating    Acc;
    Acc                 ret(0);
    for (CoordWgt const & cw : cws.m)
        ret += Acc(img[cw.coordIrcs]) * cw.wgt;
    return ret;
}

// Bilinear image sample clamped to image bounds:
template<typename T>
typename Traits<T>::Floating sampleClampPacs(Img<T> const & img,Vec2D pacs)
{
    Vec2D               ircs {pacs[0]-0.5,pacs[1]-0.5};
    return sampleClip(img,cBlerpClampIrcs(img.dims(),ircs));
}
// Bilinear image sample clamped to image bounds:
template<typename T>
typename Traits<T>::Floating sampleClampIucs(Img<T> const & img,Vec2F coordIucs)
{
    return sampleClip(img,cBlerpClampIucs(img.dims(),Vec2D{coordIucs}));
}
// Bilinear image sample clamped to image bounds:
template<typename T>
typename Traits<T>::Floating sampleClipIrcs(Img<T> const & img,Vec2D coordIrcs)
{
    return sampleClip(img,cBlerpClampIrcs(img.dims(),coordIrcs));
}

// ELEMENT-WISE OPERATIONS:

template<class T,class F>
auto                mapCall(Img<T> const & i,F f)
{
    typedef decltype(f(i.m_data[0]))   R;
    return Img<R>{i.dims(),mapCall(i.m_data,f)};
}
template<class T,class U,class F>
auto                mapCall(Img<T> const & l,Img<U> const & r,F f)
{
    typedef decltype(f(l.m_data[0],r.m_data[0]))    R;
    FGASSERT(l.dims() == r.dims());
    return Img<R>{l.dims(),mapCall(l.m_data,r.m_data,f)};
}
template<class T,class U,class V,class F>
auto                mapCall(Img<T> const & l,Img<U> const & m,Img<V> const & r,F f)
{
    typedef decltype(f(l.m_data[0],m.m_data[0],r.m_data[0]))    R;
    FGASSERT(l.dims() == m.dims());
    FGASSERT(l.dims() == r.dims());
    return Img<R>{l.dims(),mapCall(l.m_data,m.m_data,r.m_data,f)};
}

// map pow() with fixed exponent (for gamma encode/decode) to a RGB color with channel bounds [0,1]:
template<class T,FG_ENABLE_IF(T,is_floating_point)>
Arr<T,3>            mapPow(Arr<T,3> const & a,T gamma)
{
    return {std::pow(a[0],gamma),std::pow(a[1],gamma),std::pow(a[2],gamma),};
}
// Map pow() with fixed exponent (for gamma encode/decode) to a PMA color, with alpha in a[3] and channel bounds [0,1]
// Color values MUST be >= 0 or NAN values will result from pow():
template<class T,FG_ENABLE_IF(T,is_floating_point)>
Arr<T,4>            mapPowPma(Arr<T,4> const & a,T gamma)
{
    T                   alpha = a[3],
                        alpha1mg = std::pow(alpha,1-gamma);
    Arr<T,4>            ret;
    for (size_t ii=0; ii<3; ++ii)
        // (p/a)^g * a = p^g * a^(1-g)  is more numerically stable and handles alpha=0
        ret[ii] = std::pow(a[ii],gamma) * alpha1mg;
    ret[3] = alpha;
    return ret;
}
template<class T>
Rgba<T>             mapPowPma(Rgba<T> const & a,T gamma) {return Rgba<T>{mapPowPma(a.m_c,gamma)}; }
template<class T>
void                mapGamma_(Img<Arr<T,3>> & img,T gamma) {for (Arr<T,3> & e : img.m_data) e = mapPow(e,gamma); }
template<class T>
void                mapGamma_(Img<Arr<T,4>> & imgPma,T gamma) {for (Arr<T,4> & e : imgPma.m_data) e = mapPowPma(e,gamma); }
template<class T>
Img<Rgba<T>>        mapGamma(Img<Rgba<T>> const & imgPma,T gamma)
{
    return mapCall(imgPma,[gamma](Rgba<T> const & e){return mapPowPma(e,gamma); });
}
template<class T>
Img<Arr<T,3>>       mapGamma(Img<Arr<T,3>> const & img,T gamma)
{
    return mapCall(img,[gamma](Arr<T,3> const & e){return mapPow(e,gamma); });
}
template<class T>
Img<Arr<T,4>>       mapGamma(Img<Arr<T,4>> const & imgPma,T gamma)  // PMA image with alpha in last channel
{
    return mapCall(imgPma,[gamma](Arr<T,4> const & e){return mapPowPma(e,gamma); });
}

// Conversions to Rgba8
template<class T,FG_ENABLE_IF(T,is_floating_point)>     // [0,1) -> [0,255] with clamping:
inline uchar        toUchar(T flt) {return scast<uchar>(clamp<T>(flt*256,0,255)); }
template<class T,FG_ENABLE_IF(T,is_floating_point)>
inline Rgba8        toRgba8(T v) {uchar c = toUchar<T>(v); return Rgba8{c,c,c,255}; }
template<class T,FG_ENABLE_IF(T,is_floating_point)>
inline Rgba8        toRgba8(Arr<T,3> const & a) {return Rgba8{toUchar(a[0]),toUchar(a[1]),toUchar(a[2]),255}; }
template<class T,FG_ENABLE_IF(T,is_floating_point)>
inline Rgba8        toRgba8(Arr<T,4> const & a) {return Rgba8{mapCall(a,[](T v){return toUchar(v);})}; }
template<class T,FG_ENABLE_IF(T,is_floating_point)>
inline Rgba8        toRgba8(Rgba<T> const & a) {return toRgba8(a.m_c); }
template<class T>
ImgRgba8            toRgba8(Img<T> const & img) {return mapCall(img,[](T const & p){return toRgba8(p); }); }
template<class T,FG_ENABLE_IF(T,is_floating_point)>
ImgRgba8            toRgba8Gamma(Img<Arr<T,3>> const & img,T g=1/T(2.2))
{
    auto                fn = [g](Arr<T,3> v)
    {
        Arr3UC              c = mapCast<uchar>(mapPow(v,g)*255.99f);
        return Rgba8{c[0],c[1],c[2],255};
    };
    return mapCall(img,fn);
}
// Combine gamma with Rgba8 conversion since it's so commonly used together. Input must be PMA with channel values [0,1]:
template<class T,FG_ENABLE_IF(T,is_floating_point)>
inline Rgba8        toRgba8Gamma(Arr<T,4> const & pma,T gamma)
{
    return Rgba8{mapCall(mapPowPma(pma,gamma),[](T v){return toUchar(v);})};
}
template<class T,FG_ENABLE_IF(T,is_floating_point)>
inline ImgRgba8     toRgba8Gamma(Img<Arr<T,4>> const & pma,T gamma=1/2.2)
{
    return mapCall(pma,[gamma](Arr<T,4> const & p){return toRgba8Gamma(p,gamma); });
}

// Conversions. Note that the value 255U is only used for 1.0f so the effective 8-bit range is only 255 values.
// This is done to allow preservation of max value in round-trip conversions:
Img3F               toUnit3F(ImgRgba8 const &);         // [0,255] -> [0,1], ignores input alpha channel
Img4F               toUnit4F(ImgRgba8 const &);         // [0,255] -> [0,1]
Img4D               toUnit4D(ImgRgba8 const &);         // [0,255] -> [0,1]
ImgC4F              toUnitC4F(ImgRgba8 const &);        // [0,255] -> [0,1]
ImgUC               toUC(ImgRgba8 const &);             // rec. 709 RGB -> greyscale
ImgF                toFloat(ImgRgba8 const &);          // rec. 709 RGB -> greyscale [0,255]
ImgRgba8            toRgba8(ImgUC const &);             // replicate to RGB, set alpha to 255
Img4F               toApm(Img4F const &);               // convert from independent RGBA to alpha-premultiplied RGBA

template<class T,class U>
void                operator*=(Img<T> & img,U rhs) {img.m_data *= rhs; }

template<class T>
double              cDot(Img<T> const & lhs,Img<T> const & rhs)
{
    FGASSERT(lhs.dims() == rhs.dims());
    return cDot(lhs.m_data,rhs.m_data);
}

template<class T>
double              cSsd(Img<T> const & lhs,Img<T> const & rhs)
{
    FGASSERT(lhs.dims() == rhs.dims());
    return cSsd(lhs.m_data,rhs.m_data);
}

template<class T,class U>
inline Img<U>       mapMul(T op,Img<U> const & img) {return Img<U>{img.dims(),mapMul(op,img.m_data)}; }

template<class T,class F>
inline Img<T>       mapCast(Img<F> const & img) {return Img<T>{img.dims(),mapCast<T,F>(img.m_data)}; }

template<class T>
bool                isApproxEqual(Img<T> const & l,Img<T> const & r,typename Traits<T>::Scalar maxDiff)
{
    FGASSERT(l.dims() == r.dims());
    return isApproxEqual(l.m_data,r.m_data,maxDiff);
}

// Interpolate between 2 images:
template<class T>
Img<T>              interpolate(Img<T> const & i0,Img<T> const & i1,float ratio)
{
    Vec2UI          dims = i0.dims();
    FGASSERT(i1.dims() == dims);
    Img<T>          ret(dims);
    float           omv = 1.0f - ratio;
    for (size_t ii=0; ii<ret.numPixels(); ++ii) {
        auto        v = mapCast<float>(i0.m_data[ii]) * omv + mapCast<float>(i1.m_data[ii]) * ratio;
        mapRound_(v,ret.m_data[ii]);
    }
    return ret;
}

// RESIZE / TRANSFORM / CONVOLVE:

// Subsample image 2x with 2x2 box filter, rounding down the image size when odd.
// Channels must be linear (eg. alpha-weighted if alpha exists):

// accumulator image version; channel values must be at least 4x smaller than type max:
template<class T,FG_ENABLE_IF(typename Traits<T>::Scalar,is_integral)>
Img<T>              shrink2Acc(Img<T> const & img)
{
    size_t              X = img.width()/2,
                        Y = img.height()/2;
    FGASSERT(X*Y>0);
    Img<T>              ret {X,Y};
    for (size_t yy=0; yy<Y; ++yy) {
        T const             *imgPtr0 = img.rowPtr(yy*2),
                            *imgPtr1 = img.rowPtr(yy*2+1);
        T                   *retPtr = ret.rowPtr(yy);
        for (size_t xx=0; xx<X; ++xx) {
            size_t              xxi = xx*2;
            T                   acc = imgPtr0[xxi] + imgPtr0[xxi+1] + imgPtr1[xxi] + imgPtr1[xxi+1];
            retPtr[xx] = (acc + T(2)) / 4;
        }
    }
    return ret;
}

// RGBA version:
void                shrink2_(ImgRgba8 const & src,ImgRgba8 & dst);
inline ImgRgba8     shrink2(ImgRgba8 const & src)
{
    ImgRgba8             ret;
    shrink2_(src,ret);
    return ret;
}
// floating point versions:
template<typename T,
    // Floating point specialization doesn't use a different accumulator type:
    FG_ENABLE_IF(typename Traits<T>::Scalar,is_floating_point)>
void                shrink2_(
    Img<T> const &              src,
    Img<T> &                    dst,    // Must be a different instance
    typename Traits<T>::Scalar  denom=typename Traits<T>::Scalar(4))
{
    FGASSERT(src.dataPtr() != dst.dataPtr());
    dst.resize(src.dims()/2);
    for (uint yy=0; yy<dst.height(); ++yy)
        for (uint xx=0; xx<dst.width(); ++xx)
            dst.xy(xx,yy) = (
                src.xy(2*xx,2*yy) +
                src.xy(2*xx+1,2*yy) +
                src.xy(2*xx+1,2*yy+1) +
                src.xy(2*xx,2*yy+1)) / denom;
}
template<class T,FG_ENABLE_IF(typename Traits<T>::Scalar,is_floating_point)>
Img<T>              shrink2(Img<T> const & img)
{
    Img<T>                  ret;
    shrink2_(img,ret);
    return ret;
}
// uchar version:
ImgUC               shrink2Fixed(const ImgUC & img);
// Resampled 2x expansion. See 'magnify' for a unresampled.
ImgRgba8            expand2(ImgRgba8 const & src);

template<class T>
Img<T>              magnify(Img<T> const & img,uint fac)
{
    Img<T>                  ret;
    ret.m_dims = img.dims() * fac;
    ret.m_data.reserve(img.numPixels() * fac);
    for (uint yy=0; yy<img.height(); ++yy)
        for (uint ff=0; ff<fac; ++ff)
            for (uint xx=0; xx<img.width(); ++xx)
                for (uint gg=0; gg<fac; ++gg)
                    ret.m_data.push_back(img.xy(xx,yy));
    return ret;
}

// Resize to the destination image dimensions, by shrinking or expanding in each dimension.
// Samples the exact proportional amount of the source image covered by the destination image
// pixel for shrinking dimensions, which effectively uses bilinear interpolation for dilating
// dimensions:
void                imgResize_(ImgRgba8 const & src,ImgRgba8 & dst);

ImgRgba8            applyTransparencyPow2(
    ImgRgba8 const & colour,
    ImgRgba8 const & transparency);

Img<FatBool>        mapAnd(const Img<FatBool> &,const Img<FatBool> &);

template<class T>
void                flipVertical_(Img<T> & img)
{
    uint    wid = img.width(),
            halfHgt = img.height()/2;
    for (uint row=0; row<halfHgt; row++) {
        T * ptr1 = img.rowPtr(row);
        T * ptr2 = img.rowPtr(img.height()-1-row);
        for (uint col=0; col<wid; col++)
            std::swap(ptr1[col],ptr2[col]);
    }
}

template<class T>
Img<T>              flipHoriz(Img<T> const & img)
{
    Img<T>  ret(img.dims());
    uint        xh = img.width()-1;
    for (uint yy=0; yy<img.height(); ++yy) {
        for (uint xx=0; xx<img.width(); ++xx)
            ret.xy(xh-xx,yy) = img.xy(xx,yy); }
    return ret;
}

// Beyond image border, use zero values or mirror border values for convolution ?
enum struct         BorderPolicy {zero,mirror};
template<BorderPolicy B> inline uchar borderFac();
template<> inline uchar borderFac<BorderPolicy::zero>() {return 0; }
template<> inline uchar borderFac<BorderPolicy::mirror>() {return 1; }

// 1D smooth on accumulator type (T must be larger than values by a factor of at least 4).
// Does not renormalize 4x filter sum.
template<BorderPolicy B,class T,
    FG_ENABLE_IF(typename Traits<T>::Scalar,is_integral)>
void                smoothAcc1D_(T const *srcPtr,T *dstPtr,size_t sz)
{
    FGASSERT(sz > 1);
    dstPtr[0] = srcPtr[0] * (2+borderFac<B>()) + srcPtr[1];
    for (size_t ii=1; ii<sz-1; ++ii)
        dstPtr[ii] = srcPtr[ii-1] + srcPtr[ii] * 2 + srcPtr[ii+1];
    dstPtr[sz-1] = srcPtr[sz-2] + srcPtr[sz-1] * (2+borderFac<B>());
}
// Image smooth applies a [1 2 1] outer product 2D kernel to an integer channel image in a
// cache-friendly way. The channel type must be an accumulator type; it must accomodate
// values at least 16x larger than the maximum.
// The Source and destination images can be the same, for in-place convolution.
// Image dimensions must be at least 2.
template<BorderPolicy B,class T,
    FG_ENABLE_IF(typename Traits<T>::Scalar,is_integral)>
void                smoothAcc_(Img<T> const & src,Img<T> & dst)
{
    size_t              X = src.width(),
                        Y = src.height();
    FGASSERT((X>1) && (Y>1));
    dst.resize(src.dims());
    Img<T>              acc {X,3};                  // no need to zero-initialize, not used
    T                   *acc0 = acc.rowPtr(0),
                        *acc1 = acc.rowPtr(1),
                        *acc2 = acc.rowPtr(2);
    smoothAcc1D_<B,T>(src.rowPtr(0),acc0,X);
    smoothAcc1D_<B,T>(src.rowPtr(1),acc1,X);
    T                   *dstPtr = dst.rowPtr(0);
    for (size_t xx=0; xx<X; ++xx)
        dstPtr[xx] = (acc0[xx]*(2+borderFac<B>()) + acc1[xx] + T(7)) / 16;
    for (size_t yy=1; yy+1<Y; ++yy) {
        acc0 = acc.rowPtr((yy+2)%3),
        acc1 = acc.rowPtr(yy%3),
        acc2 = acc.rowPtr((yy+1)%3);
        smoothAcc1D_<B,T>(src.rowPtr(yy+1),acc2,X);
        dstPtr = dst.rowPtr(yy);
        for (size_t xx=0; xx<X; ++xx)
            // round by adding 7 before divide / truncate (downward bias but 8 would be upward):
            dstPtr[xx] = (acc0[xx] + acc1[xx]*2 + acc2[xx] + T(7)) / 16;
    }
    dstPtr = dst.rowPtr(Y-1);
    for (size_t xx=0; xx<X; ++xx)
        dstPtr[xx] = (acc2[xx]*(2+borderFac<B>()) + acc1[xx] + T(7)) / 16;
}
// Functional version of above:
template<BorderPolicy B,class T,
    FG_ENABLE_IF(typename Traits<T>::Scalar,is_integral)>
Img<T>              smoothAcc(Img<T> const & img)
{
    Img<T>              ret;
    smoothAcc_<B,T>(img,ret);
    return ret;
}

// DEPRECATED: smoothing should be done on accumulator channel type images:
void                smoothUint_(ImgUC const & src,ImgUC & dst,uchar borderPolicy=1);
ImgUC               smoothUint(ImgUC const & src,uchar borderPolicy=1);
void                smoothUint_(ImgRgba8 const & src,ImgRgba8 & dst,uchar borderPolicy=1);
ImgRgba8            smoothUint(ImgRgba8 const & src,uchar borderPolicy=1);

// Use of the __restrict keyword for the pointer args below made no speed difference here (msvc2012).
// Perhaps the compiler is smart enough to look at the calling context:
template<class T>
void                smoothFloat1D_(
    T const *           srcPtr,
    T *                 dstPtr,         // Must not overlap with srcPtr
    size_t              wid,
    uchar               borderPolicy)   // See below
{
    typedef typename Traits<T>::Scalar   Scalar;
    dstPtr[0] = srcPtr[0]*Scalar(2+borderPolicy) + srcPtr[1];
    for (size_t ii=1; ii<wid-1; ++ii)
        dstPtr[ii] = srcPtr[ii-1] + srcPtr[ii]*Scalar(2) + srcPtr[ii+1];
    dstPtr[wid-1] = srcPtr[wid-2] + srcPtr[wid-1]*Scalar(2+borderPolicy);
}
template<class T>
void                smoothFloat2D_(
    T const *           srcPtr,
    T *                 dstPtr,         // can point to same image as srcPtr
    size_t              wid,
    size_t              hgt,
    uchar               borderPolicy)
{
    typedef typename Traits<T>::Scalar   Scalar;
    Scalar constexpr    factor = 0.25 * 0.25;
    Img<T>              acc {wid,3};
    T                   *accPtr0,
                        *accPtr1 = acc.rowPtr(0),
                        *accPtr2 = acc.rowPtr(1);
    smoothFloat1D_(srcPtr,accPtr1,wid,borderPolicy);
    srcPtr += wid;
    smoothFloat1D_(srcPtr,accPtr2,wid,borderPolicy);
    for (size_t xx=0; xx<wid; ++xx)
        dstPtr[xx] = (accPtr1[xx]*Scalar(2+borderPolicy) + accPtr2[xx]) * factor;
    for (size_t yy=1; yy<hgt-1; ++yy) {
        dstPtr += wid;
        srcPtr += wid;
        accPtr0 = acc.rowPtr((yy-1)%3);
        accPtr1 = acc.rowPtr(yy%3);
        accPtr2 = acc.rowPtr((yy+1)%3);
        smoothFloat1D_(srcPtr,accPtr2,wid,borderPolicy);
        for (size_t xx=0; xx<wid; ++xx)
            dstPtr[xx] = (accPtr0[xx] + accPtr1[xx] * Scalar(2) + accPtr2[xx]) * factor;
    }
    dstPtr += wid;
    for (size_t xx=0; xx<wid; ++xx)
        dstPtr[xx] = (accPtr1[xx] + accPtr2[xx]*Scalar(2+borderPolicy)) * factor;
}
// Applies a [1 2 1] outer product 2D kernel smoothing to a floating point channel
// image in a cache-friendly way.
// The Source and destination images can be the same, for in-place convolution.
template<class T>
void                smoothFloat_(
    Img<T> const &  src,
    Img<T> &        dst,                // Returned. Can be same object as src
    uchar           borderPolicy)       // 0 - zero border policy, 1 - replication border policy
{
    FGASSERT((src.width() > 1) && (src.height() > 1));  // Algorithm not designed for dim < 2
    FGASSERT((borderPolicy == 0) || (borderPolicy == 1));
    dst.resize(src.dims());
    smoothFloat2D_(src.dataPtr(),dst.dataPtr(),src.width(),src.height(),borderPolicy);
}

// floating-point channel smooth with implicit zero border policy:
template<class T>
Img<T>              smoothF(Img<T> const & img)
{
    Img<T>              ret {img.dims()};
    smoothFloat_(img,ret,0);
    return ret;
}
// and 1D smooth:
template<class T>
Svec<T>             smoothF(Svec<T> const & img,uchar borderPolicy)
{
    Svec<T>             ret (img.size());
    smoothFloat1D_(img.data(),ret.data(),img.size(),borderPolicy);
    return ret*0.25;
}

// Preserves intrinsic aspect ratio, scales to minimally cover output dimensions.
// Returns transform from output image IRCS to input image IRCS (ie. inverse transform for resampling):
AxAffine2D          imgScaleToCover(Vec2UI inDims,Vec2UI outDims);

// Square resample with implicit zero outside boundary.
// Region and return sizes must be > 0 but region does not have to overlap image:
Img4F               resample(Img4F const & img,SquareF regionPacs,uint retSize,bool multithread);

// Resample an image to the given size with the given inverse transform with boundary clip policy:
template<class T>
Img<T>              resampleClip(Img<T> const & in,Vec2UI outDims,AxAffine2D outToInIrcs)
{
    Img<T>              ret {outDims};
    if (outDims.elemsProduct() == 0)
        return ret;
    FGASSERT(in.dims().elemsProduct() > 0);
    for (Iter2UI it(outDims); it.valid(); it.next()) {
        Vec2D           inIrcs = outToInIrcs * Vec2D{it()};
        mapRound_(sampleClipIrcs(in,inIrcs),ret[it()]);
    }
    return ret;
}

// Note that the image will be distorted if aspect ratio changes:
template<class T>
Img<T>              scaleResample(Img<T> const & in,Vec2UI dims)
{
    auto                fn = [](uint i,uint d)
    {
        FGASSERT(i>0);
        return scast<double>(d)/scast<double>(i);
    };
    Vec2D               scales = mapCall(in.dims(),dims,fn);
    AxAffine2D          xf = AxAffine2D{scales,Vec2D{-0.5}} * cIrcsToPacs<double,2>();
    return resampleClip(in,dims,xf);
}

template<class T>
Img<T>              resampleAffine(Img<T> const & in,Vec2UI dims,AxAffine2D const & outToInIrcs)
{
    typedef typename Traits<T>::Scalar  S;
    Img<T>                  ret {dims};
    for (Iter2UI it {dims}; it.valid(); it.next()) {
        Vec2D               inIrcs = outToInIrcs * Vec2D(it());
        auto                lerpv = cBlerpClampIrcs(in.dims(),inIrcs);
        T                   p {0};
        for (uint ii=0; ii<4; ++ii) {
            CoordWgt const &    cw = lerpv[ii];
            p += in[cw.coordIrcs] * scast<S>(cw.wgt);
        }
        ret[it()] = p;
    }
    return ret;
}
// Resamples a square area of an input image into the given pixel size.
// If necessary, filters first to avoid aliasing.
// Out of bounds samples clamped to boundary values:
template<class T>
Img<T>              filterResample(
    Img<T>                  in,
    Vec2D                   posPacs,        // top left corner of output image area
    double                  inSize,         // pixel size of square input domain
    uint                    outSize)        // pixel size of square output image
{
    FGASSERT(!in.empty());
    FGASSERT(inSize > 0);
    FGASSERT(outSize > 0);
    for (uint dd=0; dd<2; ++dd) {                   // Ensure overlap between 'in' and selected region:
        FGASSERT(posPacs[dd] < in.dims()[dd]);
        FGASSERT(posPacs[dd]+inSize > 0);
    }
    // Reduce the input image to avoid undersampling. 1 1/3 undersampling gives minimum contribution of 2/3
    // pixel value (max always 1). 1 1/2 undersampling gives minimum representation of 1/2:
    while (inSize / outSize > 1.3333) {
        in = shrink2(in);
        posPacs *= 0.5;
        inSize *= 0.5;
    }
    return resampleAffine(in,Vec2UI{outSize},AxAffine2D{Vec2D{inSize/outSize},posPacs});
}

ImgRgba8            filterResample(ImgRgba8 in,Vec2F loPacs,float inSize,uint outSize);

// Block resample ensures that all source image pixel values within the specified region have equal net
// contribution to the output image. This provides an optimal resampling when the output image is pointwise
// undersampled. this should not be used when the output image is oversampled as it does not interpolate.
// Use regular pointwise interpolation resampling in that case. Implicit zero values outside image bounds.
Img4F               blockResample(
    Img4F const &       src,                // must have linear alpha-premultiplied color channels
    SquareF             regionPacs,         // the square region of the image to be block resampled
    uint                retSize,            // the return image pixel size
    bool                mt);

// choose either bilinear resample or block resample depending on over or under sampling resp.
inline Img4F        adaptResample(Img4F const & img,SquareF regionPacs,uint sz,bool mt)
{
    if (regionPacs.size > sz)
        return blockResample(img,regionPacs,sz,mt);     // undersampling
    else
        return resample(img,regionPacs,sz,mt);          // oversampling
}

template <class T>
Img<T>              catH(Img<T> const & l,Img<T> const & r)
{
    size_t              H = l.height(),
                        W = l.width() + r.width();
    FGASSERT(r.height() == H);
    auto                fn = [&,H,W]()
    {
        Svec<T>             ret(H*W);
        T *                 retPtr = ret.data();
        for (size_t rr=0; rr<H; ++rr) {
            retPtr = copy_(l.rowPtr(rr),retPtr,l.width());
            retPtr = copy_(r.rowPtr(rr),retPtr,r.width());
        }
        return ret;
    };
    return Img<T>{W,H,fn()};
}
template<class T>
Img<T>              catV(Img<T> const & u,Img<T> const & l)
{
    size_t              H = u.height() + l.height(),
                        W = u.width();
    FGASSERT(l.width() == W);
    return Img<T>{W,H,cat(u.m_data,l.m_data)};
}
template<class T>
Img<T>              catV(Svec<Img<T>> const & imgs)
{
    size_t              H {0},
                        W = imgs.empty() ? 0 : imgs[0].width();
    for (Img<T> const & img : imgs) {
        H += img.height();
        FGASSERT(img.width() == W);
    }
    auto                fn = [&,H,W]()
    {
        Svec<T>             ret; ret.reserve(H*W);
        for (Img<T> const & img : imgs)
            cat_(ret,img.m_data);
        return ret;
    };
    return Img<T>{W,H,fn()};
}

template<class T>
Img<T>              cropPad(
    Img<T> const &      src,
    Vec2UI              dims,                   // Output image dimensions in pixels
    Vec2I               offset = Vec2I{0},      // Lower corner of output image in IRCS
    T                   fill = T{0})            // Value to use if region is not contained in 'src'
{
    Img<T>              ret {dims,fill};
    if (!src.empty()) {
        Vec2I               srcIlb = mapMax(offset,Vec2I{0,0}),
                            srcEub = mapMin(offset+Vec2I{dims},Vec2I{src.dims()}),
                            srcSz = srcEub - srcIlb;
        if (allGtZero(srcSz.m)) {
            for (Iter2I it(srcIlb,srcEub); it.valid(); it.next())
                ret[Vec2UI{it()}] = src[Vec2UI{it()+offset}];
        }
    }
    return ret;
}

// RGB values should not be weighted by alpha and image dimensions must be equal:
ImgRgba8            composite(ImgRgba8 const & foreground,ImgRgba8 const & background);

// direct resample using a resample map (no scaling). Undefined pixels have value {0,0,0,0}:
ImgRgba8            resampleMap(
    Img2F const &       map,        // resample coordinates in OTCS, with (-1,-1) as invalid
    ImgRgba8 const &    src);
ImgUC               resampleMap(Img2F const & map,ImgUC const & src);

template<class T>
void                rotate90(bool clockwise,Img<T> const & in,Img<T> & out)
{
    Vec2UI          dims(in.dims());
    std::swap(dims[0],dims[1]);
    out.resize(dims);
    int             xh = (clockwise ? dims[0]-1 : 0),
                    yh = (clockwise ? 0 : dims[1]-1),
                    xs = (clockwise ? -1 : 1),
                    ys = (clockwise ? 1 : -1);
    for (uint yy=0; yy<dims[1]; ++yy)
        for (uint xx=0; xx<dims[0]; ++xx)
            out.xy(xx,yy) = in.xy(yh+ys*yy,xh+xs*xx);
}

// Does any pixel contain an alpha value less than 254 ? (returns false if empty)
bool                usesAlpha(ImgRgba8 const &,uchar minVal=254);

// Thickness must be an odd number:
void                paintCrosshair(ImgRgba8 & img,Vec2I ircs);
void                paintDot(ImgRgba8 & img,Vec2I ircs,Rgba8 color={255,0,0,255},uint radius=3);
void                paintDot(ImgRgba8 & img,Vec2F pacs,Rgba8 color={255,0,0,255},uint radius=3);

// Create a mipmap (2-box-filtered 2-subsamples image pyrmamid), in which:
// * The first element is the original image
// * Each subsequent element is half the dimensions (rounded down for odd parent dimensions)
// * The last element is when the smaller dimension size has reached 1
// * If the input image is empty, an empty pyramid will be returned
template<class T>
Svec<Img<T>>        cMipmap(Img<T> const & img)
{
    Svec<Img<T>>            ret;
    uint                    minDim = cMinElem(img.dims());
    if (minDim == 0)
        return ret;
    ret.reserve(log2Floor(minDim) + 1);
    ret.push_back(img);
    while (cMinElem(ret.back().dims()) > 1)
        ret.push_back(shrink2(ret.back()));
    return ret;
}
// integral accumulator channel type version:
template<class T,FG_ENABLE_IF(typename Traits<T>::Scalar,is_integral)>
Svec<Img<T>>        cMipmapA(Img<T> const & img)
{
    Svec<Img<T>>            ret;
    uint                    minDim = cMinElem(img.dims());
    if (minDim == 0)
        return ret;
    ret.reserve(log2Floor(minDim) + 1);
    ret.push_back(img);
    while (cMinElem(ret.back().dims()) > 1)
        ret.push_back(shrink2Acc(ret.back()));
    return ret;
}

// extrapolates useful values for undefined pixels (alpha=0) to minimize UV seam visibility.
// Useful for auto-generated maps in which pixel values are not defined outside the UV layout.
// Defined pixels must have alpha=255 and undefined alpha=0. Color values must be alpha-weighted.
// The returned image will have all alpha=255.
// Uses fast, crude approach of taking first non-zero mipmap level and renormalizing alpha:
ImgRgba8            extrapolateForMipmap(ImgRgba8 const &);

// Blend images given a greyscale transition map [0,255] : 0 -> 'img0', 255 -> 'img1'
// The returned image is the size of 'img0' and 'img1' and 'transition' are bilinearly sampled.
// Output alpha is set to 255. Any of the images can be empty:
ImgRgba8            imgBlend(ImgRgba8 const & img0,ImgRgba8 const & img1,ImgUC const & transition);
// Modulate the color channels of an image with a modulation map scaled such that identity (1.0) = 64.
// Only the color channels of modulationMap are used. The input images may be different sizes but must
// have identical aspect ratios. Either image may be empty.
ImgRgba8            imgModulate(
    ImgRgba8 const &    colorImage,             // Alpha left unchanged
    ImgRgba8 const &    modulationMap,          // RGB channels modulate respective channels in 'colorImage'. Alpha ignored.
    float               modulationFactor=1.0f,  // [0.5,1.5] modulate the modulation values.
    bool                multithread=true);      // set false if calling within multithreaded context

template<class T>
Img<T>              outerProductImg(Svec<T> const & x,Svec<T> const & y)
{
    Img<T>              ret {x.size(),y.size()};
    for (Iter2 it{x.size(),y.size()}; it.valid(); it.next())
        ret[it()] = x[it.x()] * y[it.y()];
    return ret;
}

// visualize a matrix:
ImgRgba8            visualize(MatD const &);

}

#endif
