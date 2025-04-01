//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgImage.hpp"
#include "FgMath.hpp"
#include "FgTime.hpp"
#include "FgTransform.hpp"

using namespace std;

namespace Fg {

std::ostream &      operator<<(std::ostream & os,ImgRgba8 const & img)
{
    if (img.empty())
        os << "Empty image";
    else
        os
        << fgnl << "Dimensions: " << img.dims()
        << fgnl << "Channel bounds: " << updateBounds(img.m_data);
    return os;
}

std::ostream &      operator<<(std::ostream & os,ImgC4F const & img)
{
    if (img.empty())
        return os << "Empty image";
    Mat<float,4,1>    init {img.xy(0,0).m_c};
    Mat<float,4,2>    bounds = catH(init,init);
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

ImgLerp::ImgLerp(float ircs,size_t numPix)
{
    FGASSERT(numPix > 0);
    float               loF = floor(ircs);
    lo = int(loF);
    if (lo < -1)                        // 0 taps (below data)
        return;
    // very important to cast numPix to int as otherwise int is cast to size_t in comparisons:
    int                 numPixI = scast<int>(numPix);
    if (lo >= numPixI)                  // 0 taps (above data)
        return;
    float               wgtHi = ircs - loF;
    if (lo == -1)                       // 1 tap (below data)
        wgts[1] = wgtHi;
    else if (lo+1 == numPixI)           // 1 tap (above data)
        wgts[0] = 1 - wgtHi;
    else                                // 2 taps
    {
        wgts[0] = 1 - wgtHi;
        wgts[1] = wgtHi;
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

Mat<CoordWgt,2,2>   cBlerpClampIrcs(Vec2UI dims,Vec2D ircs)
{
    Vec2I               loXY = Vec2I(mapFloor(ircs)),
                        hiXY = loXY + Vec2I(1);
    Vec2D               wgtHiXY = ircs - Vec2D(loXY),
                        wgtLoXY = Vec2D(1) - wgtHiXY;
    Vec2I const         zero(0);
    Vec2I               max = Vec2I(dims) - Vec2I(1);
    loXY = mapClamp(loXY,zero,max);
    hiXY = mapClamp(hiXY,zero,max);
    return {
        {Vec2UI(loXY),              wgtLoXY[0] * wgtLoXY[1]},
        {Vec2UI(hiXY[0],loXY[1]),   wgtHiXY[0] * wgtLoXY[1]},
        {Vec2UI(loXY[0],hiXY[1]),   wgtLoXY[0] * wgtHiXY[1]},
        {Vec2UI(hiXY),              wgtHiXY[0] * wgtHiXY[1]},
    };
}

Mat<CoordWgt,2,2>   cBlerpClampIucs(Vec2UI dims,Vec2D iucs)
{
    Vec2D               ircs = mapMul(iucs,Vec2D{dims}) - Vec2D{0.5};
    return cBlerpClampIrcs(dims,ircs);
}

AxAffine2D          imgScaleToCover(Vec2UI inDims,Vec2UI outDims)
{
    FGASSERT(inDims.elemsProduct() > 0);
    Vec2D           inDimsD {inDims},
                    outDimsD {outDims},
                    relDims = mapDiv(outDimsD,inDimsD);
    float           scale = cMaxElem(relDims);  // Larger scale is the minimal cover
    Vec2D           outMargin = (inDimsD * scale - outDimsD) * 0.5;
    ScaleTrans2D    outToInIrcs =
        ScaleTrans2D{Vec2D{0.5}} * ScaleTrans2D{1.0/scale} * ScaleTrans2D{Vec2D{-0.5}-outMargin};
    return AxAffine2D{outToInIrcs};
}

Img4F               resample(Img4F const & src,SquareF regionPacs,uint dstSize,bool mt)
{
    FGASSERT(regionPacs.size > 0);
    FGASSERT(dstSize > 0);
    ScaleTrans2F        dstToSrcPacs {regionPacs.size/dstSize,regionPacs.loPos},
                        dstToSrcIrcs = cPacsToIrcs<float,2>() * dstToSrcPacs * cIrcsToPacs<float,2>();
    auto                fn = [&src,dstToSrcIrcs](size_t xx,size_t yy)
    {
        Vec2F               srcIrcs = dstToSrcIrcs * Vec2F{scast<float>(xx),scast<float>(yy)};
        return sampleBlerpZero(src,srcIrcs);
    };
    return genImg<Arr4F>(Vec2UI{dstSize},fn,mt);
}

ImgRgba8            resampleAffine(ImgRgba8 const & in,Vec2UI dims,AxAffine2F const & outToInIrcs)
{
    ImgRgba8            ret {dims};
    for (Iter2UI it {dims}; it.valid(); it.next()) {
        ImgBlerp               blerp {outToInIrcs * Vec2F(it()),in.dims()};
        ret[it()] = blerp.sampleZeroFixed(in);
    }
    return ret;
}

ImgRgba8            filterResample(ImgRgba8 in,Vec2F loPacs,float inSize,uint outSize)
{
    FGASSERT(!in.empty());
    FGASSERT(inSize > 0);
    FGASSERT(outSize > 0);
    for (uint dd=0; dd<2; ++dd) {                   // Ensure overlap between 'in' and selected region:
        FGASSERT(loPacs[dd] < in.dims()[dd]);
        FGASSERT(loPacs[dd]+inSize > 0.0f);
    }
    // Reduce the input image to avoid aliasing. 1 1/3 undersampling gives minimum contribution of 2/3
    // pixel value (max always 1). 1 1/2 undersampling gives minimum representation of 1/2:
    while (inSize / outSize > 1.3333f) {
        in = shrink2(in);
        loPacs *= 0.5f;
        inSize *= 0.5f;
    }
    return resampleAffine(in,Vec2UI{outSize},AxAffine2F{Vec2F{inSize/outSize},loPacs});
}

// not optimized at all:
Img4F               blockResample(Img4F const & src,SquareF regionPacs,uint retSize,bool mt)
{
    FGASSERT(!src.empty());
    FGASSERT(regionPacs.size > 0);
    FGASSERT(retSize > 0);
    Vec2I               srcDims = mapCast<int>(src.dims());
    float               invScale = regionPacs.size / retSize;
    FGASSERT(invScale > 1);         // otherwise use blerp resample
    auto                boundsFn = [invScale](size_t dstIrcs,float srcLo)
    {
        float               lo = invScale * scast<float>(dstIrcs) + srcLo;
        return Vec2F{lo,lo+invScale};
    };
    // returns the coverage weight of the given bounds (with associated floors) over the given raster bin,
    // where a weight of 1 corresponds to full coverage:
    auto                weightFn = [](int rasterBin,Vec2F boundsPacs,Vec2I boundFloors)
    {
        if (rasterBin == boundFloors[0])
            return scast<float>(boundFloors[0]+1) - boundsPacs[0];  // since bounds separated by > 1
        else if (rasterBin == boundFloors[1])
            return boundsPacs[1] - scast<float>(boundFloors[1]);
        else
            return 1.0f;
    };
    auto                fn = [&,srcDims](size_t X,size_t Y)
    {
        Vec2F               boundsPacsX = boundsFn(X,regionPacs.loPos[0]),
                            boundsPacsY = boundsFn(Y,regionPacs.loPos[1]);
        Vec2I               boundFloorsX {mapFloor(boundsPacsX)},
                            boundFloorsY {mapFloor(boundsPacsY)};
        Arr4F               acc {0};
        float               accW {0};
        for (int yy=boundFloorsY[0]; yy<=boundFloorsY[1]; ++yy) {
            float               weightY = weightFn(yy,boundsPacsY,boundFloorsY);
            for (int xx=boundFloorsX[0]; xx<=boundFloorsX[1]; ++xx) {
                float               weightX = weightFn(xx,boundsPacsX,boundFloorsX),
                                    currWgt = weightX * weightY;
                accW += currWgt;
                if ( (xx>=0) && (xx<srcDims[0]) && (yy>=0) && (yy<srcDims[1]) )     // implicit zero value outside this region
                    acc += src.xy(xx,yy) * currWgt;
            }
        }
        // weighted mean channel values over block of source image, with pixels outside source implicitly zero:
        return acc / accW;      // accW is guaranteed to be > 1
    };
    return genImg<Arr4F>(Vec2UI{retSize},fn,mt);
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
    ImgRgba8            dst(src.dims()*2);
    Vec2I               srcMax(src.dims()-Vec2UI(1));
    for (uint yy=0; yy<dst.height(); ++yy) {
        // Keep interim values positive to avoid rounding bias around origin:
        int                 syl = int(yy+1) / 2 - 1,
                            syh = syl + 1;
        syl = (syl < 0 ? 0 : syl);
        syh = (syh > srcMax[1] ? srcMax[1] : syh);
        uint                wyl = 1 + 2 * (yy & 1),
                            wyh = 4 - wyl;
        for (uint xx=0; xx<dst.width(); ++xx) {
            int                 sxl = int(xx+1) / 2 - 1,
                                sxh = sxl + 1;
            sxl = (sxl < 0 ? 0 : sxl);
            sxh = (sxh > srcMax[0] ? srcMax[0] : sxh);
            uint                wxl = 1 + 2 * (xx & 1),
                                wxh = 4 - wxl;
            Rgba16              acc =
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
    return mapCall(in,fn);
}

Img4F               toUnit4F(ImgRgba8 const & in)
{
    auto                fn = [](Rgba8 p)
    {
        float constexpr         f = 255.0f;         // can't stretch out the values since alpha 255 must be 1
        return Arr4F{p[0]/f,p[1]/f,p[2]/f,p[3]/f};  // use division to ensure exact values of 1
    };
    return mapCall(in,fn);
}

Img4D               toUnit4D(ImgRgba8 const & in)
{
    auto                fn = [](Rgba8 p)
    {
        double constexpr        f = 255;            // can't stretch out the values since alpha 255 must be 1
        return Arr4D{p[0]/f,p[1]/f,p[2]/f,p[3]/f};  // use division to ensure exact values of 1
    };
    return mapCall(in,fn);
}

ImgC4F              toUnitC4F(ImgRgba8 const & in)
{
    auto                fn = [](Rgba8 p)
    {
        float constexpr     f = 256.0f - 1/1024.0f;
        return RgbaF {p[0]/f,p[1]/f,p[2]/f,p[3]/f};
    };
    return mapCall(in,fn);
}

ImgUC               toUC(ImgRgba8 const & in)
{
    return mapCall(in,[](Rgba8 p){return p.rec709();});
}

ImgF                toFloat(ImgRgba8 const & in)
{
    return mapCall(in,[](Rgba8 p){return RgbaF{p}.rec709();});
}

ImgRgba8            toRgba8(ImgUC const & in)
{
    return mapCall(in,[](uchar p){return Rgba8{p,p,p,255};});
}

Img4F               toApm(Img4F const & in)
{
    auto                fn = [](Arr4F p)
    {
        float               a = p[3];
        return Arr4F{p[0]*a,p[1]*a,p[2]*a,a};
    };
    return mapCall(in,fn);
}

void                imgResize_(ImgRgba8 const & src,ImgRgba8 & dst)
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
    Vec2UI              dims = mapPow2Ceil(mapMax(colour.dims(),transparency.dims()));
    ImgRgba8            ctmp(dims),
                        ttmp(dims);
    imgResize_(colour,ctmp);
    imgResize_(transparency,ttmp);
    for (Iter2UI it(dims); it.valid(); it.next())
        ctmp[it()].alpha() = ttmp[it()].rec709();
    return ctmp;
}

ImgRgba8            resampleMap(Img2F const & map,ImgRgba8 const & src)
{
    auto                fn = [&](Vec2F crd)
    {
        if ((crd[0] == -1) && (crd[1] == -1))
            return Rgba8{0};
        else {
            Vec2F               iucs {crd[0],1-crd[1]};     // OTCS to IUCS
            return Rgba8{sampleClampIucs(src,iucs) + RgbaF{0.5}};
        }
    };
    return mapCall(map,fn);
}

ImgUC               resampleMap(Img2F const & map,ImgUC const & src)
{
    auto                fn = [&](Vec2F crd)
    {
        if ((crd[0] == -1) && (crd[1] == -1))
            return uchar{0};
        else {
            Vec2F               iucs {crd[0],1-crd[1]};     // OTCS to IUCS
            return scast<uchar>(sampleClampIucs(src,iucs) + 0.5f);
        }
    };
    return mapCall(map,fn);
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

void                paintDot(ImgRgba8 & img,Vec2F pacs,Rgba8 clr,uint radius)
{
    paintDot(img,Vec2I(mapFloor(pacs)),clr,radius);
}

ImgRgba8            extrapolateForMipmap(ImgRgba8 const & img)
{
    // lift to accumulator type and scale up to avoid losing precision due to alpha-weighting:
    struct Lift {Rgba16 operator()(Rgba8 c) const {return Rgba16{c}*16; } };
    ImgRgba16s          mipmap = cMipmapA(mapCall(img,Lift{}));
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
    AxAffine2F          ircsToIucs = cIrcsToIucs<float>(dims);
    for (Iter2UI it(dims); it.valid(); it.next()) {
        Vec2F           iucs = ircsToIucs * Vec2F(it());
        RgbaF           a = sampleClampIucs(img0,iucs);
        RgbaF           b = sampleClampIucs(img1,iucs);
        float           w = sampleClampIucs(transition,iucs) / 255.0f;
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

ImgRgba8            composite(ImgRgba8 const & foreground,ImgRgba8 const & background)
{
    // Normal unweighted encoding:
    // rc = fc * fa + bc * ba * (1-fa)
    // ra = fa + ba * (1-fa)
    auto                fn = [](Rgba8 fg,Rgba8 bg)
    {
        uint            fa = fg.alpha(),
                        ba = bg.alpha(),
                        omfa = 255 - fa,
                        tmp = (ba * omfa + 127) / 255;
        Rgba8           ret;
        for (size_t ii=0; ii<3; ++ii) {
            uint            fc = fg.m_c[ii],
                            bc = bg.m_c[ii],
                            acc = fc * fa + bc * tmp,
                            rc = (acc + 127) / 255;
            ret[ii] = scast<uchar>(rc);
        }
        return Rgba8 {ret[0],ret[1],ret[2],scast<uchar>(fa+tmp)};
    };
    return mapCall(foreground,background,fn);
}

ImgRgba8            imgModulate(ImgRgba8 const & imgIn,ImgRgba8 const & imgMod,float modFac,bool mt)
{
    if (imgMod.empty())
        return imgIn;
    ImgRgba8          ret;
    if (!areColinear(imgIn.dims(),imgMod.dims())) {
        string      info = toStr(imgIn.dims())+" !~ "+toStr(imgMod.dims());
        fgThrow("Aspect ratio mismatch between imgIn and modulation maps",info);
    }
    int             mod = int(modFac*256.0f + 0.5f);
    if (imgMod.width() > imgIn.width()) {
        AxAffine2F      ircsToIucs = cIrcsToIucs<float>(imgMod.dims());
        auto            fn = [&imgIn,&imgMod,mod,ircsToIucs](size_t xx,size_t yy)
        {
            Rgba8           td = imgMod.xy(xx,yy),
                            out(0,0,0,255);
            RgbaF           pd = sampleClampIucs(imgIn,ircsToIucs*Vec2F(xx,yy));
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
        ret = genImg<Rgba8>(imgMod.dims(),fn,true);
    }
    else {
        AxAffine2F      ircsToIucs = cIrcsToIucs<float>(imgIn.dims());
        auto            fn = [&imgIn,&imgMod,mod,ircsToIucs](size_t xx,size_t yy)
        {
            RgbaF           td = sampleClampIucs(imgMod,ircsToIucs*Vec2F(xx,yy));
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
        ret = genImg<Rgba8>(imgIn.dims(),fn,mt);
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

ImgRgba8            visualize(MatD const & mat)
{
    Arr2D               bounds = cBounds(mat.m_data);
    auto                fn = [bounds](double v)
    {
        Rgba8               ret {0,0,0,255};
        if (v<0)
            ret[0] = scast<uchar>(255*pow(v/bounds[0],1/2.2));      // if v<0, bounds[0]<0 as well
        else
            ret[1] = scast<uchar>(255*pow(v/bounds[1],1/2.2));
        return ret;
    };
    Rgba8s              data8 = mapCall(mat.m_data,fn);
    Vec2UI              dims = mat.dims();
    ImgRgba8            img {dims[0],dims[1],data8};
    uint                mag = cMax(1024U/cMaxElem(dims),1U);
    fgout << "-ve shown in red, +ve in green, element bounds: " << bounds;
    return magnify(img,mag);
}


}

// */
