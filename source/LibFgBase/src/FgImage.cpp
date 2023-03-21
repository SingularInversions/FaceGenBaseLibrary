//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgImage.hpp"
#include "FgMath.hpp"
#include "FgTime.hpp"
#include "FgScaleTrans.hpp"
#include "FgApproxEqual.hpp"

using namespace std;

namespace Fg {

std::ostream &      operator<<(std::ostream & os,ImgRgba8 const & img)
{
    if (img.empty())
        os << "Empty image";
    else
        os << "Dimensions: " << img.dims() << fgnl
            << "Channel bounds: " << cBounds(img.m_data);
    return os;
}

std::ostream &      operator<<(std::ostream & os,ImgC4F const & img)
{
    if (img.empty())
        return os << "Empty image";
    Mat<float,4,1>    init {img.xy(0,0).m_c};
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

AffineEw2D          cIpcsToIucs(Vec2UI dims)
{
    return {
        {0,0},                      // domain lo
        {double(dims[0]),double(dims[1])},  // domain hi
        {0,0},                      // map lo
        {1,1}                       // map hi
    };
}
AffineEw2D          cIrcsToIucs(Vec2UI dims)
{
    return {
        {-0.5,-0.5},                // domain lo
        {dims[0]-0.5,dims[1]-0.5},  // domain hi
        {0,0},                      // map lo
        {1,1}                       // map hi
    };
}
AffineEw2D          cIrcsToOtcs(Vec2UI dims)
{
    return {
        {-0.5,-0.5},                // domain lo
        {dims[0]-0.5,dims[1]-0.5},  // domain hi
        {0,1},                      // map lo
        {1,0}                       // map hi
    };
}
AffineEw2D          cIucsToIrcsXf(Vec2UI dims)
{
    return {
        {0,0},                      // domain lo
        {1,1},                      // domain hi
        {-0.5,-0.5},                // map lo
        {dims[0]-0.5,dims[1]-0.5}   // map hi
    };
}
AffineEw2D          cIucsToIpcsXf(Vec2UI dims)
{
    return {
        {0,0},                      // domain lo
        {1,1},                      // domain hi
        {0,0},                      // map lo
        {double(dims[0]),double(dims[1])}   // map hi
    };
}
AffineEw2D          cOicsToIucsXf()
{
    return {
        {-1,-1},                    // domain lo
        {1,1},                      // domain hi
        {0,1},                      // map lo
        {1,0}                       // map hi
    };
}

Lerp::Lerp(float rcs,size_t dim)
{
    float           lof = floor(rcs);
    lo = int(lof);
    if (lo < -1)                        // 0 taps (below data)
        return;
    // very important to cast dim to int as otherwise int is cast to size_t in comparisons:
    int             dimi = scast<int>(dim);
    if (lo >= dimi)                     // 0 taps (above data)
        return;
    float           whi = rcs - lof;
    if (lo == -1)                       // 1 tap (below data)
        wgts[1] = whi;
    else if (lo+1 == dimi)              // 1 tap (above data)
        wgts[0] = 1.0f - whi;
    else                                // 2 taps
    {
        wgts[0] = 1.0f - whi;
        wgts[1] = whi;
    }
}

LerpClamp::LerpClamp(float rcs,size_t dim)
{
    FGASSERT(dim > 0);          // approach of clamping to valid pixels doesn't work for empty image
    float           lof = floor(rcs),
                    whi = rcs - lof;
    wgts[0] = 1.0f - whi;
    wgts[1] = whi;
    int             loi (lof);
    if (loi < 0)                    // low clamp
        inds = {0,0};
    else {
        size_t          loz (loi),      // we know it's >= 0
                        hiz = loz+1,
                        maxz = dim-1;
        if (hiz > maxz)                 // high clamp
            inds = {maxz,maxz};
        else
            inds = {loz,hiz};
    }
}

Img<FatBool>        mapAnd(const Img<FatBool> & lhs,const Img<FatBool> & rhs)
{
    FGASSERT(lhs.dims() == rhs.dims());
    Img<FatBool>     ret(lhs.dims());
    for (Iter2UI it(ret.dims()); it.valid(); it.next())
        ret[it()] = lhs[it()] && rhs[it()];
    return ret;
}

Mat<CoordWgt,2,2>   getBlerpClipIrcs(Vec2UI dims,Vec2D ircs)
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
Mat<CoordWgt,2,2>   getBlerpClipIucs(Vec2UI dims,Vec2F iucs)
{
    Vec2D               ircs = mapMul(Vec2D{iucs},Vec2D{dims}) - Vec2D{0.5f};
    return getBlerpClipIrcs(dims,ircs);
}

AffineEw2D          imgScaleToCover(Vec2UI inDims,Vec2UI outDims)
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

AffineEw2D          imgScaleToFit(Vec2UI inDims,Vec2UI outDims)
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

Img3F               resampleSimple(Img3F const & in,Vec2UI dims,AffineEw2D const & outToInIrcs)
{
    Img3F               ret {dims};
    for (Iter2UI it {dims}; it.valid(); it.next()) {
        Vec2D               inIrcs = outToInIrcs * Vec2D(it());
        auto                lerp = getBlerpClipIrcs(in.dims(),inIrcs);
        Arr3F               p {0,0,0};
        for (uint ii=0; ii<4; ++ii) {
            CoordWgt const &    cw = lerp[ii];
            p += in[cw.coordIrcs] * cw.wgt;
        }
        ret[it()] = p;
    }
    return ret;
}

Img3F               resampleAdaptive(Img3F in,Vec2D posIpcs,float inSize,uint outSize)
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

ImgC4F              mapGamma(ImgC4F const & img,float gamma)
{
    auto                fn = [gamma](RgbaF const & p)
    {
        RgbaF               ret {0,0,0,0};
        float               alpha = p[3];
        if (alpha > 0) {
            for (size_t ii=0; ii<3; ++ii)
                ret[ii] = pow(p[ii]/alpha,gamma) * alpha;
            ret[3] = alpha;
        }
        return ret;
    };
    return mapCall(img,fn);
}

void                shrink2_(ImgRgba8 const & src,ImgRgba8 & dst)
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
            Rgba16      acc = Rgba16(srcPtr1[xs1]) +
                              Rgba16(srcPtr1[xs2]) +
                              Rgba16(srcPtr2[xs1]) +
                              Rgba16(srcPtr2[xs2]) +
                              Rgba16(1,1,1,1);        // Account for rounding bias
            dstPtr[xd] = Rgba8(acc/4);
        }
        dstPtr += dst.width();
        srcPtr1 += 2 * src.width();
        srcPtr2 += 2 * src.width();
    }
}

ImgRgba8            expand2(ImgRgba8 const & src)
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
            Rgba16    acc =
                Rgba16(src.xy(sxl,syl)) * wxl * wyl +
                Rgba16(src.xy(sxh,syl)) * wxh * wyl +
                Rgba16(src.xy(sxl,syh)) * wxl * wyh +
                Rgba16(src.xy(sxh,syh)) * wxh * wyh +
                Rgba16(7,7,7,7);      // Account for rounding bias
            dst.xy(xx,yy) = Rgba8(acc / 16);
        }
    }
    return dst;
}

ImgUC               shrink2Fixed(const ImgUC & img)
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
Img3F               toUnit3F(ImgRgba8 const & in)
{
    auto                fn = [](Rgba8 p)
    {
        // if we define each uchar value v as representing the bin of real values in [v,v+1)
        // then we want to use a factor just under 256 then round down:
        float constexpr         f = 256.0f - 1/1024.0f;
        return Arr3F{p[0]/f,p[1]/f,p[2]/f};
    };
    return mapCallT<Arr3F>(in,fn);
}

Img4F               toUnit4F(ImgRgba8 const & in)
{
    auto                fn = [](Rgba8 p)
    {
        float constexpr         f = 256.0f - 1/1024.0f;
        return Arr4F{p[0]/f,p[1]/f,p[2]/f,p[3]/f};
    };
    return mapCallT<Arr4F>(in,fn);
}


ImgC4F              toUnitC4F(ImgRgba8 const & in)
{
    auto                fn = [](Rgba8 p)
    {
        float constexpr     f = 256.0f - 1/1024.0f;
        return RgbaF {p[0]/f,p[1]/f,p[2]/f,p[3]/f};
    };
    return mapCallT<RgbaF>(in,fn);
}

ImgRgba8            toRgba8(Img3F const & in,float maxVal)
{
    auto                fn = [maxVal](Arr3F p)
    {
        float constexpr         uf = 256.0f - 1/1024.0f;
        float                   f = uf / maxVal;
        return Rgba8{uchar(p[0]*f),uchar(p[1]*f),uchar(p[2]*f),255};
    };
    return mapCallT<Rgba8>(in,fn);
}

ImgRgba8            toRgba8(ImgC4F const & img)
{
    auto                fn = [](RgbaF p)
    {
        // if we define each uchar value v as representing the bin of real values in [v,v+1)
        // then we want to use a factor just under 256 then round down:
        float constexpr         f = 256.0f - 1/1024.0f;
        return Rgba8{uchar(p[0]*f),uchar(p[1]*f),uchar(p[2]*f),uchar(p[3]*f)};
    };
    return mapCallT<Rgba8>(img,fn);
}

ImgUC               toUC(ImgRgba8 const & in)
{
    return mapCallT<uchar>(in,[](Rgba8 p){return p.rec709();});
}

ImgF                toFloat(ImgRgba8 const & in)
{
    return mapCallT<float>(in,[](Rgba8 p){return p.rec709();});
}

ImgRgba8            toRgba8(ImgUC const & in)
{
    return mapCallT<Rgba8>(in,[](uchar p){return Rgba8{p,p,p,255};});
}

void                imgResize(ImgRgba8 const & src,ImgRgba8 & dst)
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
					mapCast_(srcPtr[yy*srcStep + xx],fpix);
					acc += fpix * xfac * yfac;
				}
			}
            // Divide by the area of the back-project and convert
            // back to fixed point.
            mapRound_(acc * invArea,dstPtr[xDstRcs]);
		}
		dstPtr += dstStep;
	}
}

ImgRgba8            applyTransparencyPow2(ImgRgba8 const & colour,ImgRgba8 const & transparency)
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

ImgRgba8            fgResample(Img2F const & map,ImgRgba8 const & src)
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

bool                usesAlpha(ImgRgba8 const & img,uchar minVal)
{
    for (size_t ii=0; ii<img.numPixels(); ++ii) {
        if (img.m_data[ii].alpha() < minVal)
            return true;
    }
    return false;
}

void                paintCrosshair(ImgRgba8 & img,Vec2I ircs)
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

void                paintDot(ImgRgba8 & img,Vec2I ircs,Rgba8 clr,uint radius)
{
    int                 rad = int(radius);
    for (int yy=-rad; yy<=rad; ++yy)
        for (int xx=-rad; xx<=rad; ++xx)
            img.paint(ircs+Vec2I(xx,yy),clr);
}

void                paintDot(ImgRgba8 & img,Vec2F ipcs,Rgba8 clr,uint radius)
{
    paintDot(img,Vec2I(mapFloor(ipcs)),clr,radius);
}

ImgRgba8            extrapolateForMipmap(ImgRgba8 const & img)
{
    // lift to accumulator type and scale up to avoid losing precision due to alpha-weighting:
    struct Lift {Rgba16 operator()(Rgba8 c) const {return Rgba16{c}*16; } };
    ImgRgba16s          mipmap = cMipmapA(mapCallT<Rgba16>(img,Lift{}));
    Rgba16 const        off {8};
    ImgRgba8            ret {img.dims()};
    for (Iter2UI it{img.dims()}; it.valid(); it.next()) {
        Vec2UI              crd = it();
        for (ImgRgba16 const & map : mipmap) {
            // if the input image is not a power of 2 then just halving and rounding down can still
            // fall outside lower mipmap bounds, so must clamp:
            crd = mapMin(crd,map.dims()-Vec2UI{1});
            Rgba16              pix = map[crd];
            if (pix.alpha() > 0) {
                Arr3UC              clr;
                for (size_t cc=0; cc<3; ++cc) {
                    // have to cast up to uint to renormalize alpha:
                    uint                c16 = (uint(pix[cc]) * 255*16) / pix.alpha();
                    clr[cc] = uchar((c16+8)/16);
                }
                ret[it()] = {clr[0],clr[1],clr[2],255};
                break;
            }
            crd = crd/2;
        }
    }
    return ret;
}

ImgRgba8            imgBlend(ImgRgba8 const & img0,ImgRgba8 const & img1,ImgUC const & transition)
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
    AffineEw2F          ircsToIucs = cIrcsToIucs(dims);
    for (Iter2UI it(dims); it.valid(); it.next()) {
        Vec2F           iucs = ircsToIucs * Vec2F(it());
        RgbaF           a = sampleClipIucs(img0,iucs);
        RgbaF           b = sampleClipIucs(img1,iucs);
        float           w = sampleClipIucs(transition,iucs) / 255.0f;
        Arr4UC &        r = ret[it()].m_c;
        for (uint cc=0; cc<3; ++cc) {
            float           ac = a[cc],
                            bc = b[cc],
                            rc = (1-w) * ac + w * bc;
            r[cc] = rc;
        }
        r[3] = 255;
    }
    return ret;
}

// Normal unweighted encoding:
// r_c = f_c * f_a + b_c * b_a * (1-f_a)
// r_a = f_a + b_a * (1-f_a)
Rgba8               compositeFragmentUnweighted(Rgba8 foreground,Rgba8 background)
{
    uint        f_a = foreground.alpha(),
                b_a = background.alpha(),
                omfa = 255 - f_a,
                tmp = (b_a * omfa + 127U) / 255U;
    Arr3UI      f_c = mapCast<uint>(cHead<3>(foreground.m_c)),
                b_c = mapCast<uint>(cHead<3>(background.m_c)),
                acc = f_c * f_a + b_c * tmp,
                r_c = (acc + cArr<uint,3>(127)) / 255U;
    return      Rgba8(r_c[0],r_c[1],r_c[2],f_a+tmp);
}

ImgRgba8            composite(ImgRgba8 const & foreground,ImgRgba8 const & background)
{
    FGASSERT(foreground.dims() == background.dims());
    ImgRgba8                 ret(foreground.dims());
    for (size_t ii=0; ii<ret.numPixels(); ++ii)
        ret.m_data[ii] = compositeFragmentUnweighted(foreground.m_data[ii],background.m_data[ii]);
    return ret;
}

ImgRgba8            imgModulate(ImgRgba8 const & imgIn,ImgRgba8 const & imgMod,float modFac,bool mt)
{
    if (imgMod.empty())
        return imgIn;
    ImgRgba8          ret;
    if (cDeterminant(imgIn.dims(),imgMod.dims()) != 0) {
        string      info = toStr(imgIn.dims())+" !~ "+toStr(imgMod.dims());
        fgThrow("Aspect ratio mismatch between imgIn and modulation maps",info);
    }
    int             mod = int(modFac*256.0f + 0.5f);
    if (imgMod.width() > imgIn.width()) {
        AffineEw2F      ircsToIucs = cIrcsToIucs(imgMod.dims());
        auto            fn = [&imgIn,&imgMod,mod,ircsToIucs](size_t xx,size_t yy)
        {
            Rgba8           td = imgMod.xy(xx,yy),
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
            return out;
        };
        ret = generateImg<Rgba8>(imgMod.dims(),fn,true);
    }
    else {
        AffineEw2F      ircsToIucs = cIrcsToIucs(imgIn.dims());
        auto            fn = [&imgIn,&imgMod,mod,ircsToIucs](size_t xx,size_t yy)
        {
            RgbaF           td = sampleClipIucs(imgMod,ircsToIucs*Vec2F(xx,yy));
            Rgba8           out(0,0,0,255),
                            pd = imgIn.xy(xx,yy);
            for (uint cc=0; cc<3; ++cc) {
                int             gamMod = (((int(td[cc]) - 64) * mod) / 256) + 64;
                gamMod = (gamMod < 0) ? 0 : gamMod;
                uint            vv = uint(pd[cc]) * uint(gamMod);
                vv = vv >> 6;
                vv = (vv > 255) ? 255 : vv;
                out[cc] = (unsigned char)vv;
            }
            return out;
        };
        ret = generateImg<Rgba8>(imgIn.dims(),fn,mt);
    }
    return ret;
}

void                smoothUint1D_(
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
void                smoothUint_(ImgUC const & src,ImgUC & dst,uchar borderPolicy)
{
    FGASSERT((src.width() > 1) && (src.height() > 1));  // Algorithm not designed for dim < 2
    FGASSERT((borderPolicy == 0) || (borderPolicy == 1));
    dst.resize(src.width(),src.height());
    Img<uint>   acc(src.width(),3,0);
    uchar       *dstPtr = dst.rowPtr(0);
    uint        *accPtr0,
                *accPtr1 = acc.rowPtr(0),
                *accPtr2 = acc.rowPtr(1);
    smoothUint1D_(src.rowPtr(0),accPtr1,src.width(),borderPolicy);
    smoothUint1D_(src.rowPtr(1),accPtr2,src.width(),borderPolicy);
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
        smoothUint1D_(src.rowPtr(yy+1),accPtr2,src.width(),borderPolicy);
        for (uint xx=0; xx<src.width(); ++xx)
            dstPtr[xx] = uchar((accPtr0[xx] + accPtr1[xx]*2 + accPtr2[xx] + 7U) / 16);
    }
    dstPtr = dst.rowPtr(dst.height()-1);
    for (uint xx=0; xx<dst.width(); ++xx)
        dstPtr[xx] = uchar((accPtr1[xx] + accPtr2[xx]*(2+borderPolicy) + 7U) / 16);
}
ImgUC               smoothUint(ImgUC const & src,uchar borderPolicy)
{
    ImgUC               ret;
    smoothUint_(src,ret,borderPolicy);
    return ret;
}

void                smoothUint1D_(Rgba8 const *srcPtr,Rgba16 *dstPtr,uint num,uchar borderPolicy)
{
    dstPtr[0] = Rgba16(srcPtr[0])*(2+borderPolicy) + Rgba16(srcPtr[1]);
    for (uint ii=1; ii<num-1; ++ii)
        dstPtr[ii] = Rgba16(srcPtr[ii-1]) + Rgba16(srcPtr[ii])*2 + Rgba16(srcPtr[ii+1]);
    dstPtr[num-1] = Rgba16(srcPtr[num-2]) + Rgba16(srcPtr[num-1])*(2+borderPolicy);
}
void                smoothUint_(ImgRgba8 const & src,ImgRgba8 & dst,uchar borderPolicy)
{
    FGASSERT((src.width() > 1) && (src.height() > 1));      // Algorithm not designed for dim < 2
    FGASSERT((borderPolicy == 0) || (borderPolicy == 1));
    dst.resize(src.width(),src.height());
    Img<Rgba16>         acc(src.width(),3,Rgba16(0));                 // Accumulator image
    Rgba8               *dstPtr = dst.rowPtr(0);
    Rgba16              *accPtr0,
                        *accPtr1 = acc.rowPtr(0),
                        *accPtr2 = acc.rowPtr(1);
    smoothUint1D_(src.rowPtr(0),accPtr1,src.width(),borderPolicy);
    smoothUint1D_(src.rowPtr(1),accPtr2,src.width(),borderPolicy);
    for (uint xx=0; xx<src.width(); xx++) {
        // Add 7 to minimize rounding bias. Adding 8 would bias the other way so we have to
        // settle for a small amount of downward rounding bias unless we want to pseudo-randomize:
        Rgba16              tmp = accPtr1[xx]*(2+borderPolicy) + accPtr2[xx] + Rgba16(7);
        dstPtr[xx] = Rgba8(tmp / 16);
    }
    for (uint yy=1; yy<src.height()-1; ++yy) {
        dstPtr = dst.rowPtr(yy),
        accPtr0 = acc.rowPtr((yy-1)%3),
        accPtr1 = acc.rowPtr(yy%3),
        accPtr2 = acc.rowPtr((yy+1)%3);
        smoothUint1D_(src.rowPtr(yy+1),accPtr2,src.width(),borderPolicy);
        for (uint xx=0; xx<src.width(); ++xx)
            dstPtr[xx] = Rgba8((accPtr0[xx] + accPtr1[xx]*2 + accPtr2[xx] + Rgba16(7)) / 16);
    }
    dstPtr = dst.rowPtr(dst.height()-1);
    for (uint xx=0; xx<dst.width(); ++xx)
        dstPtr[xx] = Rgba8((accPtr1[xx] + accPtr2[xx]*(2+borderPolicy) + Rgba16(7)) / 16);
}
ImgRgba8            smoothUint(ImgRgba8 const & src,uchar borderPolicy)
{
    ImgRgba8             ret;
    smoothUint_(src,ret,borderPolicy);
    return ret;
}

}

// */
