//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     Aug 27, 2004
//
// Simple left-to-right, top-to-bottom (row major), tightly-packed, unaligned image templated by pixel type.
// 
// INVARIANTS:
//
// m_data.size() = m_dims[0] * m_dims[1];
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
// * posIpcs = fgMapMul(posIucs,image.dims())

#ifndef FGIMGBASE_HPP
#define FGIMGBASE_HPP

#include "FgStdLibs.hpp"
#include "FgRgba.hpp"
#include "FgIter.hpp"
#include "FgStdStream.hpp"
#include "FgAffineCwC.hpp"

template<typename T>
struct  FgImage
{
    FgVect2UI       m_dims;         // [width,height]
    vector<T>       m_data;         // Pixels stored left to right, top to bottom.

    FG_SERIALIZE2(m_dims,m_data);

    typedef T PixelType;

    FgImage() : m_dims(0) {}

    FgImage(size_t wid,size_t hgt)
    : m_dims(uint(wid),uint(hgt)), m_data(wid*hgt)
    {}

    FgImage(size_t wid,size_t hgt,T fill)
    : m_dims(uint(wid),uint(hgt)), m_data(wid*hgt,fill)
    {}

    explicit
    FgImage(FgVect2UI dims)
    : m_dims(dims), m_data(dims[0]*dims[1])
    {}

    FgImage(FgVect2UI dims,T fillVal)
    : m_dims(dims), m_data(dims[0]*dims[1],fillVal)
    {}

    FgImage(FgVect2UI dims,const vector<T> & imgData)
    : m_dims(dims), m_data(imgData)
    {FGASSERT(m_data.size() == m_dims[0]*m_dims[1]); }

    FgImage(FgVect2UI dims,const T * pixels)
    : m_dims(dims), m_data(pixels,pixels+dims[0]*dims[1])
    {}

    void
    clear()
    {m_data.clear(); m_dims = FgVect2UI(0); }

    // WARNING: This does not adjust any existing image data, just allocated dimensions and memory:
    void
    resize(uint wid,uint hgt)
    {
        m_dims = FgVect2UI(wid,hgt);
        m_data.resize(wid*hgt);
    }

    void
    resize(FgVect2UI dims)
    {resize(dims[0],dims[1]); }

    void
    resize(FgVect2UI dims,T fillVal)
    {
        resize(dims);
        std::fill(m_data.begin(),m_data.end(),fillVal);
    }

    uint
    width() const
    {return m_dims[0]; }

    uint
    height() const
    {return m_dims[1]; }

    FgVect2UI           // (width,height)
    dims() const
    {return m_dims; }

    size_t
    numPixels() const
    {return m_data.size(); }

    bool
    empty() const
    {return (m_data.empty()); }

    // Element access by (X,Y) / (column,row):
    T &
    xy(size_t ircs_x,size_t ircs_y)
    {
        FGASSERT_FAST((ircs_x < m_dims[0]) && (ircs_y < m_dims[1]));
        return m_data[ircs_y*m_dims[0]+ircs_x];
    }
    const T &
    xy(size_t ircs_x,size_t ircs_y) const
    {
        FGASSERT_FAST((ircs_x < m_dims[0]) && (ircs_y < m_dims[1]));
        return m_data[ircs_y*m_dims[0]+ircs_x];
    }

    // Direct access into memory:
    T &
    operator[](size_t idx)
    {return m_data[idx]; }

    const T &
    operator[](size_t idx) const
    {return m_data[idx]; }

    T &
    operator[](FgVect2UI ircsPos)
    {return xy(ircsPos[0],ircsPos[1]); }

    const T &
    operator[](FgVect2UI ircsPos) const
    {return xy(ircsPos[0],ircsPos[1]); }

    template<typename U>
    T &
    operator[](const FgIter<U,2> & it)
    {return xy(it()[0],it()[1]); }

    template<typename U>
    const T &
    operator[](const FgIter<U,2> & it) const
    {return xy(it()[0],it()[1]); }

    T *
    rowPtr(uint row)
    {return &m_data[row*m_dims[0]]; }

    const T *
    rowPtr(uint row) const
    {return &m_data[row*m_dims[0]]; }

    // 'paint' access is bounds checked and out of bounds paints are ignored:
    void
    paint(uint ircs_x,uint ircs_y,T val)
    {
        if ((ircs_x < m_dims[0]) && (ircs_y < m_dims[1]))
            xy(ircs_x,ircs_y) = val;
    }
    void
    paint(FgVect2UI ircs,T val)
    {
        paint(ircs[0],ircs[1],val);
    }
    void
    paint(FgVect2I ircs,T val)
    {
        if ((ircs[0] >= 0) && (ircs[1] >= 0))
            paint(ircs[0],ircs[1],val);
    }

    const T *
    dataPtr() const
    {return (!m_data.empty() ? &m_data[0] : NULL); }

    T *
    dataPtr()
    {return (!m_data.empty() ? &m_data[0] : NULL); }

    const vector<T> &
    dataVec() const
    {return m_data; }
};

typedef FgImage<uchar>      FgImgUC;
typedef FgImage<float>      FgImgF;
typedef FgImage<double>     FgImgD;
typedef FgImage<FgRgbaUB>   FgImgRgbaUb;
typedef FgImage<FgRgbaUS>   FgImgRgbaUs;
typedef FgImage<FgRgbaF>    FgImgRgbaF;
typedef FgImage<FgVect3F>   FgImg3F;
typedef FgImage<FgVect4F>   FgImg4F;
typedef FgImage<FgVect4UC>  FgImg4UC;

typedef vector<FgImgRgbaUb> FgImgs;
typedef vector<FgImgs>      FgImgss;
typedef vector<FgImg3F>     FgImg3Fs;

template<class T>
std::ostream &
operator<<(std::ostream & os,const FgImage<T> & img)
{
    return
        os << "dimensions: " << img.dims()
            << " bounds: " << fgBounds(img.m_data);
}

#endif
