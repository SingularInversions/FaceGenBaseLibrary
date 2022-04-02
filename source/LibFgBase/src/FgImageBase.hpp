//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Simple left-to-right, top-to-bottom (row major), tightly-packed, unaligned image templated by pixel type.
// 
// INVARIANTS:
//
// m_data.size() == m_dims[0] * m_dims[1];
//
// COORDINATE SYSTEMS:
//
// IPCS := Image Pixel CS
//    Origin at top left corner of image, units in pixels.
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
// USAGE NOTES:
//
// * posIrcs = posIpcs - 0.5 (thus floor(posIpcs) rounds to nearest int posIpcs)
// * posIpcs = mapMul(posIucs,image.dims())

#ifndef FGIMGBASE_HPP
#define FGIMGBASE_HPP

#include "FgStdExtensions.hpp"
#include "FgRgba.hpp"

namespace Fg {

template<typename T>
struct  Img
{
    Vec2UI          m_dims;         // [width,height]
    Svec<T>         m_data;         // Pixels stored left to right, top to bottom. size() == m_dims[0]*m_dims[1]

    FG_SERIALIZE2(m_dims,m_data);

    typedef T PixelType;

    Img() : m_dims(0) {}
    Img(size_t wid,size_t hgt) : m_dims{uint(wid),uint(hgt)}, m_data(wid*hgt) {}
    Img(size_t wid,size_t hgt,T fill) : m_dims{uint(wid),uint(hgt)}, m_data(wid*hgt,fill) {}
    Img(size_t wid,size_t hgt,Svec<T> const & data) : m_dims{uint(wid),uint(hgt)}, m_data(data)
        {FGASSERT(data.size() == wid*hgt); }
    explicit Img(Vec2UI dims) : m_dims(dims), m_data(dims[0]*dims[1]) {}
    Img(Vec2UI dims,T fillVal) : m_dims(dims), m_data(dims[0]*dims[1],fillVal) {}
    Img(Vec2UI dims,Svec<T> const & imgData) : m_dims{dims}, m_data(imgData)
        {FGASSERT(m_data.size() == m_dims[0]*m_dims[1]); }
    Img(Vec2UI dims,T const * pixels) : m_dims{dims}, m_data(pixels,pixels+dims[0]*dims[1]) {}

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
    T &             operator[](size_t idx) {return m_data[idx]; }
    T const &       operator[](size_t idx) const {return m_data[idx]; }

    T const *       dataPtr() const {return (!m_data.empty() ? &m_data[0] : NULL); }
    T *             dataPtr() {return (!m_data.empty() ? &m_data[0] : NULL); }
    Svec<T> const & dataVec() const {return m_data; }
    T *             rowPtr(uint row) {return &m_data[row*m_dims[0]]; }
    T const *       rowPtr(uint row) const {return &m_data[row*m_dims[0]]; }

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

typedef Img<uchar>      ImgUC;
typedef Img<float>      ImgF;
typedef Img<double>     ImgD;

typedef Img<Vec2F>      Img2F;

typedef Img<Arr3SC>     Img3SC;
typedef Img<Arr3I>      Img3I;
typedef Img<Arr3F>      Img3F;
typedef Svec<Img3F>     Img3Fs;
typedef Img<Vec3F>      ImgV3F;      // RGB [0,1] unless otherwise noted
typedef Svec<ImgV3F>    ImgV3Fs;

typedef Img<Arr4UC>     Img4UC;
typedef Svec<Img4UC>    Img4UCs;
typedef Img<Arr4F>      Img4F;
typedef Img<Vec4F>      ImgV4F;

typedef Img<Rgba8>     ImgRgba8;
typedef Svec<ImgRgba8>  ImgRgba8s;
typedef Svec<ImgRgba8s> ImgRgba8ss;
typedef Img<RgbaF>      ImgC4F;

template<typename To,typename From>
Img<To>             mapCast(Img<From> const & img)
{
    return Img<To>(img.dims(),mapCast<To>(img.m_data));
}

template<typename To,typename From>
void                deepCast_(Img<To> const & from,Img<From> & to)
{
    to.resize(from.dims());
    deepCast_(from.m_data,to.m_data);
}

}

#endif
