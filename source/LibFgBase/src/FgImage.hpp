//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

//

#ifndef FGIMAGE_HPP
#define FGIMAGE_HPP

#include "FgImageBase.hpp"
#include "FgImageIo.hpp"
#include "FgIter.hpp"
#include "FgAffineC.hpp"
#include "FgAffineCwC.hpp"
#include "FgArray.hpp"

namespace Fg {

std::ostream &
operator<<(std::ostream &,const ImgC4UC &);

inline
AffineEw2F
fgOicsToIucs()
{return AffineEw2F(Mat22F(-1,1,-1,1),Mat22F(0,1,1,0)); }

inline
AffineEw2F
fgIucsToIpcs(Vec2UI dims)
{return AffineEw2F(Mat22F(0,1,0,1),Mat22F(0,dims[0],0,dims[1])); }

inline
AffineEw2F
calcIrcsToIucs(Vec2UI imageDims)
{return AffineEw2F(Mat22F(-0.5,imageDims[0]-0.5,-0.5,imageDims[1]-0.5),Mat22F(0,1,0,1)); }

inline
Vec2F
fgIucsToIrcs(Vec2UI ircsDims,Vec2F iucsCoord)
{return (fgMapMul(iucsCoord,Vec2F(ircsDims)) - Vec2F(0.5)); }

template<uint dim>
inline
AffineEw<float,dim>
fgIrcsToIpcs()
{return AffineEw<float,dim>(Mat<float,dim,1>(1),Mat<float,dim,1>(0.5f)); }

struct  CoordWgt
{
    Vec2UI      coordIrcs;  // Image coordinate
    float       wgt;        // Respective weight
};

// Bilinear interpolation co-efficients culling samples outside bounds.
// Return value can contain between 0 and 4 samples depending on how many are culled:
VArray<CoordWgt,4>
blerpCoordsCull(Vec2UI dims,Vec2F coordIucs);

// Bilinear interpolation coordinates and co-efficients with coordinates clamped to image.
// Returned matrix weights sum to 1. Cols are X Lo,Hi and rows are Y Lo,Hi.
Mat<CoordWgt,2,2>
blerpCoordsClip(Vec2UI dims,Vec2F coordIucs);

// Bilinear interpolation. Returns alpha-weighted linear-interpolated value.
// Source points outside image considered to have alpha zero:
RgbaF
sampleAlpha(const ImgC4UC & img,Vec2F coordIucs);

// Sample an image with coordinates clipped to image boundaries:
template<typename T>
typename Traits<T>::Floating
sampleClip(Img<T> const & img,Vec2F coordIucs)
{
    typedef typename Traits<T>::Floating    Acc;
    Acc                 ret(0);
    Mat<CoordWgt,2,2>   bc = blerpCoordsClip(img.dims(),coordIucs);
    for (uint ii=0; ii<4; ++ii) {
        CoordWgt        cw = bc.m[ii];
        ret += Acc(img[cw.coordIrcs]) * cw.wgt;
    }
    return ret;
}

template<class T>
Mat<T,2,2>
fgSampleFdd1Clamp(const Img<T> & img,Vec2UI coord)
{
    Mat<T,2,2>      ret;
    Vec2UI          dims = img.dims();
    for (uint dim=0; dim<2; ++dim) {
        for (int xx=-1; xx<2; xx+=2) {
            int         pos = int(coord[dim]) + xx,
                        max = int(dims[dim]) - 1;
            if (pos < 0)
                pos = 0;
            else if (pos > max)
                pos = max;
            Vec2UI      crd = coord;
            crd[dim] = uint(pos);
            ret.cr((xx+1)/2,dim) = img[crd];
        }
    }
    return ret;
}

template<class T>
Img<T>
fgThreshold(const Img<T> & img,T minRoundUp)
{
    Img<T>      ret(img.dims());
    for (size_t ii=0; ii<ret.m_data.size(); ++ii)
        ret.m_data[ii] = (img.m_data[ii] < minRoundUp) ? T(0) : std::numeric_limits<T>::max();
    return ret;
}

// Shrink an image by 2x2 averaging blocks, rounding down the image size if odd:
void
fgImgShrink2(const ImgC4UC & src,ImgC4UC & dst);

inline
ImgC4UC
fgImgShrink2(const ImgC4UC & src)
{
    ImgC4UC ret;
    fgImgShrink2(src,ret);
    return ret;
}

// Resampled 2x expansion. See 'magnify' for a unresampled.
ImgC4UC
fgExpand2(const ImgC4UC & src);

// 2x image shrink using block average for types composed of floating point values.
// Truncates last row/col if width/height not even.
template<typename T>
void
fgShrink2Float(
    const Img<T> &  src,
    Img<T> &        dst)    // Must be a different instance
{
    FGASSERT(src.data() != dst.data());
    dst.resize(src.dims()/2);
    for (uint yy=0; yy<dst.height(); ++yy)
        for (uint xx=0; xx<dst.width(); ++xx)
            dst.xy(xx,yy) = (
                src.xy(2*xx,2*yy) +
                src.xy(2*xx+1,2*yy) +
                src.xy(2*xx+1,2*yy+1) +
                src.xy(2*xx,2*yy+1)) / typename Traits<T>::Scalar(4);
}

template<class T>
Img<T>
fgShrink2Float(const Img<T> & img)
{
    Img<T>      ret;
    fgShrink2Float(img,ret);
    return ret;
}

// A row/col of pixels will be truncated if the dimensions are not even.
// Not yet optimized.
ImgUC fgShrink2(const ImgUC & img);

// Shrink an image by 1x2 averaging blocks, rounding down the image width if odd.
ImgC4UC
fgImgShrinkWid2(const ImgC4UC & src);

// Shrink an image by 2x1 averaging blocks, rounding down the image height if odd.
ImgC4UC
fgImgShrinkHgt2(const ImgC4UC & src);

// Repeat each pixel value 'fac' times in both dimensions:
template<class T>
Img<T>
fgImgMagnify(const Img<T> & img,size_t fac)
{
    Img<T>      ret(img.dims()*uint(fac));
    for (Iter2UI  it(ret.dims()); it.next(); it.valid())
        ret[it()] = img[it()/uint(fac)];
    return ret;
}

void
imgConvert_(const ImgC4UC & src,ImgUC & dst);

void
imgConvert_(const ImgUC & src,ImgC4UC & dst);

inline
ImgC4UC
imgUcTo4Uc(ImgUC const & img)
{
    ImgC4UC      ret;
    imgConvert_(img,ret);
    return ret;
}

// Resize to the destination image dimensions, by shrinking or expanding in each dimension.
// Samples the exact proportional amount of the source image covered by the destination image
// pixel for shrinking dimensions, which effectively uses bilinear interpolation for dilating
// dimensions:
void
fgImgResize(
    const ImgC4UC & src,
    ImgC4UC &       dst);   // MODIFIED

void
fgImgPntRescaleConvert(const ImgD & src,ImgUC & dst);

void
fgImgPntRescaleConvert(const ImgD & src,ImgC4UC & dst);

Affine2D
fgImgGetIrcsToBounds(uint wid,uint hgt,const Mat22D & bounds);

ImgC4UC
fgImgApplyTransparencyPow2(
    const ImgC4UC & colour,
    const ImgC4UC & transparency);

Img<FgBool>
fgAnd(const Img<FgBool> &,const Img<FgBool> &);

template<class T>
void
fgImgFlipVertical(Img<T> & img)
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
Img<T>
fgFlipHoriz(const Img<T> & img)
{
    Img<T>  ret(img.dims());
    uint        xh = img.width()-1;
    for (uint yy=0; yy<img.height(); ++yy) {
        for (uint xx=0; xx<img.width(); ++xx)
            ret.xy(xh-xx,yy) = img.xy(xx,yy); }
    return ret;
}

template<class T>
inline void
fgImgFill(
    Img<T> &    img,
    T               val)
{
    for (size_t ii=0; ii<img.numPixels(); ++ii)
        img[ii] = val;
}

// Note that 'trans' is applied BEFORE scale, unlike homogenous representation, since this is
// usually more convenient & precise for what needs to be done.
template<class T>
void
fgImgPntAffine(const Img<T>& in,Img<T>& out,T trans,T scale)
{
    out.resize(in.width(),in.height());
    for (size_t ii=0; ii<out.numPixels(); ++ii)
        out[ii] = (in[ii] + trans) * scale;
}

// Accumulators for convolution:
template<class T> struct FgConvTraits;
template<> struct FgConvTraits<uchar>       {typedef ushort     Acc; };
template<> struct FgConvTraits<RgbaUC>    {typedef RgbaUS   Acc; };
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
    const Img<T> &  src,
    Img<T> &        dst,
    uchar               borderPolicy=1)     // 0 - zero border policy, 1 - replication border policy
{
    typedef typename FgConvTraits<T>::Acc     Acc;
    FGASSERT((src.width() > 1) && (src.height() > 1));  // Algorithm not designed for dim < 2
    FGASSERT((borderPolicy == 0) || (borderPolicy == 1));
    dst.resize(src.width(),src.height());
    Img<Acc> acc(src.width(),3);                  // Accumulator image
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
Img<T>
fgSmoothUint(const Img<T> & src,uchar borderPolicy=1)
{
    Img<T>      ret;
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
    Img<T>     acc(wid,3);
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
    const Img<T> &  src,
    Img<T> &        dst,                // Can be same as src
    uchar               borderPolicy)       // 0 - zero border policy, 1 - replication border policy
{
    FGASSERT((src.width() > 1) && (src.height() > 1));  // Algorithm not designed for dim < 2
    FGASSERT((borderPolicy == 0) || (borderPolicy == 1));
    dst.resize(src.dims());
    fgSmoothFloat2D(src.data(),dst.data(),src.width(),src.height(),borderPolicy);
}

// Only defined for binarized images, output is binarized:
template<class T>
Img<T>
fgDilate(const Img<T> & img)
{
    Img<T>      ret = fgSmoothUint(img);
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
    const Mat33F & krn,
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
    const Img<Pixel> &  src,
    const Mat33F &     krn,                // The kernel to be correlated
    Img<Pixel> &        dst,                // Must be different from src
    uchar                   borderPolicy)       // 0 - zero border policy, 1 - replication border policy
{
    FGASSERT((src.width() > 1) && (src.height() > 1));  // Algorithm not designed for dim < 2
    FGASSERT((borderPolicy == 0) || (borderPolicy == 1));
    dst.resize(src.dims());
    FGASSERT(src.data() != dst.data());
    uint                    wid = src.width();
    Svec<Pixel>           boundaryRow(wid,Pixel(0));
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
    const Img<T> &  in,
    Img<T> &        out)
{
    if (in.dims().cmpntsProduct() == 0)
        out.clear();
    else {
        FGASSERT(fgMinElem(out.dims()) > 0);
        Mat22F        inBoundsIucs( 0.0f,1.0f,    // sampleClip takes UICS
                                      0.0f,1.0f),
                        outBoundsIrcs(-0.5f,float(out.width())-0.5f,
                                      -0.5f,float(out.height())-0.5f);
        AffineEw2F    o2i(outBoundsIrcs,inBoundsIucs);
        for (Iter2UI it(out.dims()); it.valid(); it.next()) {
            Vec2F    pt = o2i * Vec2F(it());
            round_(sampleClip(in,pt),out[it]);
        }
    }
}
template<class T>
Img<T>
fgResampleSimple(const Img<T> & in,Vec2UI dims)
{
    Img<T>      ret(dims);
    fgResampleSimple(in,ret);
    return ret;
}

template<class T>
void
fgResizePow2Ceil_(const Img<T> & in,Img<T> & out)
{
    if (!isPow2(in.dims())) {
        out.resize(pow2Ceil(in.dims()));
        fgResampleSimple(in,out);
    }
    else
        out = in;
}

template<class T>
Img<T>
fgResizePow2Ceil(const Img<T> & img)
{
    Img<T>      ret;
    fgResizePow2Ceil_(img,ret);
    return ret;
}

bool
fgImgApproxEqual(const ImgC4UC & img0,const ImgC4UC & img1,uint maxDelta=0);

template<class T>
double
fgImgSsd(
    const Img<Rgba<T> > & im0,
    const Img<Rgba<T> > & im1)
{
    FGASSERT(im0.dims() == im1.dims());
    double      acc = 0.0;
    for (Iter2UI it(im0.dims()); it.valid(); it.next())
    {
        Vec4D        p0,p1;
        scast_(im0[it].m_c,p0);
        scast_(im1[it].m_c,p1);
        acc += (p0-p1).mag();
    }
    return acc;
}

template<class T>
Img<T>
fgJoinHoriz(const Img<T> & l,const Img<T> & r)
{
    Img<T>      ret;
    if (l.empty())
        ret = r;
    else if (r.empty())
        ret = l;
    else {
        FGASSERT(l.height() == r.height());
        ret.resize(l.width()+r.width(),l.height());
        Vec2UI       off;
        for (Iter2UI it(l.dims()); it.valid(); it.next())
            ret[it()] = l[it()];
        off[0] += l.dims()[0];
        for (Iter2UI it(r.dims()); it.valid(); it.next())
            ret[it()+off] = r[it()];
    }
    return ret;
}

template<class T>
Img<T>
fgJoinHoriz(const Img<T> & l,const Img<T> & c,const Img<T> & r)
{
    Img<T>      ret;
    if (l.empty())
        ret = fgJoinHoriz(c,r);
    else if (c.empty())
        ret = fgJoinHoriz(l,r);
    else if (r.empty())
        ret = fgJoinHoriz(l,c);
    else {
        FGASSERT((l.height() == c.height()) && (c.height() == r.height()));
        ret.resize(l.width()+c.width()+r.width(),l.height());
        Vec2UI   off;
        for (Iter2UI it(l.dims()); it.valid(); it.next())
            ret[it()] = l[it()];
        off[0] += l.dims()[0];
        for (Iter2UI it(c.dims()); it.valid(); it.next())
            ret[it()+off] = c[it()];
        off[0] += c.dims()[0];
        for (Iter2UI it(r.dims()); it.valid(); it.next())
            ret[it()+off] = r[it()];
    }
    return ret;
}

template<class T>
Img<T>
fgJoinVert(const Img<T> & t,const Img<T> & b)
{
    Img<T>      ret;
    if (t.empty())
        ret = b;
    else if (b.empty())
        ret = t;
    else {
        FGASSERT(t.width() == b.width());
        ret.resize(t.width(),t.height()+b.height());
        Vec2UI   off;
        for (Iter2UI it(t.dims()); it.valid(); it.next())
            ret[it()] = t[it()];
        off[1] += t.dims()[1];
        for (Iter2UI it(b.dims()); it.valid(); it.next())
            ret[it()+off] = b[it()];
    }
    return ret;
}

template<class T>
Img<T>
fgJoinVert(const Svec<Img<T> > & v)
{
    Img<T>  ret;
    for (size_t ii=0; ii<v.size(); ++ii)
        ret = fgJoinVert(ret,v[ii]);
    return ret;
}

template<class T>
Img<T>
fgCropPad(
    const Img<T> &  src,
    Vec2UI           dims,
    Vec2I            offset = Vec2I(0),
    T                   fill = T())
{
    Img<T>      ret(dims,fill);
    if (!src.empty()) {
        Mat22I        srcBnds = Mat22I(fgRangeToBounds(src.dims())),
                        dstBnds = Mat22I(fgRangeToBounds(dims)),
                        range = fgBoundsIntersection(srcBnds-fgJoinHoriz(offset,offset),dstBnds);
        for (Iter2I it(fgInclToExcl(range)); it.valid(); it.next())
            ret[Vec2UI(it())] = src[Vec2UI(it()+offset)];
    }
    return ret;
}

// Requires alpha-weighted color values:
template<class T>
Img<T>
fgComposite(
    const Img<T> &  foreground,
    const Img<T> &  background)
{
    FGASSERT(foreground.dims() == background.dims());
    Img<T>      ret(foreground.dims());
    for (size_t ii=0; ii<ret.numPixels(); ++ii)
        ret[ii] = fgCompositeFragmentUnweighted(foreground[ii],background[ii]);
    return ret;
}

// Simple resampling (no filtering) based on a transform map:
ImgC4UC
fgResample(
    const Img2F &   map,    // Resample coordinates in OTCS, with (-1,-1) as invalid
    const ImgC4UC &         src);

template<class T>
void
fgRotate90(
    bool                clockwise,
    const Img<T> &  in,
    Img<T> &        out)
{
    Vec2UI       dims(in.dims());
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
fgAlphaWeight(ImgC4UC & img)
{
    for (size_t ii=0; ii<img.numPixels(); ++ii)
        img[ii].alphaWeight();
}

// Does any pixel contain an alpha value less than 254 ? (returns false if empty)
bool
fgUsesAlpha(const ImgC4UC &,uchar minVal=254);

inline Vec4UC fgRed() {return Vec4UC(255,0,0,255); }
inline Vec4UC fgGreen() {return Vec4UC(255,0,0,255); }
inline Vec4UC fgBlue() {return Vec4UC(255,0,0,255); }

// Thickness must be and odd number:
void
fgPaintCrossHair(ImgC4UC & img,Vec2I ircs,Vec4UC color=fgRed(),uint thickness=1);

void
fgPaintDot(ImgC4UC & img,Vec2I ircs,Vec4UC color=fgRed(),uint radius=3);

void
fgPaintDot(ImgC4UC & img,Vec2F ipcs,Vec4UC color=fgRed(),uint radius=3);

// Returns only 2-block-filtered 2-subsampled images of the original.
// Smallest is when the largest dimension is of size 1 (smallest dim clamped to size 1).
// Non power-of-2 dimensions are truncated when subsampled.
Svec<ImgC4UC>
fgMipMap(const ImgC4UC & img);

// Convert, no scaling:
Img3F
fgImgToF3(const ImgC4UC &);

// Scale space image (3 channel float). Returns image pyramid from largest (same as source but smoothed)
// to smallest dimension equal to 2. Non power of 2 dimensions are simply rounded down at each level:
Img3Fs
fgSsi(
    const Img3F & img,                // Source image
    uchar           borderPolicy=0);    // 0: border is value 0, 1: border is mirrored

// Returns the transforms from ITCS to IPCS for each corresponding SSI level given the original image dims,
// principal point and FOV. Takes into account the dimension rounding for non power of 2 dimensions:
AffineEw2Fs
fgSsiItcsToIpcs(Vec2UI dims,Vec2F principalPointIpcs,Vec2F fovItcs);

// Blend images given a greyscale transition map [0,255] : 0 -> 'img0', 255 -> 'img1'
// The returned image is the size of 'img0' and 'img1' and 'transition' are bilinearly sampled.
// Any of the images can be empty:
ImgC4UC
imgBlend(ImgC4UC const & img0,ImgC4UC const & img1,ImgUC const & transition);

// Modulate the color channels of an image with a modulation map scaled such that identity (1.0) = 64.
// Only the color channels of modulationMap are used. The input images may be different sizes but must
// have identical aspect ratios. Either image may be empty.
ImgC4UC
imgModulate(
    ImgC4UC const &     colorImage,             // Alpha left unchanged
    ImgC4UC const &     modulationMap,          // RGB channels modulate respective channels in 'colorImage'. Alpha ignored.
    float               modulationFactor=1.0f); // [0.5,1.5] modulate the modulation values.

}

#endif
