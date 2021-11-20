//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// NOTES:
// 
// * All pixel averaging / resampling operations on images with an alpha-channel require the color
//   channels to be alpha-weighted for correct results. Alpha weighting does not commute with linear
//   composition, and only the pre-weighted order corresponds to meaningful results.

#ifndef FGIMAGE_HPP
#define FGIMAGE_HPP

#include "FgImageBase.hpp"
#include "FgImageIo.hpp"
#include "FgIter.hpp"
#include "FgAffine.hpp"
#include "FgArray.hpp"

namespace Fg {

// OSTREAM OUTPUT:

template<class T>
std::ostream &
operator<<(std::ostream & os,Img<T> const & img)
{
    return
        os << "dimensions: " << img.dims()
            << " bounds: " << cBounds(img.m_data);
}
std::ostream &      operator<<(std::ostream &,ImgRgba8 const &);
std::ostream &      operator<<(std::ostream &,ImgC4F const &);

// COORDINATES, SAMPLING, INTERPOLATION:

AffineEw2D          cIpcsToIucsXf(Vec2UI dims);
AffineEw2D          cIrcsToIucsXf(Vec2UI imageDims);
AffineEw2F          cIucsToIrcsXf(Vec2UI ircsDims);
AffineEw2F          cIucsToIpcsXf(Vec2UI dims);
AffineEw2F          cOicsToIucsXf();
inline Vec2F        cIucsToIrcs(Vec2UI ircsDims,Vec2F iucsCoord) {return (mapMul(iucsCoord,Vec2F(ircsDims)) - Vec2F(0.5)); }

struct  CoordWgt
{
    Vec2UI      coordIrcs;  // Image coordinate
    float       wgt;        // Respective weight
};

// Bilinear interpolation co-efficients culling samples outside bounds.
// Return value can contain between 0 and 4 samples depending on how many are culled:
VArray<CoordWgt,4>  cLerpCullIrcs(Vec2UI dims,Vec2F coordIrcs);
VArray<CoordWgt,4>  cLerpCullIucs(Vec2UI dims,Vec2F coordIucs);

// Bilinear interpolation coordinates and co-efficients with coordinates clamped to image.
// Returned matrix weights sum to 1. Cols are X Lo,Hi and rows are Y Lo,Hi.
Mat<CoordWgt,2,2>   blerpCoordsClipIrcs(Vec2UI dims,Vec2D coordIrcs);
Mat<CoordWgt,2,2>   blerpCoordsClipIucs(Vec2UI dims,Vec2F coordIucs);

template<class I>
typename I::PixelType
sampleCull(I const & img,Vec2D ircs)
{
    VArray<CoordWgt,4>     lerp = cLerpCullIrcs(img.dims(),Vec2F(ircs));
    typename I::PixelType ret(0);
    for (uint ii=0; ii<lerp.size(); ++ii)
        ret += img[lerp[ii].coordIrcs] * lerp[ii].wgt;
    return ret;
}

// Bilinear interpolated alpha-weighted RGBA image sampling.
// Source points outside image considered to have alpha zero:
RgbaF               sampleAlpha(ImgRgba8 const & img,Vec2F coordIucs);

// Sample an image with given matrix of coordinates (must be in image bounds) and weights:
template<typename T>
typename Traits<T>::Floating
sampleClip(Img<T> const & img,Mat<CoordWgt,2,2> const & cws)
{
    typedef typename Traits<T>::Floating    Acc;
    Acc                 ret(0);
    for (CoordWgt const & cw : cws.m)
        ret += Acc(img[cw.coordIrcs]) * cw.wgt;
    return ret;
}

// Bilinear image sample clamped to image bounds:
template<typename T>
typename Traits<T>::Floating
sampleClipIucs(Img<T> const & img,Vec2F coordIucs)
{return sampleClip(img,blerpCoordsClipIucs(img.dims(),coordIucs)); }

// Bilinear image sample clamped to image bounds:
template<typename T>
typename Traits<T>::Floating
sampleClipIrcs(Img<T> const & img,Vec2D coordIrcs)
{return sampleClip(img,blerpCoordsClipIrcs(img.dims(),coordIrcs)); }

template<class T>
Mat<T,2,2>
sampleFdd1Clamp(Img<T> const & img,Vec2UI coord)
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

// ELEMENT-WISE OPERATIONS:

// Conversions. Note that the value 255U is only used for 1.0f so the effective 8-bit range is only 255 values.
// This is done to allow preservation of max value in round-trip conversions:
Img3F               toUnit3F(ImgRgba8 const &);         // [0,255] -> [0,1], ignores input alpha channel
Img4F               toUnit4F(ImgRgba8 const &);         // [0,255] -> [0,1]
ImgRgba8            toRgba8(Img3F const &,float maxVal=1.0);    // [0,maxVal] -> [0,255], alpha set to 255
ImgRgba8            toRgba8(ImgC4F const &);            // [0,1] -> [0,255]
ImgUC               toUC(ImgRgba8 const &);             // rec. 709 RGB -> greyscale
ImgF                toFloat(ImgRgba8 const &);          // rec. 709 RGB -> greyscale [0,255]
ImgRgba8            toRgba8(ImgUC const &);             // replicate to RGB, set alpha to 255

template<typename T>
Img<T>
operator+(Img<T> const & lhs,Img<T> const & rhs)
{
    FGASSERT(lhs.dims() == rhs.dims());
    return Img<T>(lhs.dims(),lhs.m_data+rhs.m_data);
}

template<typename T>
Img<T>
operator-(Img<T> const & lhs,Img<T> const & rhs)
{
    FGASSERT(lhs.dims() == rhs.dims());
    return Img<T>(lhs.dims(),lhs.m_data-rhs.m_data);
}

template<class T,class U>
void
operator*=(Img<T> & img,U rhs)
{img.m_data *= rhs; }

template<class T>
double
cDot(Img<T> const & lhs,Img<T> const & rhs)
{
    FGASSERT(lhs.dims() == rhs.dims());
    return cDot(lhs.m_data,rhs.m_data);
}

template<class T>
double
cSsd(Img<T> const & lhs,Img<T> const & rhs)
{
    FGASSERT(lhs.dims() == rhs.dims());
    return cSsd(lhs.m_data,rhs.m_data);
}

template<typename T,typename F>
void
mapCall_(Img<T> & img,F const & fn,bool multithread)
{
    if (multithread) {
        uint                numThreads = cMin(std::thread::hardware_concurrency(),img.height()),
                            eubX = img.width();
        Svec<std::thread>   threads; threads.reserve(numThreads);
        auto                tfn = [eubX,&fn,&img](uint yy,uint eubY)
        {
            for (; yy<eubY; ++yy)
                for (uint xx=0; xx<eubX; ++xx)
                    img.xy(xx,yy) = fn(Vec2UI(xx,yy));
        };
        for (uint tt=0; tt<numThreads; ++tt) {
            uint            yy = (tt * img.height()) / numThreads,
                            eubY = ((tt+1) * img.height()) / numThreads;
            threads.emplace_back(tfn,yy,eubY);
        }
        for (std::thread & t : threads)
            t.join();
    }
    else {
        for (Iter2UI it{img.dims()}; it.valid(); it.next())
            img[it()] = fn(it());
    }
}

template<typename T,typename Fn>
Img<T>
mapCall(Img<T> const & in,Fn const & fn)
{return Img<T>{in.dims(),mapCall(in.m_data,fn)}; }

template<typename T,typename Fn>
Img<T>
mapCall(Img<T> const & l,Img<T> const & r,Fn const & fn)
{
    FGASSERT(l.dims() == r.dims());
    return Img<T>{l.dims(),mapCall(l.m_data,r.m_data,fn)};
}

template<class Out,class In,class Fn>
Img<Out>
mapCallT(Img<In> const & in,Fn const & fn)
{return Img<Out>{in.dims(),mapCallT<Out,In,Fn>(in.m_data,fn)}; }

template<class Out,class In0,class In1,class Fn>
Img<Out>
mapCallT(Img<In0> const & in0,Img<In1> const & in1,Fn const & fn)
{
    FGASSERT(in0.dims() == in1.dims());
    return Img<Out>{in0.dims(),mapCallT<Out,In0,In1,Fn>(in0.m_data,in1.m_data,fn)};
}

template<class Out,class In0,class In1,class In2,class Fn>
Img<Out>
mapCallT(Img<In0> const & in0,Img<In1> const & in1,Img<In2> const & in2,Fn const & fn)
{
    FGASSERT(in0.dims() == in1.dims() && in1.dims() == in2.dims());
    return Img<Out>{in0.dims(),mapCallT<Out,In0,In1,In2,Fn>(in0.m_data,in1.m_data,in2.m_data,fn)};
}

// Interpolate between 2 images:
template<class T>
Img<T>
interpolate(Img<T> const & i0,Img<T> const & i1,float ratio)
{
    Vec2UI          dims = i0.dims();
    FGASSERT(i1.dims() == dims);
    Img<T>          ret(dims);
    float           omv = 1.0f - ratio;
    for (size_t ii=0; ii<ret.numPixels(); ++ii) {
        auto        v = mapCast<float>(i0.m_data[ii]) * omv + mapCast<float>(i1.m_data[ii]) * ratio;
        round_(v,ret.m_data[ii]);
    }
    return ret;
}

// RESIZE / TRANSFORM / CONVOLVE:

// Subsample image 2x with 2x2 box filter, rounding down the image size when odd.
// Channels must be linear (eg. alpha-weighted if alpha exists):

// RGBA version:
void            shrink2_(ImgRgba8 const & src,ImgRgba8 & dst);
inline ImgRgba8  shrink2(ImgRgba8 const & src)
{
    ImgRgba8             ret;
    shrink2_(src,ret);
    return ret;
}
// floating point versions:
template<typename T,
    // Floating point specialization doesn't use a different accumulator type:
    FG_ENABLE_IF(typename Traits<T>::Scalar,is_floating_point)>
void
shrink2_(
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
template<class T,
    FG_ENABLE_IF(typename Traits<T>::Scalar,is_floating_point)>
Img<T>
shrink2(Img<T> const & img)
{
    Img<T>                  ret;
    shrink2_(img,ret);
    return ret;
}
// uchar version:
ImgUC shrink2Fixed(const ImgUC & img);

// Resampled 2x expansion. See 'magnify' for a unresampled.
ImgRgba8
expand2(ImgRgba8 const & src);

template<class T>
Img<T>
magnify(Img<T> const & img,uint fac)
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
void
imgResize(
    ImgRgba8 const & src,
    ImgRgba8 &       dst);   // MODIFIED

ImgRgba8
fgImgApplyTransparencyPow2(
    ImgRgba8 const & colour,
    ImgRgba8 const & transparency);

Img<FatBool>
mapAnd(const Img<FatBool> &,const Img<FatBool> &);

template<class T>
void
flipVertical_(Img<T> & img)
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
flipHoriz(Img<T> const & img)
{
    Img<T>  ret(img.dims());
    uint        xh = img.width()-1;
    for (uint yy=0; yy<img.height(); ++yy) {
        for (uint xx=0; xx<img.width(); ++xx)
            ret.xy(xh-xx,yy) = img.xy(xx,yy); }
    return ret;
}

// Applies a [1 2 1] outer product 2D kernel smoothing using border replication to an
// UNISGNED INTEGER channel image in a preicsion-friendly, cache-friendly way.
// The Source and desination images can be the same, for in-place convolution:
void    smoothUint_(ImgUC const & src,ImgUC & dst,uchar borderPolicy=1);
ImgUC   smoothUint(ImgUC const & src,uchar borderPolicy=1);
void    smoothUint_(ImgRgba8 const & src,ImgRgba8 & dst,uchar borderPolicy=1);
ImgRgba8 smoothUint(ImgRgba8 const & src,uchar borderPolicy=1);

// Use of the __restrict keyword for the pointer args below made no speed difference here (msvc2012).
// Perhaps the compiler is smart enough to look at the calling context:
template<class T>
void
smoothFloat1D(
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
void
smoothFloat2D(
    T const *   srcPtr,
    T *         dstPtr,         // Must not overlap with srcPtr
    uint        wid,
    uint        hgt,
    uchar       borderPolicy,   // See below
    float       fac=1.0f/4.0f)  // Per-axis kernel normalization factor
{
    typedef typename Traits<T>::Scalar   Scalar;
    float       factor = fac*fac;
    Img<T>     acc(wid,3);
    T           *accPtr0,
                *accPtr1 = acc.rowPtr(0),
                *accPtr2 = acc.rowPtr(1);
    smoothFloat1D(srcPtr,accPtr1,wid,borderPolicy);
    srcPtr += wid;
    smoothFloat1D(srcPtr,accPtr2,wid,borderPolicy);
    for (uint xx=0; xx<wid; ++xx)
        dstPtr[xx] = (accPtr1[xx]*Scalar(2+borderPolicy) + accPtr2[xx]) * factor;
    for (uint yy=1; yy<hgt-1; ++yy) {
        dstPtr += wid;
        srcPtr += wid;
        accPtr0 = acc.rowPtr((yy-1)%3);
        accPtr1 = acc.rowPtr(yy%3);
        accPtr2 = acc.rowPtr((yy+1)%3);
        smoothFloat1D(srcPtr,accPtr2,wid,borderPolicy);
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
void
smoothFloat(
    Img<T> const &  src,
    Img<T> &        dst,                // Can be same as src
    uchar           borderPolicy)       // 0 - zero border policy, 1 - replication border policy
{
    FGASSERT((src.width() > 1) && (src.height() > 1));  // Algorithm not designed for dim < 2
    FGASSERT((borderPolicy == 0) || (borderPolicy == 1));
    dst.resize(src.dims());
    smoothFloat2D(src.dataPtr(),dst.dataPtr(),src.width(),src.height(),borderPolicy);
}

// Only defined for binarized images, output is binarized:
template<class T>
Img<T>
fgDilate(Img<T> const & img)
{
    Img<T>      ret = smoothUint(img);
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
    FGASSERT(src.dataPtr() != dst.dataPtr());
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

// Preserves intrinsic aspect ratio, scales to minimally cover output dimensions.
// Returns transform from output image IRCS to input image IRCS (ie. inverse transform for resampling):
AffineEw2D
imgScaleToCover(Vec2UI inDims,Vec2UI outDims);

// Intrinsic aspect ratio is warped for an exact fit:
// Returns transform from output image IRCS to input image IRCS (ie. inverse transform for resampling):
AffineEw2D
imgScaleToFit(Vec2UI inDims,Vec2UI outDims);

// Resample an image to the given size with the given inverse transform with boundary clip policy:
template<class T>
Img<T>
resample(Img<T> const & in,Vec2UI outDims,AffineEw2D outToInIrcs)
{
    Img<T>              ret {outDims};
    if (outDims.cmpntsProduct() == 0)
        return ret;
    FGASSERT(in.dims().cmpntsProduct() > 0);
    for (Iter2UI it(outDims); it.valid(); it.next()) {
        Vec2D           inIrcs = outToInIrcs * Vec2D{it()};
        round_(sampleClipIrcs(in,inIrcs),ret[it()]);
    }
    return ret;
}

// Note that the image will be distorted if aspect ratio changes:
template<class T>
Img<T>
resampleToFit(Img<T> const & in,Vec2UI dims)
{return resample(in,dims,imgScaleToFit(in.dims(),dims)); }

// Resamples a square area of an input image into the given pixel size.
// Uses simple resampling but shrinks input image first if necessary to avoid undersampling.
// Out of bounds samples clamped to boundary values:
Img3F
resampleAdaptive(
    Img3F                   in,
    Vec2D                   posIpcs,        // top left corner of output image area
    float                   inSize,         // pixel size of square input domain
    uint                    outSize);       // pixel size of square output image

bool
fgImgApproxEqual(ImgRgba8 const & img0,ImgRgba8 const & img1,uint maxDelta=0);

template<class T>
Img<T>
catHoriz(Img<T> const & left,Img<T> const & right)
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
Img<T>
catHoriz(Img<T> const & i0,Img<T> const & i1,Img<T> const & i2)
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
void
catVertical_(Img<T> & top,Img<T> const & append)
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
Img<T>
catVertical(Img<T> const & top,Img<T> const & bottom)
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
Img<T>
catVertical(Svec<Img<T> > const & imgs)
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
Img<T>
cropPad(
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
ImgRgba8         composite(ImgRgba8 const & foreground,ImgRgba8 const & background);

// Simple resampling (no filtering) based on a transform map:
ImgRgba8
fgResample(
    const Img2F &   map,    // Resample coordinates in OTCS, with (-1,-1) as invalid
    ImgRgba8 const &         src);

template<class T>
void
rotate90(bool clockwise,Img<T> const & in,Img<T> & out)
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
bool
usesAlpha(ImgRgba8 const &,uchar minVal=254);

inline Vec4UC fgRed() {return Vec4UC(255,0,0,255); }
inline Vec4UC fgGreen() {return Vec4UC(255,0,0,255); }
inline Vec4UC fgBlue() {return Vec4UC(255,0,0,255); }

// Thickness must be an odd number:
void
paintCrosshair(ImgRgba8 & img,Vec2I ircs);

void
paintDot(ImgRgba8 & img,Vec2I ircs,Vec4UC color=fgRed(),uint radius=3);

void
paintDot(ImgRgba8 & img,Vec2F ipcs,Vec4UC color=fgRed(),uint radius=3);

// Create a mipmap (2-box-filtered 2-subsamples image pyrmamid), in which:
// * The first element is the original image
// * Each subsequent element is half the dimensions (rounded down for odd parent dimensions)
// * The last element is when the smaller dimension size has reached 1
// * If the input image is empty, an empty pyramid will be returned
template<class T>
Svec<Img<T>>
cMipmap(Img<T> const & img)
{
    Svec<Img<T>>            ret;
    uint                    minDim = cMinElem(img.dims());
    if (minDim == 0)
        return ret;
    uint                    sz = log2Floor(minDim) + 1;
    ret.reserve(sz);
    ret.push_back(img);
    while (cMinElem(ret.back().dims()) > 1)
        ret.push_back(shrink2(ret.back()));
    return ret;
}

// Convert, no scaling:
ImgV3F
fgImgToF3(ImgRgba8 const &);

// Scale space image (3 channel float). Returns image pyramid from largest (same as source but smoothed)
// to smallest dimension equal to 2. Non power of 2 dimensions are simply rounded down at each level:
ImgV3Fs
fgSsi(
    const ImgV3F & img,                // Source image
    uchar           borderPolicy=0);    // 0: border is value 0, 1: border is mirrored

// Returns the transforms from ITCS to IPCS for each corresponding SSI level given the original image dims,
// principal point and FOV. Takes into account the dimension rounding for non power of 2 dimensions:
AffineEw2Fs
fgSsiItcsToIpcs(Vec2UI dims,Vec2F principalPointIpcs,Vec2F fovItcs);

// Blend images given a greyscale transition map [0,255] : 0 -> 'img0', 255 -> 'img1'
// The returned image is the size of 'img0' and 'img1' and 'transition' are bilinearly sampled.
// Any of the images can be empty:
ImgRgba8
imgBlend(ImgRgba8 const & img0,ImgRgba8 const & img1,ImgUC const & transition);

// Modulate the color channels of an image with a modulation map scaled such that identity (1.0) = 64.
// Only the color channels of modulationMap are used. The input images may be different sizes but must
// have identical aspect ratios. Either image may be empty.
ImgRgba8
imgModulate(
    ImgRgba8 const &     colorImage,             // Alpha left unchanged
    ImgRgba8 const &     modulationMap,          // RGB channels modulate respective channels in 'colorImage'. Alpha ignored.
    float               modulationFactor=1.0f); // [0.5,1.5] modulate the modulation values.

template<class T>
Img<T>
outerProduct(Svec<T> const & x,Svec<T> const & y)
{
    Vec2UI          dims(Vec2SZ{x.size(),y.size()});
    Img<T>          ret(dims);
    for (Iter2UI it(dims); it.valid(); it.next())
        ret[it()] = x[it()[0]] * y[it()[1]];
    return ret;
}

}

#endif
