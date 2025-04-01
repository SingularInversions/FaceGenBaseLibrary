//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
//
// Authors:     Andrew Beatty
// Created:     June 27, 2007
//
// Heap based volume raster image templated by voxel type.
//
// PACS := Pixel Area CS (3D context):
//    Origin at lower corner of volume, units in voxels
//    X - "left to right"
//    Y - "top to bottom"
//    Z - "front to back"
//
// IRCS := Image Raster CS (3D context):
//    Origin at centre of first voxel in memory, units are voxels, storage ordered by:
//    X - "left to right"
//    Y - "top to bottom"
//    Z - "front to back"
//
// All coordinates below given in IRCS unless otherwise noted.

#ifndef FGVOLUME_HPP
#define FGVOLUME_HPP

#include "FgArray.hpp"
#include "FgImage.hpp"

namespace Fg {

template<class T>
struct      Volume
{
    Vec3UI              m_dims;         // Number of voxels along each axis (X,Y,Z)
    Svec<T>             m_data;         // Voxels stored by X, then Y, then Z (Z major)
    FG_SER(m_dims,m_data)

    typedef T           VoxelType;

    Volume() {}
    explicit Volume(uint dim) : Volume(Vec3UI{dim}) {}
    explicit Volume(Vec3UI dims) : m_dims{dims} {m_data.resize(Vec3Z{dims}.elemsProduct()); }
    Volume(Vec3UI dims,T val) : m_dims{dims} {m_data.resize(Vec3Z{dims}.elemsProduct(),val); }
    Volume(uint dim,T val) : Volume(Vec3UI{dim},val) {}
    Volume(uint x,uint y,uint z) : Volume(Vec3UI{x,y,z}) {}
    Volume(Vec3UI dims,Svec<T> const & data) : m_dims(dims), m_data(data)
    {FGASSERT(Vec3Z{dims}.elemsProduct() == data.size()); }

    void                resize(uint dim) {resize(Vec3UI(dim)); }
    void                resize(Vec3UI dims)
    {
        m_dims = dims;
        m_data.resize(size_t(m_dims[0]) * m_dims[1] * m_dims[2]);
    }
    void                resize(Vec3UI dims,T val)
    {
        m_dims = dims;
        m_data.resize(size_t(m_dims[0]) * m_dims[1] * m_dims[2],val);
    }
    bool                empty() const {return (m_data.size() == 0); }
    Vec3UI              dims() const {return m_dims; }
    Mat32F              pacsBounds() const {return Mat32F(0,m_dims[0],0,m_dims[1],0,m_dims[2]); }
    size_t              numVoxels() const {return m_data.size(); }
    template<uint ncols>
    Mat<uint,3,ncols>   clip(const Mat<int,3,ncols> & pos) const
    {
        Mat<uint,3,ncols>     ret;
        for (uint row=0; row<3; ++row)
            for (uint col=0; col<ncols; ++col)
                ret.rc(row,col) = uint(clamp(pos.rc(row,col),0,m_dims[row]-1));
        return ret;
    }
    T *                 xPtr(size_t yy,size_t zz) {return &m_data[zz*m_dims[0]*m_dims[1]+yy*m_dims[0]]; }
    T const *           xPtr(size_t yy,size_t zz) const {return &m_data[zz*m_dims[0]*m_dims[1]+yy*m_dims[0]]; }
    // Element access by (X,Y,Z):
    T &                 xyz(size_t xx,size_t yy,size_t zz)
    {
        FGASSERT_FAST((xx < m_dims[0]) && (yy < m_dims[1]) && (zz < m_dims[2]));
        return m_data[zz*m_dims[0]*m_dims[1] + yy*m_dims[0] + xx];
    }
    T const &           xyz(size_t xx,size_t yy,size_t zz) const
    {
        FGASSERT_FAST((xx < m_dims[0]) && (yy < m_dims[1]) && (zz < m_dims[2]));
        return m_data[zz*m_dims[0]*m_dims[1] + yy*m_dims[0] + xx];
    }
    T &                 operator[](size_t idx)
    {
        FGASSERT_FAST(idx < m_data.size());
        return m_data[idx];
    }
    T const &           operator[](size_t idx) const
    {
        FGASSERT_FAST(idx < m_data.size());
        return m_data[idx];
    }
    T &                 operator[](Vec3UI const & pos) {return xyz(pos[0],pos[1],pos[2]); }
    T const &           operator[](Vec3UI const & pos) const {return xyz(pos[0],pos[1],pos[2]); }
    T &                 operator[](Arr3Z const & pos) {return xyz(pos[0],pos[1],pos[2]); }
    T const &           operator[](Arr3Z const & pos) const {return xyz(pos[0],pos[1],pos[2]); }
    void                fill(T val)
    {
        for (size_t ii=0; ii<m_data.size(); ++ii)
            m_data[ii] = val;
    }
    T *                 dataPtr() {return &m_data[0]; }
    T const *           dataPtr() const {return &m_data[0]; }
    Vec3UI              idxToPos(size_t idx) const
    {
        size_t      strideZ = m_dims[0] * m_dims[1],
                    z = idx / strideZ,
                    zr = idx - z * strideZ,
                    y = zr / m_dims[0],
                    x = zr - y * m_dims[0];
        return Vec3UI(x,y,z);
    }
    Volume<T>           operator+(Volume<T> const & rhs) const
    {
        FGASSERT(rhs.m_dims == m_dims);
        return Volume<T>{m_dims,m_data+rhs.m_data};
    }
    Volume<T>           operator-(Volume<T> const & rhs) const
    {
        FGASSERT(rhs.m_dims == m_dims);
        return Volume<T>{m_dims,m_data-rhs.m_data};
    }
    Volume<T>           operator*(T rhs) const {return Volume<T>{m_dims,m_data*rhs}; }
    void                operator*=(T rhs) {m_data *= rhs; }
};

typedef Volume<float>       VolF;
typedef Volume<Vec3F>       Vol3F;
typedef Volume<Vec3D>       Vol3D;
typedef Volume<Vec4F>       Vol4F;
typedef Volume<Mat33F>      Vol33F;

template<class T>
std::ostream &      operator<<(std::ostream & os,Volume<T> const & v)
{
    os << fgnl << "[" << fgpush;
    Vec3UI      dims = v.dims();
    for (uint zz=0; zz<dims[2]; ++zz) {
        for (uint yy=0; yy<dims[1]; ++yy) {
            os << fgnl << "[";
            for (uint xx=0; xx<dims[0]; ++xx)
                os << v.xyz(xx,yy,zz) << ", ";
            os << " ]";
        }
        os << "," << fgnl;
    }
    os  << fgnl << "Sum: " << cSum(v.m_data)
        << fgnl << "Mag: " << cMagD(v.m_data)
        << fgnl << "RMS: " << cRms(v.m_data)
        << fgpop;
    return os;
}

template<class T>
Volume<T>           outerProduct(Svec<T> const & x,Svec<T> const & y,Svec<T> const & z)
{
    Vec3UI          dims(Vec3Z{x.size(),y.size(),z.size()});
    Volume<T>       ret(dims);
    for (Iter3UI it(dims); it.valid(); it.next())
        ret[it()] = x[it()[0]] * y[it()[1]] * z[it()[2]];
    return ret;
}

// Applies a [1 2 1] / 4 outer product 3D kernel smoothing to a floating point channel
// volume image in a cache-friendly way.
// The Source and destination volumes can be the same, for in-place convolution.
template<class T>
void                smoothFloat_(
    Volume<T> const &   src,
    Volume<T> &         dst,                // Can be same as src
    uchar               borderPolicy)       // 0 - zero border policy, 1 - replication border policy
{
    FGASSERT(cMinElem(src.dims()) > 1);    // Algorithm requires this
    FGASSERT((borderPolicy == 0) || (borderPolicy == 1));
    dst.resize(src.dims());
    uint            wid = src.dims()[0],
                    hgt = src.dims()[1],
                    dep = src.dims()[2],
                    stride = wid*hgt;
    Volume<T>       acc(wid,hgt,3);
    float constexpr fac = 0.25f;
    T const         *srcPtr = src.dataPtr();
    T               *accPtr0,
                    *accPtr1 = acc.dataPtr(),
                    *accPtr2 = accPtr1+stride,
                    *dstPtr = dst.dataPtr();
    smoothFloat2D_(srcPtr,accPtr1,wid,hgt,borderPolicy);
    srcPtr += stride;
    smoothFloat2D_(srcPtr,accPtr2,wid,hgt,borderPolicy);
    for (uint ii=0; ii<stride; ++ii)
        dstPtr[ii] = (accPtr1[ii]*(2U+borderPolicy) + accPtr2[ii]) * fac;
    for (uint zz=1; zz<dep-1; ++zz) {
        dstPtr += stride;
        srcPtr += stride;
        accPtr0 = acc.dataPtr() + ((zz-1)%3) * stride;
        accPtr1 = acc.dataPtr() + (zz%3) * stride;
        accPtr2 = acc.dataPtr() + ((zz+1)%3) * stride;
        smoothFloat2D_(srcPtr,accPtr2,wid,hgt,borderPolicy);
        for (uint ii=0; ii<stride; ++ii)
            dstPtr[ii] = (accPtr0[ii] + accPtr1[ii] * 2 + accPtr2[ii]) * fac;
    }
    dstPtr += stride;
    for (uint ii=0; ii<stride; ++ii)
        dstPtr[ii] = (accPtr1[ii] + accPtr2[ii]*(2+borderPolicy)) * fac;
}

// Returns PACS coordinates of 26-connected strictly greater than maxima:
template<typename T>
Vec3UIs             cMaxima(Volume<T> const & vol)   // Empty volumes not allowed but a single voxel volume will return a max:
{
    Vec3UIs          ret;
    FGASSERT(!vol.empty());
    for (Iter3UI it(vol.dims()); it.valid(); it.next()) {
        T               val = vol[it()];
        Mat32UI       neighBounds = iubToEub(it.neighbourBounds());
        bool            isMax = true;
        for (Iter3UI nit(neighBounds); nit.valid(); nit.next()) {
            if (nit() != it()) {    // value can't be strictly greater than self
                if (!(val > vol[nit()])) {
                    isMax = false;
                    break;
                }
            }
        }
        if (isMax)
            ret.push_back(it());
    }
    return ret;
}

template<class T>
double              cMagD(const Fg::Volume<T> & v) {return cMagD(v.m_data); }

struct      CoordWgt3
{
    Vec3UI      coordIrcs;      // 3D image coordinate
    double      wgt;            // Respective weight
};

// Tri-linear interpolation with sample culling
VArray<CoordWgt3,8>
lerpCoordsCull(Vec3UI dims,Vec3D ircs);     // Image Raster Coordinate System

template<class Vol>
typename Vol::VoxelType sampleCull(Vol const & vol,Vec3D ircs)
{
    VArray<CoordWgt3,8>     lerpv = lerpCoordsCull(vol.dims(),ircs);
    typename Vol::VoxelType ret(0);
    for (uint ii=0; ii<lerpv.size(); ++ii)
        ret += vol[lerpv[ii].coordIrcs] * lerpv[ii].wgt;
    return ret;
}

// 2x shrink using block average for types composed of floating point values.
// Truncates last row/col if width/height not even.
template<class T>
void                shrink2(
    Volume<T> const &   src,
    Volume<T> &         dst,    // Must be a different instance
    float               fac=0.125f)
{
    FGASSERT(!src.empty());
    Vec3UI          dims = src.dims()/2;
    dst.resize(dims);
    FGASSERT(src.dataPtr() != dst.dataPtr());
    for (uint zz=0; zz<dims[2]; ++zz)
        for (uint yy=0; yy<dims[1]; ++yy)
            for (uint xx=0; xx<dims[0]; ++xx)
                dst.xyz(xx,yy,zz) = (
                    src.xyz(2*xx,2*yy,2*zz) +
                    src.xyz(2*xx+1,2*yy,2*zz) +
                    src.xyz(2*xx+1,2*yy+1,2*zz) +
                    src.xyz(2*xx,2*yy+1,2*zz) +
                    src.xyz(2*xx,2*yy,2*zz+1) +
                    src.xyz(2*xx+1,2*yy,2*zz+1) +
                    src.xyz(2*xx+1,2*yy+1,2*zz+1) +
                    src.xyz(2*xx,2*yy+1,2*zz+1)
                ) * fac;
}
template<class T>
Volume<T>           shrink2(Volume<T> const & src)
{
    Volume<T>       ret;
    shrink2(src,ret);
    return ret;
}

}

#endif
