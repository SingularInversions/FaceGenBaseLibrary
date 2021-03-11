//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
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

// Single channel version
template<class T>
std::ostream &
operator<<(std::ostream & os,Img<T> const & img)
{
    return
        os << "dimensions: " << img.dims()
            << " bounds: " << cBounds(img.m_data);
}

// RGBA versions:
std::ostream &
operator<<(std::ostream &,ImgC4UC const &);
std::ostream &
operator<<(std::ostream &,ImgC4F const &);

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

template<typename T>
Img<T>
mapFunc(Img<T> const & in,Sfun<T(T)> const & fn)
{return Img<T>{in.dims(),mapFunc(in.m_data,fn)}; }

template<typename Out,typename In>
Img<Out>
mapFuncT(Img<In> const & in,Sfun<Out(In)> const & fn)
{return Img<Out>{in.dims(),mapFuncT<Out,In>(in.m_data,fn)}; }

// Create image coordinate affine transforms (see FgCoordSystem.hpp)
AffineEw2D          cIpcsToIucsXf(Vec2UI dims);
AffineEw2D          cIrcsToIucsXf(Vec2UI imageDims);
AffineEw2D          cIucsToIrcsXf(Vec2UI ircsDims);
AffineEw2F          cIucsToIpcsXf(Vec2UI dims);
AffineEw2F          cOicsToIucsXf();

// Transform image coordinates:
inline
Vec2F
cIucsToIrcsXf(Vec2UI ircsDims,Vec2F iucsCoord)
{return (mapMul(iucsCoord,Vec2F(ircsDims)) - Vec2F(0.5)); }

struct  CoordWgt
{
    Vec2UI      coordIrcs;  // Image coordinate
    float       wgt;        // Respective weight
};

// Bilinear interpolation co-efficients culling samples outside bounds.
// Return value can contain between 0 and 4 samples depending on how many are culled:
VArray<CoordWgt,4>
cLerpCullIrcs(Vec2UI dims,Vec2F coordIrcs);
VArray<CoordWgt,4>
cLerpCullIucs(Vec2UI dims,Vec2F coordIucs);

// Bilinear interpolation coordinates and co-efficients with coordinates clamped to image.
// Returned matrix weights sum to 1. Cols are X Lo,Hi and rows are Y Lo,Hi.
Mat<CoordWgt,2,2>
blerpCoordsClipIrcs(Vec2UI dims,Vec2D coordIrcs);
Mat<CoordWgt,2,2>
blerpCoordsClipIucs(Vec2UI dims,Vec2F coordIucs);

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
RgbaF
sampleAlpha(ImgC4UC const & img,Vec2F coordIucs);

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
fgSampleFdd1Clamp(Img<T> const & img,Vec2UI coord)
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
interpolate(Img<T> const & i0,Img<T> const & i1,float val)
{
    Vec2UI          dims = i0.dims();
    FGASSERT(i1.dims() == dims);
    Img<T>          ret(dims);
    float           omv = 1.0f - val;
    for (size_t ii=0; ii<ret.numPixels(); ++ii) {
        auto        v = mapCast<float>(i0.m_data[ii]) * omv + mapCast<float>(i1.m_data[ii]) * val;
        round_(v,ret.m_data[ii]);
    }
    return ret;
}

// Shrink an image by 2x2 averaging blocks, rounding down the image size if odd:
void
imgShrink2(ImgC4UC const & src,ImgC4UC & dst);

inline
ImgC4UC
imgShrink2(ImgC4UC const & src)
{
    ImgC4UC ret;
    imgShrink2(src,ret);
    return ret;
}

// Resampled 2x expansion. See 'magnify' for a unresampled.
ImgC4UC
expand2(ImgC4UC const & src);

// 2x image shrink using block average for types composed of floating point values.
// Truncates last row/col if width/height not even.
template<typename T>
void
shrink2Float_(
    Img<T> const &  src,
    Img<T> &        dst,    // Must be a different instance
    typename Traits<T>::Scalar denom=typename Traits<T>::Scalar(4))
{
    FGASSERT(src.data() != dst.data());
    dst.resize(src.dims()/2);
    for (uint yy=0; yy<dst.height(); ++yy)
        for (uint xx=0; xx<dst.width(); ++xx)
            dst.xy(xx,yy) = (
                src.xy(2*xx,2*yy) +
                src.xy(2*xx+1,2*yy) +
                src.xy(2*xx+1,2*yy+1) +
                src.xy(2*xx,2*yy+1)) / denom;
}

template<class T>
Img<T>
shrink2Float(Img<T> const & img)
{
    Img<T>      ret;
    shrink2Float_(img,ret);
    return ret;
}

// Fixed-point 2x2 averaging image shrink
// A row/col of pixels will be truncated if the dimensions are not even.
// Not optimized.
ImgUC shrink2Fixed(const ImgUC & img);

// Repeat each pixel value 'fac' times in both dimensions:
template<class T>
Img<T>
magnify(Img<T> const & img,size_t fac)
{
    Img<T>      ret(img.dims()*uint(fac));
    for (Iter2UI  it(ret.dims()); it.next(); it.valid())
        ret[it()] = img[it()/uint(fac)];
    return ret;
}

// Image channels must be in range [0,1]
ImgC4UC     toImgC4UC(ImgC4F const & img);

ImgC4UC
toImgC4UC(
    Img3F const &       img,
    float               offset=0.0f,        // Applied first
    float               scale=255.999f);

void
imgConvert_(ImgC4UC const & src,ImgUC & dst);

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
imgResize(
    ImgC4UC const & src,
    ImgC4UC &       dst);   // MODIFIED

void        rescaleValsToFit_(const ImgD & src,ImgUC & dst);
void        rescaleValsToFit_(const ImgD & src,ImgC4UC & dst);

ImgC4UC
fgImgApplyTransparencyPow2(
    ImgC4UC const & colour,
    ImgC4UC const & transparency);

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
void    smoothUint_(ImgC4UC const & src,ImgC4UC & dst,uchar borderPolicy=1);
ImgC4UC smoothUint(ImgC4UC const & src,uchar borderPolicy=1);

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
    dstPtr[0] = srcPtr[0]*(2+borderPolicy) + srcPtr[1];
    for (uint ii=1; ii<wid-1; ++ii)
        dstPtr[ii] = srcPtr[ii-1] + srcPtr[ii]*2 + srcPtr[ii+1];
    dstPtr[wid-1] = srcPtr[wid-2] + srcPtr[wid-1]*(2+borderPolicy);
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
    float       factor = fac*fac;
    Img<T>     acc(wid,3);
    T           *accPtr0,
                *accPtr1 = acc.rowPtr(0),
                *accPtr2 = acc.rowPtr(1);
    smoothFloat1D(srcPtr,accPtr1,wid,borderPolicy);
    srcPtr += wid;
    smoothFloat1D(srcPtr,accPtr2,wid,borderPolicy);
    for (uint xx=0; xx<wid; ++xx)
        dstPtr[xx] = (accPtr1[xx]*(2+borderPolicy) + accPtr2[xx]) * factor;
    for (uint yy=1; yy<hgt-1; ++yy) {
        dstPtr += wid;
        srcPtr += wid;
        accPtr0 = acc.rowPtr((yy-1)%3);
        accPtr1 = acc.rowPtr(yy%3);
        accPtr2 = acc.rowPtr((yy+1)%3);
        smoothFloat1D(srcPtr,accPtr2,wid,borderPolicy);
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
smoothFloat(
    Img<T> const &  src,
    Img<T> &        dst,                // Can be same as src
    uchar               borderPolicy)       // 0 - zero border policy, 1 - replication border policy
{
    FGASSERT((src.width() > 1) && (src.height() > 1));  // Algorithm not designed for dim < 2
    FGASSERT((borderPolicy == 0) || (borderPolicy == 1));
    dst.resize(src.dims());
    smoothFloat2D(src.data(),dst.data(),src.width(),src.height(),borderPolicy);
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

bool
fgImgApproxEqual(ImgC4UC const & img0,ImgC4UC const & img1,uint maxDelta=0);

template<class T>
double
cDot(Img<Rgba<T> > const & lhs,Img<Rgba<T> > const & rhs)
{
    FGASSERT(lhs.dims() == rhs.dims());
    return cDot(lhs.m_data,rhs.m_data);
}

template<class T>
double
cSsd(Img<Rgba<T> > const & lhs,Img<Rgba<T> > const & rhs)
{
    FGASSERT(lhs.dims() == rhs.dims());
    return cSsd(lhs.m_data,rhs.m_data);
}

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
ImgC4UC
composite(ImgC4UC const & foreground,ImgC4UC const & background);

// Simple resampling (no filtering) based on a transform map:
ImgC4UC
fgResample(
    const Img2F &   map,    // Resample coordinates in OTCS, with (-1,-1) as invalid
    ImgC4UC const &         src);

template<class T>
void
fgRotate90(
    bool                clockwise,
    Img<T> const &  in,
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

// Does any pixel contain an alpha value less than 254 ? (returns false if empty)
bool
usesAlpha(ImgC4UC const &,uchar minVal=254);

inline Vec4UC fgRed() {return Vec4UC(255,0,0,255); }
inline Vec4UC fgGreen() {return Vec4UC(255,0,0,255); }
inline Vec4UC fgBlue() {return Vec4UC(255,0,0,255); }

// Thickness must be an odd number:
void
paintCrosshair(ImgC4UC & img,Vec2I ircs);

void
paintDot(ImgC4UC & img,Vec2I ircs,Vec4UC color=fgRed(),uint radius=3);

void
paintDot(ImgC4UC & img,Vec2F ipcs,Vec4UC color=fgRed(),uint radius=3);

// Returns only 2-block-filtered 2-subsampled images of the original.
// Smallest is when the largest dimension is of size 1 (smallest dim clamped to size 1).
// Non power-of-2 dimensions are truncated when subsampled.
Svec<ImgC4UC>
cMipMap(ImgC4UC const & img);

// Convert, no scaling:
ImgV3F
fgImgToF3(ImgC4UC const &);

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
