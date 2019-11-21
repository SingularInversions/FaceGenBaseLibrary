//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgImage.hpp"
#include "FgMath.hpp"

using namespace std;

namespace Fg {

std::ostream &
operator<<(std::ostream & os,const ImgC4UC & img)
{
    if (img.empty())
        return os << "Empty image";
    Mat<uchar,4,1>    init = img[0].m_c;
    Mat<uchar,4,2>    bounds = fgJoinHoriz(init,init);
    for (Iter2UI it(img.dims()); it.next(); it.valid()) {
        RgbaUC    pix = img[it()];
        for (uint cc=0; cc<4; ++cc) {
            setIfLess(bounds.rc(cc,0),pix.m_c[cc]);
            setIfGreater(bounds.rc(cc,1),pix.m_c[cc]);
        }
    }
    return
        os << "Dimensions: " << img.dims() <<
        fgnl << "Channel bounds: " << Mat<uint,4,2>(bounds);
}

Img<FgBool>
fgAnd(const Img<FgBool> & lhs,const Img<FgBool> & rhs)
{
    FGASSERT(lhs.dims() == rhs.dims());
    Img<FgBool>     ret(lhs.dims());
    for (Iter2UI it(ret.dims()); it.valid(); it.next())
        ret[it()] = lhs[it()] && rhs[it()];
    return ret;
}

VArray<CoordWgt,4>
blerpCoordsCull(Vec2UI dims,Vec2F coordIucs)
{
    VArray<CoordWgt,4>   ret;
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

RgbaF
sampleAlpha(const ImgC4UC & img,Vec2F coordIucs)
{
    RgbaF                 ret(0);
    VArray<CoordWgt,4>   ics = blerpCoordsCull(img.dims(),coordIucs);
    for (uint ii=0; ii<ics.size(); ++ii)
        ret += RgbaF(img[ics[ii].coordIrcs]) * ics[ii].wgt;
    return ret;
}

Mat<CoordWgt,2,2>
blerpCoordsClip(Vec2UI dims,Vec2F iucs)
{
    Mat<CoordWgt,2,2>   ret;
    Vec2F               ircs = fgMapMul(iucs,Vec2F(dims)) - Vec2F(0.5f);
    Vec2I               loXY = Vec2I(cFloor(ircs)),
                        hiXY = loXY + Vec2I(1);
    Vec2F               wgtHiXY = ircs - Vec2F(loXY),
                        wgtLoXY = Vec2F(1) - wgtHiXY;
    ret[0].wgt = wgtLoXY[0] * wgtLoXY[1];
    ret[1].wgt = wgtHiXY[0] * wgtLoXY[1];
    ret[2].wgt = wgtLoXY[0] * wgtHiXY[1];
    ret[3].wgt = wgtHiXY[0] * wgtHiXY[1];
    Vec2I const         zero(0);
    Vec2I               max = Vec2I(dims) - Vec2I(1);
    loXY = clampBounds(loXY,zero,max);
    hiXY = clampBounds(hiXY,zero,max);
    ret[0].coordIrcs = Vec2UI(loXY);
    ret[1].coordIrcs = Vec2UI(hiXY[0],loXY[1]);
    ret[2].coordIrcs = Vec2UI(loXY[0],hiXY[1]);
    ret[3].coordIrcs = Vec2UI(hiXY);
    return ret;
}

bool
fgImgApproxEqual(const ImgC4UC & img0,const ImgC4UC & img1,uint maxDelta)
{
    if (img0.dims() != img1.dims())
        return false;
    int             lim = int(maxDelta * maxDelta);
    for (Iter2UI it(img0.dims()); it.valid(); it.next()) {
        Vec4I    delta = Vec4I(img0[it()].m_c) - Vec4I(img1[it()].m_c);
        if ((sqr(delta[0]) > lim) ||
            (sqr(delta[1]) > lim) ||
            (sqr(delta[2]) > lim) ||
            (sqr(delta[3]) > lim))
            return false;
    }
    return true;
}

void
fgImgShrink2(
    const ImgC4UC & src,
    ImgC4UC &       dst)
{
    FGASSERT(!src.empty());
    dst.resize(src.dims()/2);
    const RgbaUC  * srcPtr1 = src.data(),
                    * srcPtr2 = srcPtr1 + src.width();
    RgbaUC        * dstPtr =  dst.data();
    for (uint yd=0; yd<dst.height(); yd++) {
        for (uint xd=0; xd<dst.width(); xd++) {
            uint        xs1 = xd * 2,
                        xs2 = xs1 + 1;
            RgbaUS    acc = RgbaUS(srcPtr1[xs1]) +
                              RgbaUS(srcPtr1[xs2]) +
                              RgbaUS(srcPtr2[xs1]) +
                              RgbaUS(srcPtr2[xs2]) +
                              RgbaUS(1,1,1,1);        // Account for rounding bias
            dstPtr[xd] = RgbaUC(acc/4);
        }
        dstPtr += dst.width();
        srcPtr1 += 2 * src.width();
        srcPtr2 += 2 * src.width();
    }
}

ImgC4UC
fgExpand2(const ImgC4UC & src)
{
    ImgC4UC     dst(src.dims()*2);
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
            dst.xy(xx,yy) = RgbaUC(acc / 16);
        }
    }
    return dst;
}

ImgUC
fgShrink2(const ImgUC & img)
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

ImgC4UC
fgImgShrinkWid2(const ImgC4UC & src)
{
    ImgC4UC     dst;
    FGASSERT((src.width() > 0) && (src.height() > 0));
    dst.resize(src.width()/2, src.height());
    const RgbaUC     *srcPtr = src.data();
    RgbaUC           *dstPtr = dst.data();
    for (uint yd=0; yd<dst.height(); yd++) {
        for (uint xd=0; xd<dst.width(); xd++) {
            uint        xs1 = xd * 2,
                        xs2 = xs1 + 1;
            RgbaUS    acc = RgbaUS(srcPtr[xs1]) +
                              RgbaUS(srcPtr[xs2]);
            dstPtr[xd] = RgbaUC(acc/2);
        }
        dstPtr += dst.width();
        srcPtr += src.width();
    }
    return dst;
}

ImgC4UC
fgImgShrinkHgt2(const ImgC4UC & src)
{
    ImgC4UC     dst;
    FGASSERT((src.width() > 0) && (src.height() > 0));
    dst.resize(src.width(), src.height()/2);
    const RgbaUC      *srcPtr1 = src.data(),
                        *srcPtr2 = srcPtr1 + src.width();
    RgbaUC            *dstPtr = dst.data();
    for (uint yd=0; yd<dst.height(); yd++) {
        for (uint xd=0; xd<dst.width(); xd++) {
            RgbaUS    acc = RgbaUS(srcPtr1[xd]) +
                              RgbaUS(srcPtr2[xd]);
            dstPtr[xd] = RgbaUC(acc/2);
        }
        dstPtr += dst.width();
        srcPtr1 += 2 * src.width();
        srcPtr2 += 2 * src.width();
    }
    return dst;
}

// RGBA -> mono uses rec709
void
imgConvert_(const ImgC4UC &src,ImgUC &dst)
{
    dst.resize(src.width(),src.height());
    for (size_t ii=0; ii<dst.numPixels(); ++ii)
        dst[ii] = src[ii].rec709();
}

void    imgConvert_(const ImgUC &src,ImgC4UC &dst)
{
    dst.resize(src.width(),src.height());
    for (size_t ii=0; ii<dst.numPixels(); ++ii) {
        uchar       val = src[ii];
        dst[ii] = RgbaUC(val,val,val,255);
    }
}

void
fgImgPntRescaleConvert(
    const ImgD    &src,
    ImgUC         &dst)
{
    VecD2        bounds = cBounds(src.dataVec());
    double          scale = bounds[1] - bounds[0];
    scale = (scale > 0) ? scale : 1.0;
    ImgD          adjusted;
    fgImgPntAffine(src,adjusted,-bounds[0],255.0/scale);
    scast_(adjusted,dst);
}

void
fgImgPntRescaleConvert(
    const ImgD    &src,
    ImgC4UC     &dst)
{
    ImgUC         tmp;
    fgImgPntRescaleConvert(src,tmp);
    imgConvert_(tmp,dst);
}

Affine2D
fgImgGetIrcsToBounds(
    uint                wid,
    uint                hgt,
    const Mat22D   &bounds)
{
    FGASSERT(wid*hgt>0);
    double          scaleX = (bounds.rc(0,1)-bounds.rc(0,0)) / double(wid),
                    scaleY = (bounds.rc(1,1)-bounds.rc(1,0)) / double(hgt);
    Mat22D     linear(scaleX,0.0,0.0,scaleY);
    Vec2D        centre = Vec2D(wid-1,hgt-1) * -0.5;
    return Affine2D(centre,linear);
}

void
fgImgResize(
	const ImgC4UC & src,
	ImgC4UC       & dst)
{
    FGASSERT(!src.empty());
    FGASSERT(!dst.empty());
    typedef ImgC4UC          ImageType;
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
	const PixelType *srcPtr = src.data();
	PixelType       *dstPtr	= dst.data();
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
			Rgba<float> acc, fpix;
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
					scast_(srcPtr[yy*srcStep + xx],fpix);
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

ImgC4UC
fgImgApplyTransparencyPow2(
    const ImgC4UC & colour,
    const ImgC4UC & transparency)
{
    FGASSERT(!colour.empty() && !transparency.empty());
    Vec2UI       dims = pow2Ceil(cMax(colour.dims(),transparency.dims()));
    ImgC4UC     ctmp(dims),
                    ttmp(dims);
    fgImgResize(colour,ctmp);
    fgImgResize(transparency,ttmp);

    for (Iter2UI it(dims); it.valid(); it.next())
        ctmp[it].alpha() = ttmp[it()].rec709();
    return ctmp;
}

ImgC4UC
fgResample(
    const Img2F &       map,
    const ImgC4UC &     src)
{
    ImgC4UC         ret(map.dims());
    Vec2F           noVal(-1,-1);
    for (Iter2UI it(ret.dims()); it.valid(); it.next()) {
        if (map[it()] != noVal) {
            Vec2F       pos = map[it()];
            pos[1] = 1.0f - pos[1];         // OTCS to UICS
            ret[it()] = RgbaUC(sampleClip(src,pos) + RgbaF(0.5));
        }
    }
    return ret;
}

bool
fgUsesAlpha(const ImgC4UC & img,uchar minVal)
{
    for (size_t ii=0; ii<img.numPixels(); ++ii) {
        if (img.m_data[ii].alpha() < minVal)
            return true;
    }
    return false;
}

void
fgPaintCrossHair(ImgC4UC & img,Vec2I ircs,Vec4UC c,uint thickness)
{
    RgbaUC    clr(c[0],c[1],c[2],c[3]);
    int         rad = int(thickness/2),
                hlen = int(thickness)*4;
    // Horizontal stroke:
    for (int yy=-rad; yy<=rad; ++yy)
        for (int xx=-hlen; xx<=hlen; ++xx)
            img.paint(ircs+Vec2I(xx,yy),clr);
    // Vertical stroke:
    for (int yy=-hlen; yy<=hlen; ++yy)
        for (int xx=-rad; xx<=rad; ++xx)
            img.paint(ircs+Vec2I(xx,yy),clr);
}

void
fgPaintDot(ImgC4UC & img,Vec2I ircs,Vec4UC c,uint radius)
{
    RgbaUC    clr(c[0],c[1],c[2],c[3]);
    int         rad = int(radius);
    for (int yy=-rad; yy<=rad; ++yy)
        for (int xx=-rad; xx<=rad; ++xx)
            img.paint(ircs+Vec2I(xx,yy),clr);
}

void
fgPaintDot(ImgC4UC & img,Vec2F ipcs,Vec4UC c,uint radius)
{
    fgPaintDot(img,Vec2I(cFloor(ipcs)),c,radius);
}

// Creates a mipmap from the given image; the original will be downsampled
// to a power of 2 if necessary:
vector<ImgC4UC>
fgMipMap(const ImgC4UC & img)
{
    vector<ImgC4UC>     ret(log2Ceil(cMinElem(img.dims()))); // To min elem size 2
    fgResizePow2Ceil_(img,ret[0]);   // Just copies if already pow2 dims
    for (size_t sl=1; sl<ret.size(); ++sl) {
        const ImgC4UC & src = ret[sl-1];
        ret[sl] = fgImgShrink2(src);
    }
    return ret;
}

Img3F
fgImgToF3(const ImgC4UC & img)
{
    Img3F     ret(img.dims());
    for (size_t pp=0; pp<ret.numPixels(); ++pp) {
        Arr4UC       p = img[pp].m_c.m;
        ret[pp] = {static_cast<float>(p[0]),static_cast<float>(p[1]),static_cast<float>(p[2])};
    }
    return ret;
}

Img3Fs
fgSsi(const Img3F & img,uchar borderPolicy)
{
    Img3Fs        ret(log2Floor(cMinElem(img.dims()))+1);
    ret[0] = img;
    for (size_t ii=0; ii<ret.size()-1; ++ii) {
        fgSmoothFloat(ret[ii],ret[ii],borderPolicy);
        fgSmoothFloat(ret[ii],ret[ii],borderPolicy);
        fgShrink2Float(ret[ii],ret[ii+1]);
    }
    return ret;
}

AffineEw2Fs
fgSsiItcsToIpcs(Vec2UI dims,Vec2F principalPointIpcs,Vec2F fovItcs)
{
    AffineEw2Fs       ret;
    do {
        Vec2F        pixPerTan = fgMapDiv(Vec2F(dims),fovItcs);
        ret.push_back(AffineEw2F(pixPerTan,principalPointIpcs));
        for (uint dd=0; dd<2; ++dd)
            if (dims[dd] & 1)
                fovItcs[dd] *= float(dims[dd]-1)/float(dims[dd]);
        dims /= 2U;
        principalPointIpcs *= 0.5f;
    } while (cMinElem(dims) > 1);
    return ret;
}

ImgC4UC
imgBlend(ImgC4UC const & img0,ImgC4UC const & img1,ImgUC const & transition)
{
    if (img0.empty())
        return img1;
    if (img1.empty() ||transition.empty())
        return img0;
    Vec2UI              dims = img0.dims();
    ImgC4UC             ret(dims);
    AffineEw2F          ircsToIucs = calcIrcsToIucs(dims);
    for (Iter2UI it(dims); it.valid(); it.next()) {
        Vec4F           a = scast<float>(img0[it()].m_c);
        Vec2F           iucs = ircsToIucs * Vec2F(it());
        RgbaF           b = sampleClip(img1,iucs);
        float           w = sampleClip(transition,iucs) / 255.0f;
        Vec4UC &        r = ret[it()].m_c;
        for (uint cc=0; cc<4; ++cc) {
            float           ac = a[cc],
                            bc = b.m_c[cc],
                            rc = (1-w) * ac + w * bc;
            r[cc] = rc;
        }
    }
    return ret;
}

ImgC4UC
imgModulate(ImgC4UC const & imgIn,ImgC4UC const & imgMod,float modulationFactor)
{
    if (imgMod.empty())
        return imgIn;
    ImgC4UC          ret;
    if (determinant(imgIn.dims(),imgMod.dims()) != 0) {
        string      info = toString(imgIn.dims())+" !~ "+toString(imgMod.dims());
        fgThrow("Aspect ratio mismatch between imgIn and modulation maps",info);
    }
    int             mod = int(modulationFactor*256.0f + 0.5f);
    if (imgMod.width() > imgIn.width()) {
        ret.resize(imgMod.dims());
        AffineEw2F      ircsToIucs = calcIrcsToIucs(ret.dims());
        for (Iter2UI it(ret.dims()); it.valid(); it.next()) {
            RgbaUC          td = imgMod[it()],
                            out(0,0,0,255);
            RgbaF           pd = sampleClip(imgIn,ircsToIucs*Vec2F(it()));
            for (uint cc=0; cc<3; ++cc) {
                int             gamMod = (((int(td.m_c[cc]) - 64) * mod) / 256) + 64;
                gamMod = (gamMod < 0) ? 0 : gamMod;
                uint            vv = uint(pd.m_c[cc]) * uint(gamMod);
                vv = vv >> 6;
                vv = (vv > 255) ? 255 : vv;
                out.m_c[cc] = (unsigned char)vv;
            }
            ret[it()] = out;
        }
    }
    else {
        ret.resize(imgIn.dims());
        AffineEw2F      ircsToIucs = calcIrcsToIucs(ret.dims());
        for (Iter2UI it(ret.dims()); it.valid(); it.next()) {
            RgbaF           td = sampleClip(imgMod,ircsToIucs*Vec2F(it()));
            RgbaUC          out(0,0,0,255),
                            pd = imgIn[it()];
            for (uint cc=0; cc<3; ++cc) {
                int             gamMod = (((int(td.m_c[cc]) - 64) * mod) / 256) + 64;
                gamMod = (gamMod < 0) ? 0 : gamMod;
                uint            vv = uint(pd.m_c[cc]) * uint(gamMod);
                vv = vv >> 6;
                vv = (vv > 255) ? 255 : vv;
                out.m_c[cc] = (unsigned char)vv;
            }
            ret[it()] = out;
        }
    }
    return ret;
}

}

// */
