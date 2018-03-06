//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     May 14, 2005
//

#include "stdafx.h"

#include "FgImage.hpp"
#include "FgMath.hpp"

using namespace std;

std::ostream &
operator<<(std::ostream & os,const FgImgRgbaUb & img)
{
    if (img.empty())
        return os << "Empty image";
    FgMatrixC<uchar,4,1>    init = img[0].m_c;
    FgMatrixC<uchar,4,2>    bounds = fgConcatHoriz(init,init);
    for (FgIter2UI it(img.dims()); it.next(); it.valid()) {
        FgRgbaUB    pix = img[it()];
        for (uint cc=0; cc<4; ++cc) {
            fgSetIfLess(bounds.rc(cc,0),pix.m_c[cc]);
            fgSetIfGreater(bounds.rc(cc,1),pix.m_c[cc]);
        }
    }
    return
        os << "Dimensions: " << img.dims() <<
        fgnl << "Channel bounds: " << FgMatrixC<uint,4,2>(bounds);
}

FgImage<FgBool>
fgAnd(const FgImage<FgBool> & lhs,const FgImage<FgBool> & rhs)
{
    FGASSERT(lhs.dims() == rhs.dims());
    FgImage<FgBool>     ret(lhs.dims());
    for (FgIter2UI it(ret.dims()); it.valid(); it.next())
        ret[it()] = lhs[it()] && rhs[it()];
    return ret;
}

FgArray<FgCoordWgt,4>
fgBlerpCoordsCull(FgVect2UI dims,FgVect2F coordIucs)
{
    FgArray<FgCoordWgt,4>   ret;
    float       xf = coordIucs[0] * float(dims[0]) - 0.5f,     // IRCS
                yf = coordIucs[1] * float(dims[1]) - 0.5f;
    int         xil = int(std::floor(xf)),
                yil = int(std::floor(yf)),
                xih = xil + 1,
                yih = yil + 1,
                wid(dims[0]),
                hgt(dims[1]);
    float       wxh = xf - float(xil),
                wyh = yf - float(yil),
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

FgRgbaF
fgBlerpAlpha(const FgImgRgbaUb & img,FgVect2F coordIucs)
{
    FgRgbaF                 ret(0);
    FgArray<FgCoordWgt,4>   ics = fgBlerpCoordsCull(img.dims(),coordIucs);
    for (uint ii=0; ii<ics.size(); ++ii)
        ret += FgRgbaF(img[ics[ii].coordIrcs]) * ics[ii].wgt;
    return ret;
}

FgMatrixC<FgCoordWgt,4,1>
fgBlerpCoordsClip(FgVect2UI dims,FgVect2F iucs)
{
    FgMatrixC<FgCoordWgt,4,1>   ret;
    FgVect2F        ircs = fgMapMul(iucs,FgVect2F(dims)) - FgVect2F(0.5f);
    FgVect2I        loRcs = FgVect2I(fgFloor(ircs)),
                    hiRcs = loRcs + FgVect2I(1);
    FgVect2F        wgtHi = ircs - FgVect2F(loRcs),
                    wgtLo = FgVect2F(1) - wgtHi;
    ret[0].wgt = wgtLo[0] * wgtLo[1];
    ret[1].wgt = wgtHi[0] * wgtLo[1];
    ret[2].wgt = wgtLo[0] * wgtHi[1];
    ret[3].wgt = wgtHi[0] * wgtHi[1];
    FgVect2I        max = FgVect2I(dims) - FgVect2I(1);
    loRcs = fgClipElems(loRcs,0,max);
    hiRcs = fgClipElems(hiRcs,0,max);
    ret[0].coordIrcs = FgVect2UI(loRcs);
    ret[1].coordIrcs = FgVect2UI(hiRcs[0],loRcs[1]);
    ret[2].coordIrcs = FgVect2UI(loRcs[0],hiRcs[1]);
    ret[3].coordIrcs = FgVect2UI(hiRcs);
    return ret;
}

void
fgImgShrink2(
    const FgImgRgbaUb & src,
    FgImgRgbaUb &       dst)
{
    FGASSERT(!src.empty());
    dst.resize(src.dims()/2);
    const FgRgbaUB  * srcPtr1 = src.dataPtr(),
                    * srcPtr2 = srcPtr1 + src.width();
    FgRgbaUB        * dstPtr =  dst.dataPtr();
    for (uint yd=0; yd<dst.height(); yd++) {
        for (uint xd=0; xd<dst.width(); xd++) {
            uint        xs1 = xd * 2,
                        xs2 = xs1 + 1;
            FgRgbaUS    acc = FgRgbaUS(srcPtr1[xs1]) +
                              FgRgbaUS(srcPtr1[xs2]) +
                              FgRgbaUS(srcPtr2[xs1]) +
                              FgRgbaUS(srcPtr2[xs2]) +
                              FgRgbaUS(1,1,1,1);        // Account for rounding bias
            dstPtr[xd] = FgRgbaUB(acc/4);
        }
        dstPtr += dst.width();
        srcPtr1 += 2 * src.width();
        srcPtr2 += 2 * src.width();
    }
}

FgImgRgbaUb
fgExpand2(const FgImgRgbaUb & src)
{
    FgImgRgbaUb     dst(src.dims()*2);
    FgVect2F        off(-0.5f,-0.5f);
    FgVect2I        srcMax(src.dims()-FgVect2UI(1));
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
            FgRgbaUS    acc =
                FgRgbaUS(src.xy(sxl,syl)) * wxl * wyl +
                FgRgbaUS(src.xy(sxh,syl)) * wxh * wyl +
                FgRgbaUS(src.xy(sxl,syh)) * wxl * wyh +
                FgRgbaUS(src.xy(sxh,syh)) * wxh * wyh +
                FgRgbaUS(7,7,7,7);      // Account for rounding bias
            dst.xy(xx,yy) = FgRgbaUB(acc / 16);
        }
    }
    return dst;
}

FgImgRgbaUb
fgImgShrinkWid2(const FgImgRgbaUb & src)
{
    FgImgRgbaUb     dst;
    FGASSERT((src.width() > 0) && (src.height() > 0));
    dst.resize(src.width()/2, src.height());
    const FgRgbaUB     *srcPtr = src.dataPtr();
    FgRgbaUB           *dstPtr = dst.dataPtr();
    for (uint yd=0; yd<dst.height(); yd++) {
        for (uint xd=0; xd<dst.width(); xd++) {
            uint        xs1 = xd * 2,
                        xs2 = xs1 + 1;
            FgRgbaUS    acc = FgRgbaUS(srcPtr[xs1]) +
                              FgRgbaUS(srcPtr[xs2]);
            dstPtr[xd] = FgRgbaUB(acc/2);
        }
        dstPtr += dst.width();
        srcPtr += src.width();
    }
    return dst;
}

FgImgRgbaUb
fgImgShrinkHgt2(const FgImgRgbaUb & src)
{
    FgImgRgbaUb     dst;
    FGASSERT((src.width() > 0) && (src.height() > 0));
    dst.resize(src.width(), src.height()/2);
    const FgRgbaUB      *srcPtr1 = src.dataPtr(),
                        *srcPtr2 = srcPtr1 + src.width();
    FgRgbaUB            *dstPtr = dst.dataPtr();
    for (uint yd=0; yd<dst.height(); yd++) {
        for (uint xd=0; xd<dst.width(); xd++) {
            FgRgbaUS    acc = FgRgbaUS(srcPtr1[xd]) +
                              FgRgbaUS(srcPtr2[xd]);
            dstPtr[xd] = FgRgbaUB(acc/2);
        }
        dstPtr += dst.width();
        srcPtr1 += 2 * src.width();
        srcPtr2 += 2 * src.width();
    }
    return dst;
}

// RGBA -> mono uses rec709
void
fgImgConvert(const FgImgRgbaUb &src,FgImgUC &dst)
{
    dst.resize(src.width(),src.height());
    for (size_t ii=0; ii<dst.numPixels(); ++ii)
        dst[ii] = src[ii].rec709();
}

void    fgImgConvert(const FgImgUC &src,FgImgRgbaUb &dst)
{
    dst.resize(src.width(),src.height());
    for (size_t ii=0; ii<dst.numPixels(); ++ii) {
        uchar       val = src[ii];
        dst[ii] = FgRgbaUB(val,val,val,255);
    }
}

void
fgImgPntRescaleConvert(
    const FgImgD    &src,
    FgImgUC         &dst)
{
    FgVectD2        bounds = fgBounds(src.dataVec());
    double          scale = bounds[1] - bounds[0];
    scale = (scale > 0) ? scale : 1.0;
    FgImgD          adjusted;
    fgImgPntAffine(src,adjusted,-bounds[0],255.0/scale);
    fgCast_(adjusted,dst);
}

void
fgImgPntRescaleConvert(
    const FgImgD    &src,
    FgImgRgbaUb     &dst)
{
    FgImgUC         tmp;
    fgImgPntRescaleConvert(src,tmp);
    fgImgConvert(tmp,dst);
}

FgAffine2D
fgImgGetIrcsToBounds(
    uint                wid,
    uint                hgt,
    const FgMat22D   &bounds)
{
    FGASSERT(wid*hgt>0);
    double          scaleX = (bounds.rc(0,1)-bounds.rc(0,0)) / double(wid),
                    scaleY = (bounds.rc(1,1)-bounds.rc(1,0)) / double(hgt);
    FgMat22D     linear(scaleX,0.0,0.0,scaleY);
    FgVect2D        centre = FgVect2D(wid-1,hgt-1) * -0.5;
    return FgAffine2D(centre,linear);
}

void
fgImgResize(
	const FgImgRgbaUb & src,
	FgImgRgbaUb       & dst)
{
    FGASSERT(!src.empty());
    FGASSERT(!dst.empty());
    typedef FgImgRgbaUb          ImageType;
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
			FgRgba<float> acc, fpix;
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
					fgCast_(srcPtr[yy*srcStep + xx],fpix);
					acc += fpix * xfac * yfac;
				}
			}
            // Divide by the area of the back-project and convert
            // back to fixed point.
            fgRound(acc * invArea,dstPtr[xDstRcs]);
		}
		dstPtr += dstStep;
	}
}

FgImgRgbaUb
fgImgApplyTransparencyPow2(
    const FgImgRgbaUb & colour,
    const FgImgRgbaUb & transparency)
{
    FGASSERT(!colour.empty() && !transparency.empty());
    FgVect2UI       dims = fgPow2Ceil(fgMax(colour.dims(),transparency.dims()));
    FgImgRgbaUb     ctmp(dims),
                    ttmp(dims);
    fgImgResize(colour,ctmp);
    fgImgResize(transparency,ttmp);

    for (FgIter2UI it(dims); it.valid(); it.next())
        ctmp[it].alpha() = ttmp[it()].rec709();
    return ctmp;
}

FgImgRgbaUb
fgResample(
    const FgImage<FgVect2F> &   map,
    const FgImgRgbaUb &         src)
{
    FgImgRgbaUb         ret(map.dims());
    FgVect2F            noVal(-1,-1);
    for (FgIter2UI it(ret.dims()); it.valid(); it.next()) {
        if (map[it()] != noVal) {
            FgVect2F    pos = map[it()];
            pos[1] = 1.0f - pos[1];         // OTCS to UICS
            ret[it()] = FgRgbaUB(fgBlerpClipIucs(src,pos) + FgRgbaD(0.5));
        }
    }
    return ret;
}

bool
fgUsesAlpha(const FgImgRgbaUb & img,uchar minVal)
{
    FGASSERT(!img.empty());
    for (size_t ii=0; ii<img.numPixels(); ++ii) {
        if (img.m_data[ii].alpha() < minVal)
            return true;
    }
    return false;
}

void
fgPaintCrossHair(FgImgRgbaUb & img,FgVect2I ircs,FgVect4UC c,uint thickness)
{
    FgRgbaUB    clr(c[0],c[1],c[2],c[3]);
    int         rad = int(thickness/2),
                hlen = int(thickness)*4;
    // Horizontal stroke:
    for (int yy=-rad; yy<=rad; ++yy)
        for (int xx=-hlen; xx<=hlen; ++xx)
            img.paint(ircs+FgVect2I(xx,yy),clr);
    // Vertical stroke:
    for (int yy=-hlen; yy<=hlen; ++yy)
        for (int xx=-rad; xx<=rad; ++xx)
            img.paint(ircs+FgVect2I(xx,yy),clr);
}

void
fgPaintDot(FgImgRgbaUb & img,FgVect2I ircs,FgVect4UC c,uint radius)
{
    FgRgbaUB    clr(c[0],c[1],c[2],c[3]);
    int         rad = int(radius);
    for (int yy=-rad; yy<=rad; ++yy)
        for (int xx=-rad; xx<=rad; ++xx)
            img.paint(ircs+FgVect2I(xx,yy),clr);
}

void
fgPaintDot(FgImgRgbaUb & img,FgVect2F ipcs,FgVect4UC c,uint radius)
{
    fgPaintDot(img,FgVect2I(fgFloor(ipcs)),c,radius);
}

// Creates an OpenGL compatible mipmap from the given image; the original will be upsampled
// to a power of 2 if necessary and storage order reversed to bottom-to-top:
vector<FgImgRgbaUb>
fgOglMipMap(const FgImgRgbaUb & img)
{
    vector<FgImgRgbaUb>     ret(fgLog2Ceil(fgMaxElem(img.dims()))+1);
    fgResizePow2Ceil_(img,ret[0]);   // Just copies if already pow2 dims
    fgImgFlipVertical(ret[0]);
    for (size_t sl=1; sl<ret.size(); ++sl) {
        const FgImgRgbaUb & src = ret[sl-1];
        if (fgMinElem(src.dims()) > 1)
            ret[sl] = fgImgShrink2(src);
        else {
            if (src.width() > 1)
                ret[sl] = fgImgShrinkWid2(src);
            else
                ret[sl] = fgImgShrinkHgt2(src);
        }
    }
    return ret;
}

FgImg3F
fgImgToF3(const FgImgRgbaUb & img)
{
    FgImg3F     ret(img.dims());
    for (size_t pp=0; pp<ret.numPixels(); ++pp) {
        FgVect4UC       p = img[pp].m_c;
        ret[pp] = FgVect3F(p[0],p[1],p[2]);
    }
    return ret;
}

FgImg3Fs
fgSsi(const FgImg3F & img,uchar borderPolicy)
{
    FgImg3Fs        ret(fgLog2Floor(fgMinElem(img.dims()))+1);
    ret[0] = img;
    for (size_t ii=0; ii<ret.size()-1; ++ii) {
        fgSmoothFloat(ret[ii],ret[ii],borderPolicy);
        fgSmoothFloat(ret[ii],ret[ii],borderPolicy);
        fgShrink2Float(ret[ii],ret[ii+1]);
    }
    return ret;
}

FgAffineCw2Fs
fgSsiItcsToIpcs(FgVect2UI dims,FgVect2F principalPointIpcs,FgVect2F fovItcs)
{
    FgAffineCw2Fs       ret;
    do {
        FgVect2F        pixPerTan = fgMapDiv(FgVect2F(dims),fovItcs);
        ret.push_back(FgAffineCw2F(pixPerTan,principalPointIpcs));
        for (uint dd=0; dd<2; ++dd)
            if (dims[dd] & 1)
                fovItcs[dd] *= float(dims[dd]-1)/float(dims[dd]);
        dims /= 2U;
        principalPointIpcs *= 0.5f;
    } while (fgMinElem(dims) > 1);
    return ret;
}

// */
