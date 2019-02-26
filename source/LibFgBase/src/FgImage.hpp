//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     May 14, 2005
//

#ifndef FGIMAGE_HPP
#define FGIMAGE_HPP

#include "FgImageBase.hpp"
#include "FgImageIo.hpp"
#include "FgAlgs.hpp"
#include "FgIter.hpp"
#include "FgAffineC.hpp"
#include "FgAffineCwC.hpp"
#include "FgArray.hpp"

std::ostream &
operator<<(std::ostream &,const FgImgRgbaUb &);

inline
FgAffineCw2F
fgOicsToIucs()
{return FgAffineCw2F(FgMat22F(-1,1,-1,1),FgMat22F(0,1,1,0)); }

inline
FgAffineCw2F
fgIucsToIpcs(FgVect2UI dims)
{return FgAffineCw2F(FgMat22F(0,1,0,1),FgMat22F(0,dims[0],0,dims[1])); }

inline
FgAffineCw2F
fgIrcsToIucs(FgVect2UI imageDims)
{return FgAffineCw2F(FgMat22F(-0.5,imageDims[0]-0.5,-0.5,imageDims[1]-0.5),FgMat22F(0,1,0,1)); }

inline
FgVect2F
fgIucsToIrcs(FgVect2UI ircsDims,FgVect2F iucsCoord)
{return (fgMapMul(iucsCoord,FgVect2F(ircsDims)) - FgVect2F(0.5)); }

template<uint dim>
inline
FgAffineCwC<float,dim>
fgIrcsToIpcs()
{return FgAffineCwC<float,dim>(FgMatrixC<float,dim,1>(1),FgMatrixC<float,dim,1>(0.5f)); }

struct  FgCoordWgt
{
    FgVect2UI   coordIrcs;
    double      wgt;
};

// Bilinear interpolation co-efficients culling samples outside bounds.
// Return value can contain between 0 and 4 samples depending on how many are culled:
FgArray<FgCoordWgt,4>
fgBlerpCoordsCull(FgVect2UI dims,FgVect2F coordIucs);

// Bilinear interpolation co-efficients with coordinate clipping.
// Return value structure much easier to use than a terser bounds-based representation:
FgMatrixC<FgCoordWgt,4,1>
fgBlerpCoordsClip(FgVect2UI dims,FgVect2F coordIucs);

// Bilinear interpolation. Returns alpha-weighted linear-interpolated value.
// Source points outside image considered to have alpha zero:
FgRgbaF
fgBlerpAlpha(const FgImgRgbaUb & img,FgVect2F coordIucs);

// Bilinear interpolation.
// Clips sample point coordinates to image boundaries.
template<class T>
typename FgTraits<T>::Floating      // 'float' for scalars, 'float' components for vectors
fgBlerpClipIpcs(
    const FgImage<T> &  img,        // Must not be empty
    FgVect2F            coordIpcs)
{
    FGASSERT(!img.empty());         // Required for algorithm below
    typedef typename FgTraits<T>::Floating    Acc;
    Acc                 acc(0.0);
    float               xf = coordIpcs[0] - 0.5f,     // to IRCS
                        yf = coordIpcs[1] - 0.5f;
    int                 xi = int(std::floor(xf)),
                        yi = int(std::floor(yf));
    float               wxh = xf - float(xi),
                        wyh = yf - float(yi),
                        wxl = 1.0f - wxh,
                        wyl = 1.0f - wyh;
    uint                xl = uint((xi < 0) ? 0 : xi),
                        yl = uint((yi < 0) ? 0 : yi),
                        xh = uint((xi+1 < 0) ? 0 : xi+1),
                        yh = uint((yi+1 < 0) ? 0 : yi+1),
                        xm = img.width()-1,
                        ym = img.height()-1;
    xl = (xl > xm) ? xm : xl;
    yl = (yl > ym) ? ym : yl;
    xh = (xh > xm) ? xm : xh;
    yh = (yh > ym) ? ym : yh;
    acc += Acc(img.xy(xl,yl)) * wxl * wyl;
    acc += Acc(img.xy(xh,yl)) * wxh * wyl;
    acc += Acc(img.xy(xl,yh)) * wxl * wyh;
    acc += Acc(img.xy(xh,yh)) * wxh * wyh;
    return acc;
}
template<class T>
typename FgTraits<T>::Floating
fgBlerpClipIucs(const FgImage<T> & img,FgVect2F coordIucs)
{return fgBlerpClipIpcs(img,fgMapMul(coordIucs,FgVect2F(img.dims()))); }

template<class T>
FgMatrixC<T,2,2>
fgSampleFdd1Clamp(const FgImage<T> & img,FgVect2UI coord)
{
    FgMatrixC<T,2,2>    ret;
    FgVect2UI           dims = img.dims();
    for (uint dim=0; dim<2; ++dim) {
        for (int xx=-1; xx<2; xx+=2) {
            int         pos = int(coord[dim]) + xx,
                        max = int(dims[dim]) - 1;
            if (pos < 0)
                pos = 0;
            else if (pos > max)
                pos = max;
            FgVect2UI   crd = coord;
            crd[dim] = uint(pos);
            ret.cr((xx+1)/2,dim) = img[crd];
        }
    }
    return ret;
}

template<class T>
FgImage<T>
fgThreshold(const FgImage<T> & img,T minRoundUp)
{
    FgImage<T>      ret(img.dims());
    for (size_t ii=0; ii<ret.m_data.size(); ++ii)
        ret.m_data[ii] = (img.m_data[ii] < minRoundUp) ? T(0) : std::numeric_limits<T>::max();
    return ret;
}

// Shrink an image by 2x2 averaging blocks, rounding down the image size if odd:
void
fgImgShrink2(const FgImgRgbaUb & src,FgImgRgbaUb & dst);

inline
FgImgRgbaUb
fgImgShrink2(const FgImgRgbaUb & src)
{
    FgImgRgbaUb ret;
    fgImgShrink2(src,ret);
    return ret;
}

// Resampled 2x expansion. See 'magnify' for a unresampled.
FgImgRgbaUb
fgExpand2(const FgImgRgbaUb & src);

// 2x image shrink using block average for types composed of floating point values.
// Truncates last row/col if width/height not even.
template<typename T>
void
fgShrink2Float(
    const FgImage<T> &  src,
    FgImage<T> &        dst)    // Must be a different instance
{
    FGASSERT(src.dataPtr() != dst.dataPtr());
    dst.resize(src.dims()/2);
    for (uint yy=0; yy<dst.height(); ++yy)
        for (uint xx=0; xx<dst.width(); ++xx)
            dst.xy(xx,yy) = (
                src.xy(2*xx,2*yy) +
                src.xy(2*xx+1,2*yy) +
                src.xy(2*xx+1,2*yy+1) +
                src.xy(2*xx,2*yy+1)) / 4;
}

template<class T>
FgImage<T>
fgShrink2Float(const FgImage<T> & img)
{
    FgImage<T>      ret;
    fgShrink2Float(img,ret);
    return ret;
}

// Shrink an image by 1x2 averaging blocks, rounding down the image width if odd.
FgImgRgbaUb
fgImgShrinkWid2(const FgImgRgbaUb & src);

// Shrink an image by 2x1 averaging blocks, rounding down the image height if odd.
FgImgRgbaUb
fgImgShrinkHgt2(const FgImgRgbaUb & src);

// Repeat each pixel value 'fac' times in both dimensions:
template<class T>
FgImage<T>
fgImgMagnify(const FgImage<T> & img,size_t fac)
{
    FgImage<T>      ret(img.dims()*uint(fac));
    for (FgIter2UI  it(ret.dims()); it.next(); it.valid())
        ret[it()] = img[it()/uint(fac)];
    return ret;
}

void
fgImgConvert(const FgImgRgbaUb & src,FgImgUC & dst);

void
fgImgConvert(const FgImgUC & src,FgImgRgbaUb & dst);

// Resize to the destination image dimensions, by shrinking or expanding in each dimension.
// Samples the exact proportional amount of the source image covered by the destination image
// pixel for shrinking dimensions, which effectively uses bilinear interpolation for dilating
// dimensions:
void
fgImgResize(
    const FgImgRgbaUb & src,
    FgImgRgbaUb &       dst);   // MODIFIED

void
fgImgPntRescaleConvert(const FgImgD & src,FgImgUC & dst);

void
fgImgPntRescaleConvert(const FgImgD & src,FgImgRgbaUb & dst);

FgAffine2D
fgImgGetIrcsToBounds(uint wid,uint hgt,const FgMat22D & bounds);

FgImgRgbaUb
fgImgApplyTransparencyPow2(
    const FgImgRgbaUb & colour,
    const FgImgRgbaUb & transparency);

FgImage<FgBool>
fgAnd(const FgImage<FgBool> &,const FgImage<FgBool> &);

template<class T>
void
fgImgFlipVertical(FgImage<T> & img)
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
FgImage<T>
fgFlipHoriz(const FgImage<T> & img)
{
    FgImage<T>  ret(img.dims());
    uint        xh = img.width()-1;
    for (uint yy=0; yy<img.height(); ++yy) {
        for (uint xx=0; xx<img.width(); ++xx)
            ret.xy(xh-xx,yy) = img.xy(xx,yy); }
    return ret;
}

template<class T>
inline void
fgImgFill(
    FgImage<T> &    img,
    T               val)
{
    for (size_t ii=0; ii<img.numPixels(); ++ii)
        img[ii] = val;
}

// Note that 'trans' is applied BEFORE scale, unlike homogenous representation, since this is
// usually more convenient & precise for what needs to be done.
template<class T>
void
fgImgPntAffine(const FgImage<T>& in,FgImage<T>& out,T trans,T scale)
{
    out.resize(in.width(),in.height());
    for (size_t ii=0; ii<out.numPixels(); ++ii)
        out[ii] = (in[ii] + trans) * scale;
}

template<class T,class U>
void
fgCast_(const FgImage<T> & in,FgImage<U> & out)
{
    out.resize(in.dims());
    for (size_t ii=0; ii<out.numPixels(); ++ii)
        fgCast_(in[ii],out[ii]);
}

// Accumulators for convolution:
template<class T> struct FgConvTraits;
template<> struct FgConvTraits<uchar>       {typedef ushort     Acc; };
template<> struct FgConvTraits<FgRgbaUB>    {typedef FgRgbaUS   Acc; };
template<class T>
void
fgSmoothUint1D(
    const T             *srcPtr,
    typename FgConvTraits<T>::Acc *dstPtr,
    uint                num,
    uchar               borderPolicy)               // See below
{
    typedef typename FgConvTraits<T>::Acc     Acc;
    dstPtr[0] = Acc(srcPtr[0])*(2+borderPolicy) + Acc(srcPtr[1]);
    for (uint ii=1; ii<num-1; ++ii)
        dstPtr[ii] = Acc(srcPtr[ii-1]) + Acc(srcPtr[ii])*2 + Acc(srcPtr[ii+1]);
    dstPtr[num-1] = Acc(srcPtr[num-2]) + Acc(srcPtr[num-1])*(2+borderPolicy);
}
// Applies a [1 2 1] outer product 2D kernel smoothing using border replication to an
// UNISGNED INTEGER channel image in a preicsion-friendly, cache-friendly way.
// The Source and desination images can be the same, for in-place convolution:
template<class T>
void
fgSmoothUint(
    const FgImage<T> &  src,
    FgImage<T> &        dst,
    uchar               borderPolicy=1)     // 0 - zero border policy, 1 - replication border policy
{
    typedef typename FgConvTraits<T>::Acc     Acc;
    FGASSERT((src.width() > 1) && (src.height() > 1));  // Algorithm not designed for dim < 2
    FGASSERT((borderPolicy == 0) || (borderPolicy == 1));
    dst.resize(src.width(),src.height());
    FgImage<Acc> acc(src.width(),3);                  // Accumulator image
    T           *dstPtr = dst.rowPtr(0);
    Acc         *accPtr0,
                *accPtr1 = acc.rowPtr(0),
                *accPtr2 = acc.rowPtr(1);
    fgSmoothUint1D(src.rowPtr(0),accPtr1,src.width(),borderPolicy);
    fgSmoothUint1D(src.rowPtr(1),accPtr2,src.width(),borderPolicy);
    for (uint xx=0; xx<src.width(); xx++) {
        // Add 7 to minimize rounding bias. Adding 8 would bias the other way so we have to
        // settle for a small amount of downward rounding bias unless we want to pseudo-randomize:
        Acc        tmp = accPtr1[xx]*(2+borderPolicy) + accPtr2[xx] + Acc(7);
        dstPtr[xx] = T(tmp / 16);
    }
    for (uint yy=1; yy<src.height()-1; ++yy) {
        dstPtr = dst.rowPtr(yy),
        accPtr0 = acc.rowPtr((yy-1)%3),
        accPtr1 = acc.rowPtr(yy%3),
        accPtr2 = acc.rowPtr((yy+1)%3);
        fgSmoothUint1D(src.rowPtr(yy+1),accPtr2,src.width(),borderPolicy);
        for (uint xx=0; xx<src.width(); ++xx)
            dstPtr[xx] = T((accPtr0[xx] + accPtr1[xx]*2 + accPtr2[xx] + Acc(7)) / 16);
    }
    dstPtr = dst.rowPtr(dst.height()-1);
    for (uint xx=0; xx<dst.width(); ++xx)
        dstPtr[xx] = T((accPtr1[xx] + accPtr2[xx]*(2+borderPolicy) + Acc(7)) / 16);
}
template<class T>
FgImage<T>
fgSmoothUint(const FgImage<T> & src,uchar borderPolicy=1)
{
    FgImage<T>      ret;
    fgSmoothUint(src,ret,borderPolicy);
    return ret;
}

// Use of the __restrict keyword for the pointer args below made no speed difference here (msvc2012).
// Perhaps the compiler is smart enough to look at the calling context:
template<class T>
void
fgSmoothFloat1D(
    const T *   srcPtr,
    T *         dstPtr,         // Must not overlap with srcPtr
    uint        wid,
    uchar       borderPolicy)   // See below
{
    dstPtr[0] = srcPtr[0]*(2+borderPolicy) + srcPtr[1];
    for (uint ii=1; ii<wid-1; ++ii)
        dstPtr[ii] = srcPtr[ii-1] + srcPtr[ii]*2 + srcPtr[ii+1];
    dstPtr[wid-1] = srcPtr[wid-2] + srcPtr[wid-1]*(2+borderPolicy);
}
template<class T>
void
fgSmoothFloat2D(
    const T *   srcPtr,
    T *         dstPtr,         // Must not overlap with srcPtr
    uint        wid,
    uint        hgt,
    uchar       borderPolicy,   // See below
    float       fac=1.0f/4.0f)  // Per-axis kernel normalization factor
{
    float       factor = fac*fac;
    FgImage<T>  acc(wid,3);
    T           *accPtr0,
                *accPtr1 = acc.rowPtr(0),
                *accPtr2 = acc.rowPtr(1);
    fgSmoothFloat1D(srcPtr,accPtr1,wid,borderPolicy);
    srcPtr += wid;
    fgSmoothFloat1D(srcPtr,accPtr2,wid,borderPolicy);
    for (uint xx=0; xx<wid; ++xx)
        dstPtr[xx] = (accPtr1[xx]*(2+borderPolicy) + accPtr2[xx]) * factor;
    for (uint yy=1; yy<hgt-1; ++yy) {
        dstPtr += wid;
        srcPtr += wid;
        accPtr0 = acc.rowPtr((yy-1)%3);
        accPtr1 = acc.rowPtr(yy%3);
        accPtr2 = acc.rowPtr((yy+1)%3);
        fgSmoothFloat1D(srcPtr,accPtr2,wid,borderPolicy);
        for (uint xx=0; xx<wid; ++xx)
            dstPtr[xx] = (accPtr0[xx] + accPtr1[xx] * 2 + accPtr2[xx]) * factor;
    }
    dstPtr += wid;
    for (uint xx=0; xx<wid; ++xx)
        dstPtr[xx] = (accPtr1[xx] + accPtr2[xx]*(2+borderPolicy)) * factor;
}
// Applies a [1 2 1] outer product 2D kernel smoothing to a floating point channel
// image in a cache-friendly way.
// The Source and destination images can be the same, for in-place convolution.
template<class T>
void
fgSmoothFloat(
    const FgImage<T> &  src,
    FgImage<T> &        dst,                // Can be same as src
    uchar               borderPolicy)       // 0 - zero border policy, 1 - replication border policy
{
    FGASSERT((src.width() > 1) && (src.height() > 1));  // Algorithm not designed for dim < 2
    FGASSERT((borderPolicy == 0) || (borderPolicy == 1));
    dst.resize(src.dims());
    fgSmoothFloat2D(src.dataPtr(),dst.dataPtr(),src.width(),src.height(),borderPolicy);
}

// Only defined for binarized images, output is binarized:
template<class T>
FgImage<T>
fgDilate(const FgImage<T> & img)
{
    FgImage<T>      ret = fgSmoothUint(img);
    for (T & p : ret.m_data)
        p = (p > 0) ? std::numeric_limits<T>::max() : 0;
    return ret;
}

// Applies a 3x3 non-separable kernel to a floating-point channel image. (technically a correlation
// rather than a convolution since we don't mirror the kernel but client can do that).
// No normalization of the kernel is done, that's up to the client.
template<class Pixel>
void
fgConvolveFloatHoriz(
    const Pixel *       srcPtrs[3],
    const FgMat33F & krn,
    Pixel *             dstPtr,
    uint                sz,
    uchar               borderPolicy)               // See below
{
    dstPtr[0] = Pixel(0);
    for (uint jj=0; jj<3; ++jj)
        dstPtr[0] += srcPtrs[jj][0] * (krn.rc(jj,0)*borderPolicy + krn.rc(jj,1)) +
                     srcPtrs[jj][1] * krn.rc(jj,2);
    for (uint ii=1; ii<sz-1; ++ii) {
        dstPtr[ii] = Pixel(0);
        for (uint jj=0; jj<3; ++jj)
            dstPtr[ii] += srcPtrs[jj][ii-1] * krn.rc(jj,0) +
                          srcPtrs[jj][ii]   * krn.rc(jj,1) +
                          srcPtrs[jj][ii+1] * krn.rc(jj,2);
    }
    dstPtr[sz-1] = Pixel(0);
    for (uint jj=0; jj<3; ++jj)
        dstPtr[sz-1] += srcPtrs[jj][sz-2] * krn.rc(jj,0) +
                        srcPtrs[jj][sz-1] * (krn.rc(jj,1)+krn.rc(jj,2)*borderPolicy);
}
template<class Pixel>
void
fgConvolveFloat(
    const FgImage<Pixel> &  src,
    const FgMat33F &     krn,                // The kernel to be correlated
    FgImage<Pixel> &        dst,                // Must be different from src
    uchar                   borderPolicy)       // 0 - zero border policy, 1 - replication border policy
{
    FGASSERT((src.width() > 1) && (src.height() > 1));  // Algorithm not designed for dim < 2
    FGASSERT((borderPolicy == 0) || (borderPolicy == 1));
    dst.resize(src.dims());
    FGASSERT(src.dataPtr() != dst.dataPtr());
    uint                    wid = src.width();
    vector<Pixel>           boundaryRow(wid,Pixel(0));
    const Pixel *           srcPtrs[3];
    if (borderPolicy == 0)
        srcPtrs[0] = &boundaryRow[0];
    else
        srcPtrs[0] = src.rowPtr(0);
    srcPtrs[1] = src.rowPtr(0);
    srcPtrs[2] = src.rowPtr(1);
    fgConvolveFloatHoriz(srcPtrs,krn,dst.rowPtr(0),wid,borderPolicy);
    for (uint yy=1; yy<dst.height()-1; ++yy) {
        srcPtrs[0] = srcPtrs[1];
        srcPtrs[1] = srcPtrs[2];
        srcPtrs[2] = src.rowPtr(yy+1);
        fgConvolveFloatHoriz(srcPtrs,krn,dst.rowPtr(yy),wid,borderPolicy);
    }
    srcPtrs[0] = srcPtrs[1];
    srcPtrs[1] = srcPtrs[2];
    if (borderPolicy == 0)
        srcPtrs[2] = &boundaryRow[0];
    fgConvolveFloatHoriz(srcPtrs,krn,dst.rowPtr(dst.height()-1),wid,borderPolicy);
}

// Resample 'in' at the centre of each pixel in 'out' (assuming images are spatially 1-1):
template<class T>
void
fgResampleSimple(
    const FgImage<T> &  in,
    FgImage<T> &        out)
{
    if (in.dims().cmpntsProduct() == 0)
        out.clear();
    else {
        FGASSERT(fgMinElem(out.dims()) > 0);
        FgMat22F        inBoundsIucs( 0.0f,1.0f,    // fgBlerpClipIucs takes UICS
                                      0.0f,1.0f),
                        outBoundsIrcs(-0.5f,float(out.width())-0.5f,
                                      -0.5f,float(out.height())-0.5f);
        FgAffineCw2F    o2i(outBoundsIrcs,inBoundsIucs);
        for (FgIter2UI it(out.dims()); it.valid(); it.next()) {
            FgVect2F    pt = o2i * FgVect2F(it());
            fgRound(fgBlerpClipIucs(in,pt),out[it]);
        }
    }
}
template<class T>
FgImage<T>
fgResampleSimple(const FgImage<T> & in,FgVect2UI dims)
{
    FgImage<T>      ret(dims);
    fgResampleSimple(in,ret);
    return ret;
}

template<class T>
void
fgResizePow2Ceil_(const FgImage<T> & in,FgImage<T> & out)
{
    if (!fgIsPow2(in.dims())) {
        out.resize(fgPow2Ceil(in.dims()));
        fgResampleSimple(in,out);
    }
    else
        out = in;
}

template<class T>
FgImage<T>
fgResizePow2Ceil(const FgImage<T> & img)
{
    FgImage<T>      ret;
    fgResizePow2Ceil_(img,ret);
    return ret;
}

bool
fgImgApproxEqual(const FgImgRgbaUb & img0,const FgImgRgbaUb & img1,uint maxDelta=0);

template<class T>
double
fgImgSsd(
    const FgImage<FgRgba<T> > & im0,
    const FgImage<FgRgba<T> > & im1)
{
    FGASSERT(im0.dims() == im1.dims());
    double      acc = 0.0;
    for (FgIter2UI it(im0.dims()); it.valid(); it.next())
    {
        FgVect4D        p0,p1;
        fgCast_(im0[it].m_c,p0);
        fgCast_(im1[it].m_c,p1);
        acc += (p0-p1).mag();
    }
    return acc;
}

template<class T>
FgImage<T>
fgJoinHoriz(const FgImage<T> & l,const FgImage<T> & r)
{
    FgImage<T>      ret;
    if (l.empty())
        ret = r;
    else if (r.empty())
        ret = l;
    else {
        FGASSERT(l.height() == r.height());
        ret.resize(l.width()+r.width(),l.height());
        FgVect2UI       off;
        for (FgIter2UI it(l.dims()); it.valid(); it.next())
            ret[it()] = l[it()];
        off[0] += l.dims()[0];
        for (FgIter2UI it(r.dims()); it.valid(); it.next())
            ret[it()+off] = r[it()];
    }
    return ret;
}

template<class T>
FgImage<T>
fgJoinHoriz(const FgImage<T> & l,const FgImage<T> & c,const FgImage<T> & r)
{
    FgImage<T>      ret;
    if (l.empty())
        ret = fgJoinHoriz(c,r);
    else if (c.empty())
        ret = fgJoinHoriz(l,r);
    else if (r.empty())
        ret = fgJoinHoriz(l,c);
    else {
        FGASSERT((l.height() == c.height()) && (c.height() == r.height()));
        ret.resize(l.width()+c.width()+r.width(),l.height());
        FgVect2UI   off;
        for (FgIter2UI it(l.dims()); it.valid(); it.next())
            ret[it()] = l[it()];
        off[0] += l.dims()[0];
        for (FgIter2UI it(c.dims()); it.valid(); it.next())
            ret[it()+off] = c[it()];
        off[0] += c.dims()[0];
        for (FgIter2UI it(r.dims()); it.valid(); it.next())
            ret[it()+off] = r[it()];
    }
    return ret;
}

template<class T>
FgImage<T>
fgJoinVert(const FgImage<T> & t,const FgImage<T> & b)
{
    FgImage<T>      ret;
    if (t.empty())
        ret = b;
    else if (b.empty())
        ret = t;
    else {
        FGASSERT(t.width() == b.width());
        ret.resize(t.width(),t.height()+b.height());
        FgVect2UI   off;
        for (FgIter2UI it(t.dims()); it.valid(); it.next())
            ret[it()] = t[it()];
        off[1] += t.dims()[1];
        for (FgIter2UI it(b.dims()); it.valid(); it.next())
            ret[it()+off] = b[it()];
    }
    return ret;
}

template<class T>
FgImage<T>
fgJoinVert(const std::vector<FgImage<T> > & v)
{
    FgImage<T>  ret;
    for (size_t ii=0; ii<v.size(); ++ii)
        ret = fgJoinVert(ret,v[ii]);
    return ret;
}

template<class T>
FgImage<T>
fgCropPad(
    const FgImage<T> &  src,
    FgVect2UI           dims,
    FgVect2I            offset = FgVect2I(0),
    T                   fill = T())
{
    FgImage<T>      ret(dims,fill);
    if (!src.empty()) {
        FgMat22I        srcBnds = FgMat22I(fgRangeToBounds(src.dims())),
                        dstBnds = FgMat22I(fgRangeToBounds(dims)),
                        range = fgBoundsIntersection(srcBnds-fgJoinHoriz(offset,offset),dstBnds);
        for (FgIter2I it(fgInclToExcl(range)); it.valid(); it.next())
            ret[FgVect2UI(it())] = src[FgVect2UI(it()+offset)];
    }
    return ret;
}

// Requires alpha-weighted color values:
template<class T>
FgImage<T>
fgComposite(
    const FgImage<T> &  foreground,
    const FgImage<T> &  background)
{
    FGASSERT(foreground.dims() == background.dims());
    FgImage<T>      ret(foreground.dims());
    for (size_t ii=0; ii<ret.numPixels(); ++ii)
        ret[ii] = fgCompositeFragmentUnweighted(foreground[ii],background[ii]);
    return ret;
}

// Simple resampling (no filtering) based on a transform map:
FgImgRgbaUb
fgResample(
    const FgImage<FgVect2F> &   map,    // Resample coordinates in OTCS, with (-1,-1) as invalid
    const FgImgRgbaUb &         src);

template<class T>
void
fgRotate90(
    bool                clockwise,
    const FgImage<T> &  in,
    FgImage<T> &        out)
{
    FgVect2UI       dims(in.dims());
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

// Alpha-weight the pixels (assuming they are initially NOT):
inline
void
fgAlphaWeight(FgImgRgbaUb & img)
{
    for (size_t ii=0; ii<img.numPixels(); ++ii)
        img[ii].alphaWeight();
}

// Does any pixel contain an alpha value less than 254 ?
bool
fgUsesAlpha(const FgImgRgbaUb &,uchar minVal=254);

inline FgVect4UC fgRed() {return FgVect4UC(255,0,0,255); }
inline FgVect4UC fgGreen() {return FgVect4UC(255,0,0,255); }
inline FgVect4UC fgBlue() {return FgVect4UC(255,0,0,255); }

// Thickness must be and odd number:
void
fgPaintCrossHair(FgImgRgbaUb & img,FgVect2I ircs,FgVect4UC color=fgRed(),uint thickness=1);

void
fgPaintDot(FgImgRgbaUb & img,FgVect2I ircs,FgVect4UC color=fgRed(),uint radius=3);

void
fgPaintDot(FgImgRgbaUb & img,FgVect2F ipcs,FgVect4UC color=fgRed(),uint radius=3);

// Returns only 2-block-filtered 2-subsampled images of the original.
// Smallest is when the largest dimension is of size 1 (smallest dim clamped to size 1).
// Non power-of-2 dimensions are truncated when subsampled.
vector<FgImgRgbaUb>
fgOglMipMap(const FgImgRgbaUb & img);

// Convert, no scaling:
FgImg3F
fgImgToF3(const FgImgRgbaUb &);

// Scale space image (3 channel float). Returns image pyramid from largest (same as source but smoothed)
// to smallest dimension equal to 2. Non power of 2 dimensions are simply rounded down at each level:
FgImg3Fs
fgSsi(
    const FgImg3F & img,                // Source image
    uchar           borderPolicy=0);    // 0: border is value 0, 1: border is mirrored

// Returns the transforms from ITCS to IPCS for each corresponding SSI level given the original image dims,
// principal point and FOV. Takes into account the dimension rounding for non power of 2 dimensions:
FgAffineCw2Fs
fgSsiItcsToIpcs(FgVect2UI dims,FgVect2F principalPointIpcs,FgVect2F fovItcs);

#endif
