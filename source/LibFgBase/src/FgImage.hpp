//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Simple left-to-right, top-to-bottom (row major), tightly-packed, unaligned image templated by pixel type.
// 
// INVARIANTS:
//
// m_data.size() == m_dims[0] * m_dims[1];
//
// NOTES:
//
// * posIrcs = posPacs - 0.5 (thus floor(posPacs) rounds to nearest int posPacs)
// * posPacs = mapMul(posIucs,image.dims())
// * All pixel averaging / resampling operations on images with an alpha-channel require the color
//   channels to be alpha-premultiplied (APM) for correct results.
// * Images loaded from formats supporting alpha (ie. PNG) are not APM and must be converted.
// * APM should be avoided on 8-bit channels as it loses precision.

#ifndef FGIMAGE_HPP
#define FGIMAGE_HPP

#include "FgRgba.hpp"
#include "FgIter.hpp"
#include "FgGeometry.hpp"

namespace Fg {

// COORDINATE SYSTEMS:
//
// PACS := Pixel Area CS
//    Origin at lower corner (top left corner of image), units in pixels.
//    X - viewer�s right
//    Y - viewer�s down 
//
// IRCS := Image Raster CS
//    Origin at centre of first pixel in memory (top left), units are pixels, storage ordered by:
//    X - viewer�s right
//    Y - viewer�s down 
//
// IUCS := Image Unit CS
//    Origin at top left corner of image, (1,1) at bottom right corner.
//    X - viewer�s right
//    Y - viewer�s down 
//
// OTCS := OGL Texture CS
//    Origin at bottom left corner of image, (1,1) at top right
//    X - viewer�s right    [0,1]
//    Y - viewer�s up       [0,1]
//
AffineEw2D          cPacsToIucs(Vec2UI imgDims);
AffineEw2D          cIrcsToIucs(Vec2UI imgDims);
AffineEw2D          cIrcsToOtcs(Vec2UI imgDims);
AffineEw2D          cIucsToPacsXf(Vec2UI imgDims);
AffineEw2D          cIucsToIrcsXf(Vec2UI imgDims);
AffineEw2D          cOicsToIucsXf();
inline AffineEw2F   cOtcsToIucs() {return {Vec2F{1,-1},Vec2F{0,1}}; }   // flip Y axis
inline ScaleTrans2F cPacsToIrcs() {return {1,{-0.5f,-0.5f}}; }
inline ScaleTrans2F cIrcsToPacs() {return {1,{0.5f,0.5f}}; }

inline Vec2F        pacsToIrcs(Vec2F pacs) {return {pacs[0]-0.5f,pacs[1]-0.5f}; }
inline Vec2D        pacsToIrcs(Vec2D pacs) {return {pacs[0]-0.5, pacs[1]-0.5 }; }
inline Vec2F        ircsToPacs(Vec2F ircs) {return {ircs[0]+0.5f,ircs[1]+0.5f}; }
inline Vec2D        ircsToPacs(Vec2D ircs) {return {ircs[0]+0.5, ircs[1]+0.5 }; }

template<typename T>
struct      Img
{
    Vec2UI          m_dims;         // [width,height]
    Svec<T>         m_data;         // Pixels stored left to right, top to bottom. size() == m_dims[0]*m_dims[1]
    FG_SER2(m_dims,m_data)

    typedef T PixelType;

    Img() : m_dims(0) {}
    explicit Img(Vec2UI dims) : m_dims{dims}, m_data(dims[0]*dims[1]) {}
    Img(size_t wid,size_t hgt) : m_dims{uint(wid),uint(hgt)}, m_data(wid*hgt) {}
    Img(size_t wid,size_t hgt,T fill) : m_dims{uint(wid),uint(hgt)}, m_data(wid*hgt,fill) {}
    Img(size_t wid,size_t hgt,Svec<T> const & data) : m_dims{uint(wid),uint(hgt)}, m_data(data)
        {FGASSERT(data.size() == m_dims.cmpntsProduct()); }
    Img(Vec2UI dims,T fillVal) : m_dims{dims}, m_data(dims.cmpntsProduct(),fillVal) {}
    Img(Vec2UI dims,Svec<T> const & imgData) : m_dims{dims}, m_data{imgData}
        {FGASSERT(m_data.size() == m_dims.cmpntsProduct()); }
    Img(Vec2UI dims,T const * pixels) : m_dims{dims}, m_data(pixels,pixels+dims.cmpntsProduct()) {}

    uint            width() const {return m_dims[0]; }
    uint            height() const {return m_dims[1]; }
    Vec2UI          dims() const {return m_dims; }
    size_t          numPixels() const {return m_data.size(); }
    bool            empty() const {return (m_data.empty()); }

    void            clear() {m_data.clear(); m_dims = Vec2UI{0}; }
    // WARNING: This does not adjust any existing image data, just allocated dimensions and memory:
    void            resize(uint wid,uint hgt) {m_dims = Vec2UI{wid,hgt}; m_data.resize(wid*hgt); }
    void            resize(Vec2UI dims) {m_dims = dims; m_data.resize(dims[0]*dims[1]); }
    void            resize(Vec2UI dims,T fillVal) {resize(dims); std::fill(m_data.begin(),m_data.end(),fillVal); }

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
    T &             operator[](Vec2UI ircsPos) {return xy(ircsPos[0],ircsPos[1]); }
    T const &       operator[](Vec2UI ircsPos) const {return xy(ircsPos[0],ircsPos[1]); }

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
typedef Img<Vec4F>          ImgV4F;

typedef Img<Rgba8>          ImgRgba8;
typedef Svec<ImgRgba8>      ImgRgba8s;
typedef Svec<ImgRgba8s>     ImgRgba8ss;
typedef Img<Rgba16>         ImgRgba16;
typedef Svec<ImgRgba16>     ImgRgba16s;
typedef Img<RgbaF>          ImgC4F;

template<typename To,typename From>
void                scast_(Img<To> const & from,Img<From> & to)
{
    to.resize(from.dims());
    scast_(from.m_data,to.m_data);
}

inline size_t       cNumElems(Vec2UI dims) {return scast<size_t>(dims[0]) * scast<size_t>(dims[1]); }

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

// SAMPLING & INTERPOLATION:
 
// linear interpolation of equispaced values with bounds checking and bounds-aware weighting
struct      Lerp
{
    int             lo;             // lower tap index. May be out of bounds. Upper tap is lo+1. May be out of bounds.
    Arr2F           wgts {{0,0}};   // Always >= 0. Only non-zero for in-bounds taps.

    // The coordinate must be given in image raster coordinates (IRCS), in which the origin is at the sample point
    // (centre) of the 0 index pixel. The IRCS area bounds are then [-0.5 , numPixels-0.5]
    Lerp(float ircs,size_t numPixels);
};

template<typename T>
struct      ValWgt
{
    T           wval;       // weighted value
    float       wgt;        // [0,1] sampling weight of val
};

// bilinear interpolation of an image with bounds-aware weighting:
struct      Blerp
{
    Lerp            xLerp,
                    yLerp;

    Blerp(Vec2F ircs,Vec2UI dims) : xLerp{ircs[0],dims[0]}, yLerp{ircs[1],dims[1]} {}

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
T                   sampleBlerpZero(Img<T> const & img,Vec2F ircs) {return Blerp{ircs,img.dims()}.sampleZero(img); }

struct      CoordWgt
{
    Vec2UI      coordIrcs;  // Image coordinate
    float       wgt;        // Respective weight
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
    return sampleClip(img,cBlerpClampIrcs(img.dims(),pacsToIrcs(pacs)));
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

// Conversions. Note that the value 255U is only used for 1.0f so the effective 8-bit range is only 255 values.
// This is done to allow preservation of max value in round-trip conversions:
Img3F               toUnit3F(ImgRgba8 const &);         // [0,255] -> [0,1], ignores input alpha channel
Img4F               toUnit4F(ImgRgba8 const &);         // [0,255] -> [0,1]
ImgC4F              toUnitC4F(ImgRgba8 const &);        // [0,255] -> [0,1]
ImgRgba8            toRgba8(Img3F const &,float maxVal=1.0);    // [0,maxVal] -> [0,255], alpha set to 255
ImgRgba8            toRgba8(ImgC4F const &);            // [0,1] -> [0,255] with clamping
ImgRgba8            toRgba8(Img4F const &);             // [0,1] -> [0,255] with clamping
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

template<typename T,typename C>
Img<T>              generateImg(
    Vec2UI              dims,           // [width,height]
    C const &           callable,       // (size_t x,size_t y) -> T over [x,y] < dims
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

template<typename T,typename Fn>
Img<T>              mapCall(Img<T> const & in,Fn const & fn)
{
    return Img<T>{in.dims(),mapCall(in.m_data,fn)};
}
template<typename T,typename Fn>
Img<T>              mapCall(Img<T> const & l,Img<T> const & r,Fn const & fn)
{
    FGASSERT(l.dims() == r.dims());
    return Img<T>{l.dims(),mapCall(l.m_data,r.m_data,fn)};
}
template<typename T,typename Fn>
Img<T>              mapCall(Img<T> const & l,Img<T> const & m,Img<T> const & r,Fn const & fn)
{
    FGASSERT(l.dims() == r.dims());
    return Img<T>{l.dims(),mapCall(l.m_data,m.m_data,r.m_data,fn)};
}

template<class Out,class In,class Fn>
Img<Out>            mapCallT(Img<In> const & in,Fn const & fn)
{
    return Img<Out>{in.dims(),mapCallT<Out,In,Fn>(in.m_data,fn)};
}

template<class Out,class In0,class In1,class Fn>
Img<Out>            mapCallT(Img<In0> const & in0,Img<In1> const & in1,Fn const & fn)
{
    FGASSERT(in0.dims() == in1.dims());
    return Img<Out>{in0.dims(),mapCallT<Out,In0,In1,Fn>(in0.m_data,in1.m_data,fn)};
}

template<class Out,class In0,class In1,class In2,class Fn>
Img<Out>            mapCallT(Img<In0> const & in0,Img<In1> const & in1,Img<In2> const & in2,Fn const & fn)
{
    FGASSERT(in0.dims() == in1.dims() && in1.dims() == in2.dims());
    return Img<Out>{in0.dims(),mapCallT<Out,In0,In1,In2,Fn>(in0.m_data,in1.m_data,in2.m_data,fn)};
}

template<class T,class U>
inline Img<U>       mapMul(T op,Img<U> const & img) {return Img<U>{img.dims(),mapMul(op,img.m_data)}; }

template<class T,class F>
inline Img<T>       mapCast(Img<F> const & img) {return Img<T>{img.dims(),mapCast<T,F>(img.m_data)}; }

void                mapGamma_(Img3F & img,float gamma);
void                mapGamma_(Img4F & imgApm,float gamma);
ImgC4F              mapGamma(ImgC4F const & imgApm,float gamma);
Img4F               mapGamma(Img4F const & imgApm,float gamma);

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
    T const *   srcPtr,
    T *         dstPtr,         // Must not overlap with srcPtr
    uint        wid,
    uchar       borderPolicy)   // See below
{
    typedef typename Traits<T>::Scalar   Scalar;
    dstPtr[0] = srcPtr[0]*Scalar(2+borderPolicy) + srcPtr[1];
    for (uint ii=1; ii<wid-1; ++ii)
        dstPtr[ii] = srcPtr[ii-1] + srcPtr[ii]*Scalar(2) + srcPtr[ii+1];
    dstPtr[wid-1] = srcPtr[wid-2] + srcPtr[wid-1]*Scalar(2+borderPolicy);
}
template<class T>
void                smoothFloat2D_(
    T const *   srcPtr,
    T *         dstPtr,         // can point to same image as srcPtr
    uint        wid,
    uint        hgt,
    uchar       borderPolicy)
{
    typedef typename Traits<T>::Scalar   Scalar;
    float constexpr factor = 0.25f * 0.25f;
    Img<T>      acc {wid,3};
    T           *accPtr0,
                *accPtr1 = acc.rowPtr(0),
                *accPtr2 = acc.rowPtr(1);
    smoothFloat1D_(srcPtr,accPtr1,wid,borderPolicy);
    srcPtr += wid;
    smoothFloat1D_(srcPtr,accPtr2,wid,borderPolicy);
    for (uint xx=0; xx<wid; ++xx)
        dstPtr[xx] = (accPtr1[xx]*Scalar(2+borderPolicy) + accPtr2[xx]) * factor;
    for (uint yy=1; yy<hgt-1; ++yy) {
        dstPtr += wid;
        srcPtr += wid;
        accPtr0 = acc.rowPtr((yy-1)%3);
        accPtr1 = acc.rowPtr(yy%3);
        accPtr2 = acc.rowPtr((yy+1)%3);
        smoothFloat1D_(srcPtr,accPtr2,wid,borderPolicy);
        for (uint xx=0; xx<wid; ++xx)
            dstPtr[xx] = (accPtr0[xx] + accPtr1[xx] * Scalar(2) + accPtr2[xx]) * factor;
    }
    dstPtr += wid;
    for (uint xx=0; xx<wid; ++xx)
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

// Preserves intrinsic aspect ratio, scales to minimally cover output dimensions.
// Returns transform from output image IRCS to input image IRCS (ie. inverse transform for resampling):
AffineEw2D          imgScaleToCover(Vec2UI inDims,Vec2UI outDims);

// Square resample with implicit zero outside boundary.
// Region and return sizes must be > 0 but region does not have to overlap image:
Img4F               resample(Img4F const & img,SquareF regionPacs,uint retSize,bool multithread);

// Resample an image to the given size with the given inverse transform with boundary clip policy:
template<class T>
Img<T>              resampleClip(Img<T> const & in,Vec2UI outDims,AffineEw2D outToInIrcs)
{
    Img<T>              ret {outDims};
    if (outDims.cmpntsProduct() == 0)
        return ret;
    FGASSERT(in.dims().cmpntsProduct() > 0);
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
    Mat22D          inBounds {
                        -0.5, scast<double>(in.dims()[0])-0.5,
                        -0.5, scast<double>(in.dims()[1])-0.5
                    },
                    outBounds {
                        -0.5, scast<double>(dims[0])-0.5,
                        -0.5, scast<double>(dims[1])-0.5
                    };
    return resampleClip(in,dims,AffineEw2D{outBounds,inBounds});
}

// Resamples a square area of an input image into the given pixel size.
// If necessary, filters first to avoid aliasing.
// Out of bounds samples clamped to boundary values:
Img3F               filterResample(
    Img3F                   in,
    Vec2D                   posPacs,        // top left corner of output image area
    float                   inSize,         // pixel size of square input domain
    uint                    outSize);       // pixel size of square output image
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

template<class T>
Img<T>              catHoriz(Img<T> const & left,Img<T> const & right)
{
    Img<T>              ret;        // RVE (return value elision)
    if (left.empty())
        ret = right;
    else if (right.empty())
        ret = left;
    else {
        FGASSERT(left.height() == right.height());
        ret.resize(left.width()+right.width(),left.height());
        Vec2UI       off;
        for (Iter2UI it(left.dims()); it.valid(); it.next())
            ret[it()] = left[it()];
        off[0] += left.dims()[0];
        for (Iter2UI it(right.dims()); it.valid(); it.next())
            ret[it()+off] = right[it()];
    }
    return ret;
}
template<class T>
Img<T>              catHoriz(Img<T> const & i0,Img<T> const & i1,Img<T> const & i2)
{
    Img<T>              ret;        // RVE
    if (i0.empty())
        ret = catHoriz(i1,i2);
    else if (i1.empty())
        ret = catHoriz(i0,i2);
    else if (i2.empty())
        ret = catHoriz(i0,i1);
    else {
        FGASSERT((i0.height() == i1.height()) && (i1.height() == i2.height()));
        ret.resize(i0.width()+i1.width()+i2.width(),i0.height());
        Vec2UI   off;
        for (Iter2UI it(i0.dims()); it.valid(); it.next())
            ret[it()] = i0[it()];
        off[0] += i0.dims()[0];
        for (Iter2UI it(i1.dims()); it.valid(); it.next())
            ret[it()+off] = i1[it()];
        off[0] += i1.dims()[0];
        for (Iter2UI it(i2.dims()); it.valid(); it.next())
            ret[it()+off] = i2[it()];
    }
    return ret;
}

// concatenate vertically:
template<class T>
void                catVertical_(Img<T> & top,Img<T> const & append)
{
    if (top.empty())
        top = append;
    else if (!append.empty()) {
        FGASSERT(top.width() == append.width());
        // No need to re-arrange data for vertical concatenation:
        top.m_dims[1] += append.m_dims[1];
        cat_(top.m_data,append.m_data);
    }
}
// concatenate vertically:
template<class T>
Img<T>              catVertical(Img<T> const & top,Img<T> const & bottom)
{
    if (top.empty())
        return bottom;
    if (bottom.empty())
        return top;
    FGASSERT(top.width() == bottom.width());
    // No need to re-arrange data for vertical concatenation:
    return Img<T>(top.width(),top.height()+bottom.height(),cat(top.m_data,bottom.m_data));
}
template<class T>
Img<T>              catVertical(Svec<Img<T> > const & imgs)
{
    Img<T>              ret;        // RVE
    if (imgs.empty())
        return ret;
    ret = imgs[0];
    for (size_t ii=1; ii<imgs.size(); ++ii) {
        Img<T> const &      img = imgs[ii];
        FGASSERT(img.width() == ret.width());
        ret.m_dims[1] += img.height();
        // No need to re-arrange data for vertical concatenation:
        cat_(ret.m_data,img.m_data);
    }
    return ret;
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
        Mat22I          srcBnds = dimsToBoundsEub(Vec2I(src.dims())),
                        dstBnds = dimsToBoundsEub(Vec2I(dims)),
                        range = intersectBounds(srcBnds-catHoriz(offset,offset),dstBnds);
        for (Iter2I it(range); it.valid(); it.next())
            ret[Vec2UI(it())] = src[Vec2UI(it()+offset)];
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
Img<T>              outerProduct(Svec<T> const & x,Svec<T> const & y)
{
    Vec2UI          dims(Vec2Z{x.size(),y.size()});
    Img<T>          ret(dims);
    for (Iter2UI it(dims); it.valid(); it.next())
        ret[it()] = x[it()[0]] * y[it()[1]];
    return ret;
}

}

#endif
