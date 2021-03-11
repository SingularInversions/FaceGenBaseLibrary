//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
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
operator<<(std::ostream & os,ImgC4UC const & img)
{
    if (img.empty())
        return os << "Empty image";
    Mat<uchar,4,1>    init {img[0].m_c};
    Mat<uchar,4,2>    bounds = catHoriz(init,init);
    for (Iter2UI it(img.dims()); it.next(); it.valid()) {
        RgbaUC    pix = img[it()];
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

AffineEw2D
cIucsToIrcsXf(Vec2UI ircsDims)
{return {Mat22D{0,1,0,1},Mat22D{-0.5,ircsDims[0]-0.5,-0.5,ircsDims[1]-0.5}}; }

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
sampleAlpha(ImgC4UC const & img,Vec2F coordIucs)
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
    Vec2I               loXY = Vec2I(cFloor(ircs)),
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

bool
fgImgApproxEqual(ImgC4UC const & img0,ImgC4UC const & img1,uint maxDelta)
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
imgShrink2(ImgC4UC const & src,ImgC4UC & dst)
{
    FGASSERT(!src.empty());
    dst.resize(src.dims()/2);
    RgbaUC const    *srcPtr1 = src.data(),
                    *srcPtr2 = srcPtr1 + src.width();
    RgbaUC          *dstPtr =  dst.data();
    for (uint yd=0; yd<dst.height(); yd++) {
        for (uint xd=0; xd<dst.width(); xd++) {
            uint        xs1 = xd * 2,
                        xs2 = xs1 + 1;
            RgbaUS      acc = RgbaUS(srcPtr1[xs1]) +
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
expand2(ImgC4UC const & src)
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

ImgC4UC
toImgC4UC(ImgC4F const & img)
{
    ImgC4UC         ret(img.dims());
    for (size_t ii=0; ii<img.m_data.size(); ++ii)
        ret.m_data[ii] = mapCast<uchar>(img.m_data[ii]*255.999f);
    return ret;
}

ImgC4UC
toImgC4UC(Img3F const & img,float offset,float scale)
{
    ImgC4UC         ret(img.dims());
    for (size_t ii=0; ii<img.m_data.size(); ++ii) {
        Arr3F           in = img.m_data[ii];
        RgbaUC          out;
        for (size_t jj=0; jj<3; ++jj)
            out[jj] = scast<uchar>((in[jj]+offset)*scale);
        out[3] = 255;
        ret.m_data[ii] = out;
    }
    return ret;
}

// RGBA -> mono uses rec709
void
imgConvert_(ImgC4UC const &src,ImgUC &dst)
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
rescaleValsToFit_(
    const ImgD    &src,
    ImgUC         &dst)
{
    VecD2           bounds = cBounds(src.dataVec());
    double          scale = bounds[1] - bounds[0];
    scale = (scale > 0) ? scale : 1.0;
    dst.resize(src.width(),src.height());
    for (size_t ii=0; ii<dst.numPixels(); ++ii)
        dst[ii] = uchar((src[ii] - bounds[0]) * 255.0/scale);
}

void
rescaleValsToFit_(
    const ImgD    &src,
    ImgC4UC     &dst)
{
    ImgUC         tmp;
    rescaleValsToFit_(src,tmp);
    imgConvert_(tmp,dst);
}

void
imgResize(
	ImgC4UC const & src,
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

ImgC4UC
fgImgApplyTransparencyPow2(
    ImgC4UC const & colour,
    ImgC4UC const & transparency)
{
    FGASSERT(!colour.empty() && !transparency.empty());
    Vec2UI       dims = pow2Ceil(cMax(colour.dims(),transparency.dims()));
    ImgC4UC     ctmp(dims),
                    ttmp(dims);
    imgResize(colour,ctmp);
    imgResize(transparency,ttmp);

    for (Iter2UI it(dims); it.valid(); it.next())
        ctmp[it].alpha() = ttmp[it()].rec709();
    return ctmp;
}

ImgC4UC
fgResample(
    const Img2F &       map,
    ImgC4UC const &     src)
{
    ImgC4UC         ret(map.dims());
    Vec2F           noVal(-1,-1);
    for (Iter2UI it(ret.dims()); it.valid(); it.next()) {
        if (map[it()] != noVal) {
            Vec2F       pos = map[it()];
            pos[1] = 1.0f - pos[1];         // OTCS to UICS
            ret[it()] = RgbaUC(sampleClipIucs(src,pos) + RgbaF(0.5));
        }
    }
    return ret;
}

bool
usesAlpha(ImgC4UC const & img,uchar minVal)
{
    for (size_t ii=0; ii<img.numPixels(); ++ii) {
        if (img.m_data[ii].alpha() < minVal)
            return true;
    }
    return false;
}

void
paintCrosshair(ImgC4UC & img,Vec2I ircs)
{
    RgbaUC              centre {0,150,200,255},
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
paintDot(ImgC4UC & img,Vec2I ircs,Vec4UC c,uint radius)
{
    RgbaUC    clr(c[0],c[1],c[2],c[3]);
    int         rad = int(radius);
    for (int yy=-rad; yy<=rad; ++yy)
        for (int xx=-rad; xx<=rad; ++xx)
            img.paint(ircs+Vec2I(xx,yy),clr);
}

void
paintDot(ImgC4UC & img,Vec2F ipcs,Vec4UC c,uint radius)
{
    paintDot(img,Vec2I(cFloor(ipcs)),c,radius);
}

// Creates a mipmap from the given image; the original will be downsampled
// to a power of 2 if necessary:
ImgC4UCs
cMipMap(ImgC4UC const & img)
{
    ImgC4UCs        ret(log2Ceil(cMinElem(img.dims())));    // To min elem size 2
    ret[0] = isPow2(img.dims()) ? img : resampleToFit(img,pow2Ceil(img.dims()));
    for (size_t sl=1; sl<ret.size(); ++sl) {
        ImgC4UC const & src = ret[sl-1];
        ret[sl] = imgShrink2(src);
    }
    return ret;
}

ImgV3F
fgImgToF3(ImgC4UC const & img)
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
        smoothFloat(ret[ii],ret[ii],borderPolicy);
        smoothFloat(ret[ii],ret[ii],borderPolicy);
        shrink2Float_(ret[ii],ret[ii+1]);
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

ImgC4UC
imgBlend(ImgC4UC const & img0,ImgC4UC const & img1,ImgUC const & transition)
{
    if (img0.empty())
        return img1;
    if (img1.empty() || transition.empty())
        return img0;
    size_t              np0 = img0.numPixels(),
                        np1 = img1.numPixels();
    // Choose the larger image for output dimensions:
    Vec2UI              dims = (np0 > np1) ? img0.dims() : img1.dims();
    ImgC4UC             ret(dims);
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

ImgC4UC
composite(ImgC4UC const & foreground,ImgC4UC const & background)
{
    FGASSERT(foreground.dims() == background.dims());
    ImgC4UC                 ret(foreground.dims());
    for (size_t ii=0; ii<ret.numPixels(); ++ii)
        ret[ii] = compositeFragmentUnweighted(foreground[ii],background[ii]);
    return ret;
}

static
void
mod1(uint off,uint step,ImgC4UC const & imgIn,ImgC4UC const & imgMod,int mod,AffineEw2F ircsToIucs,ImgC4UC & ret)
{
    for (uint yy=off; yy<ret.m_dims[1]; yy+=step) {
        for (uint xx=0; xx<ret.m_dims[0]; ++xx) {
            RgbaUC          td = imgMod.xy(xx,yy),
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
mod2(uint off,uint step,ImgC4UC const & imgIn,ImgC4UC const & imgMod,int mod,AffineEw2F ircsToIucs,ImgC4UC & ret)
{
    for (uint yy=off; yy<ret.m_dims[1]; yy+=step) {
        for (uint xx=0; xx<ret.m_dims[0]; ++xx) {
            RgbaF           td = sampleClipIucs(imgMod,ircsToIucs*Vec2F(xx,yy));
            RgbaUC          out(0,0,0,255),
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

ImgC4UC
imgModulate(ImgC4UC const & imgIn,ImgC4UC const & imgMod,float modulationFactor)
{
    if (imgMod.empty())
        return imgIn;
    ImgC4UC          ret;
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
    RgbaUC const        *srcPtr,
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
    ImgC4UC const &     src,
    ImgC4UC &           dst,
    uchar               borderPolicy)
{
    FGASSERT((src.width() > 1) && (src.height() > 1));  // Algorithm not designed for dim < 2
    FGASSERT((borderPolicy == 0) || (borderPolicy == 1));
    dst.resize(src.width(),src.height());
    Img<RgbaUS> acc(src.width(),3);                  // Accumulator image
    RgbaUC           *dstPtr = dst.rowPtr(0);
    RgbaUS         *accPtr0,
                *accPtr1 = acc.rowPtr(0),
                *accPtr2 = acc.rowPtr(1);
    smoothUint1D(src.rowPtr(0),accPtr1,src.width(),borderPolicy);
    smoothUint1D(src.rowPtr(1),accPtr2,src.width(),borderPolicy);
    for (uint xx=0; xx<src.width(); xx++) {
        // Add 7 to minimize rounding bias. Adding 8 would bias the other way so we have to
        // settle for a small amount of downward rounding bias unless we want to pseudo-randomize:
        RgbaUS        tmp = accPtr1[xx]*(2+borderPolicy) + accPtr2[xx] + RgbaUS(7);
        dstPtr[xx] = RgbaUC(tmp / 16);
    }
    for (uint yy=1; yy<src.height()-1; ++yy) {
        dstPtr = dst.rowPtr(yy),
        accPtr0 = acc.rowPtr((yy-1)%3),
        accPtr1 = acc.rowPtr(yy%3),
        accPtr2 = acc.rowPtr((yy+1)%3);
        smoothUint1D(src.rowPtr(yy+1),accPtr2,src.width(),borderPolicy);
        for (uint xx=0; xx<src.width(); ++xx)
            dstPtr[xx] = RgbaUC((accPtr0[xx] + accPtr1[xx]*2 + accPtr2[xx] + RgbaUS(7)) / 16);
    }
    dstPtr = dst.rowPtr(dst.height()-1);
    for (uint xx=0; xx<dst.width(); ++xx)
        dstPtr[xx] = RgbaUC((accPtr1[xx] + accPtr2[xx]*(2+borderPolicy) + RgbaUS(7)) / 16);
}
ImgC4UC
smoothUint(ImgC4UC const & src,uchar borderPolicy)
{
    ImgC4UC             ret;
    smoothUint_(src,ret,borderPolicy);
    return ret;
}

}

// */
