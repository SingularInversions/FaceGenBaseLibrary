//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgImage.hpp"
#include "FgMath.hpp"
#include "FgTime.hpp"
#include "FgScaleTrans.hpp"

using namespace std;

namespace Fg {

std::ostream &
operator<<(std::ostream & os,ImgRgba8 const & img)
{
    if (img.empty())
        return os << "Empty image";
    Mat<uchar,4,1>    init {img[0].m_c};
    Mat<uchar,4,2>    bounds = catHoriz(init,init);
    for (Iter2UI it(img.dims()); it.next(); it.valid()) {
        Rgba8    pix = img[it()];
        for (uint cc=0; cc<4; ++cc) {
            updateMin_(bounds.rc(cc,0),pix[cc]);
            updateMax_(bounds.rc(cc,1),pix[cc]);
        }
    }
    return os <<
        fgnl << "Dimensions: " << img.dims() <<
        fgnl << "Channel bounds: " << Mat<uint,4,2>(bounds);
}

std::ostream &
operator<<(std::ostream & os,ImgC4F const & img)
{
    if (img.empty())
        return os << "Empty image";
    Mat<float,4,1>    init {img[0].m_c};
    Mat<float,4,2>    bounds = catHoriz(init,init);
    for (Iter2UI it(img.dims()); it.next(); it.valid()) {
        RgbaF         pix = img[it()];
        for (uint cc=0; cc<4; ++cc) {
            updateMin_(bounds.rc(cc,0),pix[cc]);
            updateMax_(bounds.rc(cc,1),pix[cc]);
        }
    }
    return os <<
        fgnl << "Dimensions: " << img.dims() <<
        fgnl << "Channel bounds: " << bounds;
}

AffineEw2D
cIpcsToIucsXf(Vec2UI dims)
{return {Mat22D(0,dims[0],0,dims[1]),Mat22D(0,1,0,1)}; }

AffineEw2D
cIrcsToIucsXf(Vec2UI imageDims)
{return {Mat22D{-0.5,imageDims[0]-0.5,-0.5,imageDims[1]-0.5},Mat22D{0,1,0,1}}; }

AffineEw2F
cIucsToIpcsXf(Vec2UI dims)
{return AffineEw2F(Mat22F(0,1,0,1),Mat22F(0,dims[0],0,dims[1])); }

AffineEw2F
cIucsToIrcsXf(Vec2UI ircsDims)
{return {Mat22F{0,1,0,1},Mat22F{-0.5f,ircsDims[0]-0.5f,-0.5f,ircsDims[1]-0.5f}}; }

AffineEw2F
cOicsToIucsXf()
{return AffineEw2F(Mat22F(-1,1,-1,1),Mat22F(0,1,1,0)); }

Img<FatBool>
mapAnd(const Img<FatBool> & lhs,const Img<FatBool> & rhs)
{
    FGASSERT(lhs.dims() == rhs.dims());
    Img<FatBool>     ret(lhs.dims());
    for (Iter2UI it(ret.dims()); it.valid(); it.next())
        ret[it()] = lhs[it()] && rhs[it()];
    return ret;
}

VArray<CoordWgt,4>
cLerpCullIrcs(Vec2UI dims,Vec2F coordIrcs)
{
    VArray<CoordWgt,4>   ret;
    int         xil = int(std::floor(coordIrcs[0])),
                yil = int(std::floor(coordIrcs[1])),
                xih = xil + 1,
                yih = yil + 1,
                wid(dims[0]),
                hgt(dims[1]);
    float       wxh = coordIrcs[0] - float(xil),
                wyh = coordIrcs[1] - float(yil),
                wxl = 1.0f - wxh,
                wyl = 1.0f - wyh;
    if ((yil >= 0) && (yil < hgt)) {
        if ((xil >= 0) && (xil < wid)) {
            ret.m[ret.sz].coordIrcs[0] = uint(xil);
            ret.m[ret.sz].coordIrcs[1] = uint(yil);
            ret.m[ret.sz].wgt = wxl * wyl;
            ++ret.sz;
        }
        if ((xih >= 0) && (xih < wid)) {
            ret.m[ret.sz].coordIrcs[0] = uint(xih);
            ret.m[ret.sz].coordIrcs[1] = uint(yil);
            ret.m[ret.sz].wgt = wxh * wyl;
            ++ret.sz;
        }
    }
    if ((yih >= 0) && (yih < hgt)) {
        if ((xil >= 0) && (xil < wid)) {
            ret.m[ret.sz].coordIrcs[0] = uint(xil);
            ret.m[ret.sz].coordIrcs[1] = uint(yih);
            ret.m[ret.sz].wgt = wxl * wyh;
            ++ret.sz;
        }
        if ((xih >= 0) && (xih < wid)) {
            ret.m[ret.sz].coordIrcs[0] = uint(xih);
            ret.m[ret.sz].coordIrcs[1] = uint(yih);
            ret.m[ret.sz].wgt = wxh * wyh;
            ++ret.sz;
        }
    }
    return ret;
}

VArray<CoordWgt,4>
cLerpCullIucs(Vec2UI dims,Vec2F coordIucs)
{
    VArray<CoordWgt,4>   ret;
    float       xf = coordIucs[0] * float(dims[0]) - 0.5f,     // IRCS
                yf = coordIucs[1] * float(dims[1]) - 0.5f;
    return cLerpCullIrcs(dims,Vec2F{xf,yf});
}

RgbaF
sampleAlpha(ImgRgba8 const & img,Vec2F coordIucs)
{
    RgbaF                 ret(0);
    VArray<CoordWgt,4>   ics = cLerpCullIucs(img.dims(),coordIucs);
    for (uint ii=0; ii<ics.size(); ++ii)
        ret += RgbaF(img[ics[ii].coordIrcs]) * ics[ii].wgt;
    return ret;
}

Mat<CoordWgt,2,2>
blerpCoordsClipIrcs(Vec2UI dims,Vec2D ircs)
{
    Mat<CoordWgt,2,2>   ret;
    Vec2I               loXY = Vec2I(mapFloor(ircs)),
                        hiXY = loXY + Vec2I(1);
    Vec2D               wgtHiXY = ircs - Vec2D(loXY),
                        wgtLoXY = Vec2D(1) - wgtHiXY;
    ret[0].wgt = wgtLoXY[0] * wgtLoXY[1];
    ret[1].wgt = wgtHiXY[0] * wgtLoXY[1];
    ret[2].wgt = wgtLoXY[0] * wgtHiXY[1];
    ret[3].wgt = wgtHiXY[0] * wgtHiXY[1];
    Vec2I const         zero(0);
    Vec2I               max = Vec2I(dims) - Vec2I(1);
    loXY = mapClamp(loXY,zero,max);
    hiXY = mapClamp(hiXY,zero,max);
    ret[0].coordIrcs = Vec2UI(loXY);
    ret[1].coordIrcs = Vec2UI(hiXY[0],loXY[1]);
    ret[2].coordIrcs = Vec2UI(loXY[0],hiXY[1]);
    ret[3].coordIrcs = Vec2UI(hiXY);
    return ret;
}
Mat<CoordWgt,2,2>
blerpCoordsClipIucs(Vec2UI dims,Vec2F iucs)
{
    Vec2D               ircs = mapMul(Vec2D{iucs},Vec2D{dims}) - Vec2D{0.5f};
    return blerpCoordsClipIrcs(dims,ircs);
}

AffineEw2D
imgScaleToCover(Vec2UI inDims,Vec2UI outDims)
{
    FGASSERT(inDims.cmpntsProduct() > 0);
    Vec2D           inDimsD {inDims},
                    outDimsD {outDims},
                    relDims = mapDiv(outDimsD,inDimsD);
    float           scale = cMaxElem(relDims);  // Larger scale is the minimal cover
    Vec2D           outMargin = (inDimsD * scale - outDimsD) * 0.5;
    ScaleTrans2D    outToInIrcs =
        ScaleTrans2D{Vec2D{0.5}} * ScaleTrans2D{1.0/scale} * ScaleTrans2D{Vec2D{-0.5}-outMargin};
    return outToInIrcs.asAffineEw();
}

AffineEw2D
imgScaleToFit(Vec2UI inDims,Vec2UI outDims)
{
    Mat22D          inBoundsIrcs{
                        -0.5, scast<double>(inDims[0])-0.5,
                        -0.5, scast<double>(inDims[1])-0.5
                    },
                    outBoundsIrcs{
                        -0.5, scast<double>(outDims[0])-0.5,
                        -0.5, scast<double>(outDims[1])-0.5
                    };
    return AffineEw2D{outBoundsIrcs,inBoundsIrcs};
}

Img3F
resampleSimple(Img3F const & in,Vec2UI dims,AffineEw2D const & outToInIrcs)
{
    Img3F               ret {dims};
    for (Iter2UI it {dims}; it.valid(); it.next()) {
        Vec2D               inIrcs = outToInIrcs * Vec2D(it());
        auto                lerp = blerpCoordsClipIrcs(in.dims(),inIrcs);
        Arr3F               p {0,0,0};
        for (uint ii=0; ii<4; ++ii) {
            CoordWgt const &    cw = lerp[ii];
            p += in[cw.coordIrcs] * cw.wgt;
        }
        ret[it()] = p;
    }
    return ret;
}

Img3F
resampleAdaptive(Img3F in,Vec2D posIpcs,float inSize,uint outSize)
{
    FGASSERT(!in.empty());
    FGASSERT(inSize > 0);
    FGASSERT(outSize > 0);
    // Reduce the input image to avoid undersampling. 1 1/3 undersampling gives minimum contribution of 2/3
    // pixel value (max always 1). 1 1/2 undersampling gives minimum representation of 1/2:
    while (inSize / outSize > 1.3333f) {
        in = shrink2(in);
        posIpcs *= 0.5f;
        inSize *= 0.5f;
    }
    for (uint dd=0; dd<2; ++dd) {                   // Ensure overlap between 'in' and selected region:
        FGASSERT(posIpcs[dd] < in.dims()[dd]);
        FGASSERT(posIpcs[dd]+inSize > 0.0f);
    }
    return resampleSimple(in,Vec2UI{outSize},AffineEw2D{Vec2D{inSize/outSize},posIpcs});
}

bool
fgImgApproxEqual(ImgRgba8 const & img0,ImgRgba8 const & img1,uint maxDelta)
{
    if (img0.dims() != img1.dims())
        return false;
    int             lim = int(maxDelta * maxDelta);
    for (Iter2UI it(img0.dims()); it.valid(); it.next()) {
        Arr4I       delta = mapCast<int>(img0[it()].m_c) - mapCast<int>(img1[it()].m_c);
        if ((sqr(delta[0]) > lim) ||
            (sqr(delta[1]) > lim) ||
            (sqr(delta[2]) > lim) ||
            (sqr(delta[3]) > lim))
            return false;
    }
    return true;
}

void
shrink2_(ImgRgba8 const & src,ImgRgba8 & dst)
{
    FGASSERT(!src.empty());
    dst.resize(src.dims()/2);
    Rgba8 const    *srcPtr1 = src.dataPtr(),
                    *srcPtr2 = srcPtr1 + src.width();
    Rgba8          *dstPtr =  dst.dataPtr();
    for (uint yd=0; yd<dst.height(); yd++) {
        for (uint xd=0; xd<dst.width(); xd++) {
            uint        xs1 = xd * 2,
                        xs2 = xs1 + 1;
            RgbaUS      acc = RgbaUS(srcPtr1[xs1]) +
                              RgbaUS(srcPtr1[xs2]) +
                              RgbaUS(srcPtr2[xs1]) +
                              RgbaUS(srcPtr2[xs2]) +
                              RgbaUS(1,1,1,1);        // Account for rounding bias
            dstPtr[xd] = Rgba8(acc/4);
        }
        dstPtr += dst.width();
        srcPtr1 += 2 * src.width();
        srcPtr2 += 2 * src.width();
    }
}

ImgRgba8
expand2(ImgRgba8 const & src)
{
    ImgRgba8     dst(src.dims()*2);
    Vec2F        off(-0.5f,-0.5f);
    Vec2I        srcMax(src.dims()-Vec2UI(1));
    for (uint yy=0; yy<dst.height(); ++yy) {
        // Keep interim values positive to avoid rounding bias around origin:
        int     syl = int(yy+1) / 2 - 1,
                syh = syl + 1;
        syl = (syl < 0 ? 0 : syl);
        syh = (syh > srcMax[1] ? srcMax[1] : syh);
        uint    wyl = 1 + 2 * (yy & 1),
                wyh = 4 - wyl;
        for (uint xx=0; xx<dst.width(); ++xx) {
            int     sxl = int(xx+1) / 2 - 1,
                    sxh = sxl + 1;
            sxl = (sxl < 0 ? 0 : sxl);
            sxh = (sxh > srcMax[0] ? srcMax[0] : sxh);
            uint    wxl = 1 + 2 * (xx & 1),
                    wxh = 4 - wxl;
            RgbaUS    acc =
                RgbaUS(src.xy(sxl,syl)) * wxl * wyl +
                RgbaUS(src.xy(sxh,syl)) * wxh * wyl +
                RgbaUS(src.xy(sxl,syh)) * wxl * wyh +
                RgbaUS(src.xy(sxh,syh)) * wxh * wyh +
                RgbaUS(7,7,7,7);      // Account for rounding bias
            dst.xy(xx,yy) = Rgba8(acc / 16);
        }
    }
    return dst;
}

ImgUC
shrink2Fixed(const ImgUC & img)
{
    ImgUC     dst(img.dims()/2);
    for (uint yy=0; yy<dst.height(); ++yy)
        for (uint xx=0; xx<dst.width(); ++xx)
            dst.xy(xx,yy) = uchar((
                uint(img.xy(2*xx,2*yy)) +
                uint(img.xy(2*xx+1,2*yy)) +
                uint(img.xy(2*xx+1,2*yy+1)) +
                uint(img.xy(2*xx,2*yy+1))+1) / 4);
    return dst;
}

// To ensure that floating point value 1 can round-trip to 255 and back we need to sacrifice that value
// from the 8-bit range. Any other method will not round-trip between 1 and 255 which is often important
// for background processing etc:

Svec<Arr<float,3>>
toUnit3F(Svec<Rgba8> const & data)
{
    float constexpr             f = 255.0f;                         // Ensure that highest value maps back to 1
    Svec<Arr<float,3>>          ret; ret.reserve(data.size());
    for (Rgba8 d : data)
        ret.push_back(Arr<float,3>{d[0]/f,d[1]/f,d[2]/f});          // emplace doesn't work for some reason
    return ret;
}

Img3F
toUnit3F(ImgRgba8 const & in)
{
    return Img3F {in.dims(),toUnit3F(in.m_data)};
}

Svec<Arr<float,4>>
toUnit4F(Svec<Rgba8> const & data)
{
    float constexpr             f = 255.0f;                         // Ensure that highest value maps back to 1
    Svec<Arr<float,4>>          ret; ret.reserve(data.size());
    for (Rgba8 d : data)
        ret.push_back(Arr<float,4>{d[0]/f,d[1]/f,d[2]/f,d[3]/f});   // emplace doesn't work for some reason
    return ret;
}

Img4F
toUnit4F(ImgRgba8 const & in)
{return Img4F {in.dims(),toUnit4F(in.m_data)}; }

Svec<Rgba8>
toRgba8(Svec<Arr<float,3>> const & data,float maxVal)
{
    float                       f = 255.0f / maxVal;                // Ensure we can round-trip 1.0f <-> 255U
    Svec<Rgba8>                 ret; ret.reserve(data.size());
    for (Arr<float,3> const & d : data)
        ret.emplace_back(uchar(d[0]*f),uchar(d[1]*f),uchar(d[2]*f),255);
    return ret;
}

ImgRgba8
toRgba8(Img3F const & in,float maxVal)
{return ImgRgba8 {in.dims(),toRgba8(in.m_data,maxVal)}; }

Svec<Rgba8>
toRgba8(Svec<RgbaF> const & data)
{
    float constexpr             f = 255.0f;                         // Ensure we can round-trip 1.0f <-> 255U
    Svec<Rgba8>                ret; ret.reserve(data.size());
    for (RgbaF const & p : data)
        ret.emplace_back(uchar(p[0]*f),uchar(p[1]*f),uchar(p[2]*f),uchar(p[3]*f));
    return ret;
}

ImgRgba8
toRgba8(ImgC4F const & img)
{return ImgRgba8 {img.dims(),toRgba8(img.m_data)}; }

ImgUC
toUC(ImgRgba8 const & in)
{
    struct C {
        uchar operator()(Rgba8 rgba) const {return rgba.rec709(); }
    };
    return mapCallT<uchar,Rgba8,C>(in,C{});
}

ImgF
toFloat(ImgRgba8 const & in)
{
    struct C {
        float operator()(Rgba8 rgba) const {return RgbaF(rgba).rec709(); }
    };
    return mapCallT<float,Rgba8,C>(in,C{});
}

ImgRgba8
toRgba8(ImgUC const & in)
{
    struct C {
        Rgba8 operator()(uchar v) const {return {v,v,v,255}; }
    };
    return mapCallT<Rgba8,uchar,C>(in,C{});
}

void
imgResize(
	ImgRgba8 const & src,
	ImgRgba8       & dst)
{
    FGASSERT(!src.empty());
    FGASSERT(!dst.empty());
    typedef ImgRgba8          ImageType;
    typedef ImageType::PixelType PixelType;
	uint        swid = src.width(),
                shgt = src.height(),
                wid = dst.width(),
                hgt = dst.height();
    if ((wid == swid) && (hgt == shgt)) {
        dst = src;
		return;
	}
	float   sx = (float)wid / (float)swid,
            sy   = (float)hgt / (float)shgt;
	const PixelType *srcPtr = src.dataPtr();
	PixelType       *dstPtr	= dst.dataPtr();
	uint        srcStep = src.width(),
                dstStep  = dst.width();
	float       ex    = 1.0f / sx,
                ey      = 1.0f / sy,
                invArea	= 1.0f / (ex * ey);
	for	(uint yDstRcs=0; yDstRcs<hgt; yDstRcs++) {
		// Calculate the y pixel bounds in the source image IRCS
		// Note that we need to check the upper bound since
		// this algorithm will want to add an out of bounds pixel
		// with weight 0. This still works with the weighting
		// algorithm in the inner loop since the float value
		// will only ever just hit the out of bounds integer
		// value and not exceed it.
		float	yblo = (float)yDstRcs * ey,
				ybhi = yblo + ey;
		uint	yblob = (uint)yblo,
				ybhib = (uint)ybhi;
		if (ybhib >= shgt)
            --ybhib;
		for (uint xDstRcs=0; xDstRcs<wid; xDstRcs++) {
            // Calculate the x pixel bounds in the source image IRCS
			float	xblo = (float)xDstRcs * ex,
					xbhi = xblo + ex;
			uint	xblob = (uint)xblo,
					xbhib = (uint)xbhi;
			if (xbhib >= swid)
                --xbhib;
            // Now sum up the contributions of the source pixels
            // that fall within this back-projected window.
			Rgba<float>     acc(0),
                            fpix;
			for (uint yy=yblob; yy<=ybhib; yy++) {
				float	yfac = 1.0f;
				if (yy == yblob)
					yfac -= yblo - (float)yblob;
				if (yy == ybhib)
					yfac -= (float)(ybhib+1) - ybhi;
				for (uint xx=xblob; xx<=xbhib; xx++) {
					float	xfac = 1.0f;
					if (xx == xblob)
						xfac -= xblo - (float)xblob;
					if (xx == xbhib)
						xfac -= (float)(xbhib+1) - xbhi;
					deepCast_(srcPtr[yy*srcStep + xx],fpix);
					acc += fpix * xfac * yfac;
				}
			}
            // Divide by the area of the back-project and convert
            // back to fixed point.
            round_(acc * invArea,dstPtr[xDstRcs]);
		}
		dstPtr += dstStep;
	}
}

ImgRgba8
fgImgApplyTransparencyPow2(
    ImgRgba8 const & colour,
    ImgRgba8 const & transparency)
{
    FGASSERT(!colour.empty() && !transparency.empty());
    Vec2UI       dims = mapPow2Ceil(cMax(colour.dims(),transparency.dims()));
    ImgRgba8     ctmp(dims),
                    ttmp(dims);
    imgResize(colour,ctmp);
    imgResize(transparency,ttmp);

    for (Iter2UI it(dims); it.valid(); it.next())
        ctmp[it()].alpha() = ttmp[it()].rec709();
    return ctmp;
}

ImgRgba8
fgResample(
    const Img2F &       map,
    ImgRgba8 const &     src)
{
    ImgRgba8             ret {map.dims(),Rgba8{0}};
    Vec2F               noVal {-1,-1};
    for (Iter2UI it(ret.dims()); it.valid(); it.next()) {
        if (map[it()] != noVal) {
            Vec2F           pos = map[it()];
            pos[1] = 1.0f - pos[1];         // OTCS to UICS
            ret[it()] = Rgba8(sampleClipIucs(src,pos) + RgbaF{0.5});
        }
    }
    return ret;
}

bool
usesAlpha(ImgRgba8 const & img,uchar minVal)
{
    for (size_t ii=0; ii<img.numPixels(); ++ii) {
        if (img.m_data[ii].alpha() < minVal)
            return true;
    }
    return false;
}

void
paintCrosshair(ImgRgba8 & img,Vec2I ircs)
{
    Rgba8              centre {0,150,200,255},
                        thick {0,255,0,255};
    for (int yy=-4; yy<=4; ++yy) {
        for (int xx=-1; xx<=1; ++xx) {
            if (xx*yy == 0) {       // centre line
                img.paint(ircs+Vec2I{xx,yy},centre);
                img.paint(ircs+Vec2I{yy,xx},centre);
            }
            else {                  // thickness
                img.paint(ircs+Vec2I{xx,yy},thick);
                img.paint(ircs+Vec2I{yy,xx},thick);
            }
        }
    }
}

void
paintDot(ImgRgba8 & img,Vec2I ircs,Vec4UC c,uint radius)
{
    Rgba8    clr(c[0],c[1],c[2],c[3]);
    int         rad = int(radius);
    for (int yy=-rad; yy<=rad; ++yy)
        for (int xx=-rad; xx<=rad; ++xx)
            img.paint(ircs+Vec2I(xx,yy),clr);
}

void
paintDot(ImgRgba8 & img,Vec2F ipcs,Vec4UC c,uint radius)
{
    paintDot(img,Vec2I(mapFloor(ipcs)),c,radius);
}

ImgV3F
fgImgToF3(ImgRgba8 const & img)
{
    ImgV3F     ret(img.dims());
    for (size_t pp=0; pp<ret.numPixels(); ++pp) {
        Arr4UC       p = img[pp].m_c;
        ret[pp] = {static_cast<float>(p[0]),static_cast<float>(p[1]),static_cast<float>(p[2])};
    }
    return ret;
}

ImgV3Fs
fgSsi(const ImgV3F & img,uchar borderPolicy)
{
    ImgV3Fs        ret(log2Floor(cMinElem(img.dims()))+1);
    ret[0] = img;
    for (size_t ii=0; ii<ret.size()-1; ++ii) {
        smoothFloat_(ret[ii],ret[ii],borderPolicy);
        smoothFloat_(ret[ii],ret[ii],borderPolicy);
        shrink2_(ret[ii],ret[ii+1]);
    }
    return ret;
}

AffineEw2Fs
fgSsiItcsToIpcs(Vec2UI dims,Vec2F principalPointIpcs,Vec2F fovItcs)
{
    AffineEw2Fs       ret;
    do {
        Vec2F        pixPerTan = mapDiv(Vec2F(dims),fovItcs);
        ret.push_back(AffineEw2F(pixPerTan,principalPointIpcs));
        for (uint dd=0; dd<2; ++dd)
            if (dims[dd] & 1)
                fovItcs[dd] *= float(dims[dd]-1)/float(dims[dd]);
        dims /= 2U;
        principalPointIpcs *= 0.5f;
    } while (cMinElem(dims) > 1);
    return ret;
}

ImgRgba8
imgBlend(ImgRgba8 const & img0,ImgRgba8 const & img1,ImgUC const & transition)
{
    if (img0.empty())
        return img1;
    if (img1.empty() || transition.empty())
        return img0;
    size_t              np0 = img0.numPixels(),
                        np1 = img1.numPixels();
    // Choose the larger image for output dimensions:
    Vec2UI              dims = (np0 > np1) ? img0.dims() : img1.dims();
    ImgRgba8             ret(dims);
    AffineEw2F          ircsToIucs = cIrcsToIucsXf(dims);
    for (Iter2UI it(dims); it.valid(); it.next()) {
        Vec2F           iucs = ircsToIucs * Vec2F(it());
        RgbaF           a = sampleClipIucs(img0,iucs);
        RgbaF           b = sampleClipIucs(img1,iucs);
        float           w = sampleClipIucs(transition,iucs) / 255.0f;
        Arr4UC &        r = ret[it()].m_c;
        for (uint cc=0; cc<4; ++cc) {
            float           ac = a[cc],
                            bc = b[cc],
                            rc = (1-w) * ac + w * bc;
            r[cc] = rc;
        }
    }
    return ret;
}

ImgRgba8
composite(ImgRgba8 const & foreground,ImgRgba8 const & background)
{
    FGASSERT(foreground.dims() == background.dims());
    ImgRgba8                 ret(foreground.dims());
    for (size_t ii=0; ii<ret.numPixels(); ++ii)
        ret[ii] = compositeFragmentUnweighted(foreground[ii],background[ii]);
    return ret;
}

static
void
mod1(uint off,uint step,ImgRgba8 const & imgIn,ImgRgba8 const & imgMod,int mod,AffineEw2F ircsToIucs,ImgRgba8 & ret)
{
    for (uint yy=off; yy<ret.m_dims[1]; yy+=step) {
        for (uint xx=0; xx<ret.m_dims[0]; ++xx) {
            Rgba8          td = imgMod.xy(xx,yy),
                            out(0,0,0,255);
            RgbaF           pd = sampleClipIucs(imgIn,ircsToIucs*Vec2F(xx,yy));
            for (uint cc=0; cc<3; ++cc) {
                int             gamMod = (((int(td[cc]) - 64) * mod) / 256) + 64;
                gamMod = (gamMod < 0) ? 0 : gamMod;
                uint            vv = uint(pd[cc]) * uint(gamMod);
                vv = vv >> 6;
                vv = (vv > 255) ? 255 : vv;
                out[cc] = (unsigned char)vv;
            }
            ret.xy(xx,yy) = out;
        }
    }
}

static
void
mod2(uint off,uint step,ImgRgba8 const & imgIn,ImgRgba8 const & imgMod,int mod,AffineEw2F ircsToIucs,ImgRgba8 & ret)
{
    for (uint yy=off; yy<ret.m_dims[1]; yy+=step) {
        for (uint xx=0; xx<ret.m_dims[0]; ++xx) {
            RgbaF           td = sampleClipIucs(imgMod,ircsToIucs*Vec2F(xx,yy));
            Rgba8          out(0,0,0,255),
                            pd = imgIn.xy(xx,yy);
            for (uint cc=0; cc<3; ++cc) {
                int             gamMod = (((int(td[cc]) - 64) * mod) / 256) + 64;
                gamMod = (gamMod < 0) ? 0 : gamMod;
                uint            vv = uint(pd[cc]) * uint(gamMod);
                vv = vv >> 6;
                vv = (vv > 255) ? 255 : vv;
                out[cc] = (unsigned char)vv;
            }
            ret.xy(xx,yy) = out;
        }
    }
}

ImgRgba8
imgModulate(ImgRgba8 const & imgIn,ImgRgba8 const & imgMod,float modulationFactor)
{
    if (imgMod.empty())
        return imgIn;
    ImgRgba8          ret;
    if (cDeterminant(imgIn.dims(),imgMod.dims()) != 0) {
        string      info = toStr(imgIn.dims())+" !~ "+toStr(imgMod.dims());
        fgThrow("Aspect ratio mismatch between imgIn and modulation maps",info);
    }
    // Using threads directly instead of OpenMP here is slightly slower (82ms vs 79ms on i9-9900K)
    // for very large image which takes 783ms single-threaded:
    //PushTimer     tm("mod1");
    int             mod = int(modulationFactor*256.0f + 0.5f);
    if (imgMod.width() > imgIn.width()) {
        ret.resize(imgMod.dims());
        AffineEw2F      ircsToIucs = cIrcsToIucsXf(ret.dims());
        uint            nt = cMin(thread::hardware_concurrency(),ret.m_dims[1]);
        vector<thread>  threads;
        threads.reserve(nt);
        for (uint tt=0; tt<nt; ++tt)
            threads.emplace_back(mod1,tt,nt,cref(imgIn),cref(imgMod),mod,ircsToIucs,ref(ret));
        for (thread & thread : threads)
            thread.join();
    }
    else {
        ret.resize(imgIn.dims());
        AffineEw2F      ircsToIucs = cIrcsToIucsXf(ret.dims());
        uint            nt = cMin(thread::hardware_concurrency(),ret.m_dims[1]);
        vector<thread>  threads;
        threads.reserve(nt);
        for (uint tt=0; tt<nt; ++tt)
            threads.emplace_back(mod2,tt,nt,cref(imgIn),cref(imgMod),mod,ircsToIucs,ref(ret));
        for (thread & thread : threads)
            thread.join();
    }
    return ret;
}

void
smoothUint1D(
    uchar const         *srcPtr,
    uint                *dstPtr,
    uint                num,
    uchar               borderPolicy)               // See below
{
    dstPtr[0] = uint(srcPtr[0])*(2+borderPolicy) + uint(srcPtr[1]);
    for (uint ii=1; ii<num-1; ++ii)
        dstPtr[ii] = uint(srcPtr[ii-1]) + uint(srcPtr[ii])*2 + uint(srcPtr[ii+1]);
    dstPtr[num-1] = uint(srcPtr[num-2]) + uint(srcPtr[num-1])*(2+borderPolicy);
}
void
smoothUint_(ImgUC const & src,ImgUC & dst,uchar borderPolicy)
{
    FGASSERT((src.width() > 1) && (src.height() > 1));  // Algorithm not designed for dim < 2
    FGASSERT((borderPolicy == 0) || (borderPolicy == 1));
    dst.resize(src.width(),src.height());
    Img<uint>   acc(src.width(),3);
    uchar       *dstPtr = dst.rowPtr(0);
    uint        *accPtr0,
                *accPtr1 = acc.rowPtr(0),
                *accPtr2 = acc.rowPtr(1);
    smoothUint1D(src.rowPtr(0),accPtr1,src.width(),borderPolicy);
    smoothUint1D(src.rowPtr(1),accPtr2,src.width(),borderPolicy);
    for (uint xx=0; xx<src.width(); xx++) {
        // Add 7 to minimize rounding bias. Adding 8 would bias the other way so we have to
        // settle for a small amount of downward rounding bias unless we want to pseudo-randomize:
        uint        tmp = accPtr1[xx]*(2+borderPolicy) + accPtr2[xx] + 7U;
        dstPtr[xx] = uchar(tmp / 16);
    }
    for (uint yy=1; yy<src.height()-1; ++yy) {
        dstPtr = dst.rowPtr(yy),
        accPtr0 = acc.rowPtr((yy-1)%3),
        accPtr1 = acc.rowPtr(yy%3),
        accPtr2 = acc.rowPtr((yy+1)%3);
        smoothUint1D(src.rowPtr(yy+1),accPtr2,src.width(),borderPolicy);
        for (uint xx=0; xx<src.width(); ++xx)
            dstPtr[xx] = uchar((accPtr0[xx] + accPtr1[xx]*2 + accPtr2[xx] + 7U) / 16);
    }
    dstPtr = dst.rowPtr(dst.height()-1);
    for (uint xx=0; xx<dst.width(); ++xx)
        dstPtr[xx] = uchar((accPtr1[xx] + accPtr2[xx]*(2+borderPolicy) + 7U) / 16);
}
ImgUC
smoothUint(ImgUC const & src,uchar borderPolicy)
{
    ImgUC               ret;
    smoothUint_(src,ret,borderPolicy);
    return ret;
}

void
smoothUint1D(
    Rgba8 const        *srcPtr,
    RgbaUS              *dstPtr,
    uint                num,
    uchar               borderPolicy)               // See below
{
    dstPtr[0] = RgbaUS(srcPtr[0])*(2+borderPolicy) + RgbaUS(srcPtr[1]);
    for (uint ii=1; ii<num-1; ++ii)
        dstPtr[ii] = RgbaUS(srcPtr[ii-1]) + RgbaUS(srcPtr[ii])*2 + RgbaUS(srcPtr[ii+1]);
    dstPtr[num-1] = RgbaUS(srcPtr[num-2]) + RgbaUS(srcPtr[num-1])*(2+borderPolicy);
}
void
smoothUint_(
    ImgRgba8 const &     src,
    ImgRgba8 &           dst,
    uchar               borderPolicy)
{
    FGASSERT((src.width() > 1) && (src.height() > 1));  // Algorithm not designed for dim < 2
    FGASSERT((borderPolicy == 0) || (borderPolicy == 1));
    dst.resize(src.width(),src.height());
    Img<RgbaUS> acc(src.width(),3);                  // Accumulator image
    Rgba8           *dstPtr = dst.rowPtr(0);
    RgbaUS         *accPtr0,
                *accPtr1 = acc.rowPtr(0),
                *accPtr2 = acc.rowPtr(1);
    smoothUint1D(src.rowPtr(0),accPtr1,src.width(),borderPolicy);
    smoothUint1D(src.rowPtr(1),accPtr2,src.width(),borderPolicy);
    for (uint xx=0; xx<src.width(); xx++) {
        // Add 7 to minimize rounding bias. Adding 8 would bias the other way so we have to
        // settle for a small amount of downward rounding bias unless we want to pseudo-randomize:
        RgbaUS        tmp = accPtr1[xx]*(2+borderPolicy) + accPtr2[xx] + RgbaUS(7);
        dstPtr[xx] = Rgba8(tmp / 16);
    }
    for (uint yy=1; yy<src.height()-1; ++yy) {
        dstPtr = dst.rowPtr(yy),
        accPtr0 = acc.rowPtr((yy-1)%3),
        accPtr1 = acc.rowPtr(yy%3),
        accPtr2 = acc.rowPtr((yy+1)%3);
        smoothUint1D(src.rowPtr(yy+1),accPtr2,src.width(),borderPolicy);
        for (uint xx=0; xx<src.width(); ++xx)
            dstPtr[xx] = Rgba8((accPtr0[xx] + accPtr1[xx]*2 + accPtr2[xx] + RgbaUS(7)) / 16);
    }
    dstPtr = dst.rowPtr(dst.height()-1);
    for (uint xx=0; xx<dst.width(); ++xx)
        dstPtr[xx] = Rgba8((accPtr1[xx] + accPtr2[xx]*(2+borderPolicy) + RgbaUS(7)) / 16);
}
ImgRgba8
smoothUint(ImgRgba8 const & src,uchar borderPolicy)
{
    ImgRgba8             ret;
    smoothUint_(src,ret,borderPolicy);
    return ret;
}

}

// */
