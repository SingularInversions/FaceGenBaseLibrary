//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgImage.hpp"
#include "FgCamera.hpp"

using namespace std;

namespace Fg {

AxAffine2D          cItcsToIucs(Vec2D const & hfi)
{
    return {
        Vec2D{0.5/hfi[0],0.5/hfi[1]},
        Vec2D{0.5,0.5},
    };
}

Rigid3D             moveFmcsToFccs(Mat32D const & bounds,double halfFovTanMax)
{
    double              sz = cMaxElem(bounds * Vec2D{-1,1});
    Vec3D               centre = bounds * Vec2D{0.5,0.5};
    return Trans3D{0,0,0.5*sz/halfFovTanMax} * cRotateX(pi) *  Trans3D{-centre};
}

ScaleTrans2D        cItcsToPacs(double halfFovTanMax,Vec2UI imgDims)
{
    uint                pixMax = cMaxElem(imgDims);
    FGASSERT(pixMax > 0);
    double              scale = 0.5 * pixMax / halfFovTanMax;
    Vec2D               trans = Vec2D{imgDims} / 2;
    return {scale,trans};
}

ProjVerts           projectVerts(Vec3Ds const & vertsFccs,ScaleTrans2D const & itcsToPacs)
{
    auto                fn = [itcsToPacs](Vec3D const & v) -> ProjVert
    {
        if (v[2] > 0) {
            double          id = 1/v[2];
            Vec2D           itcs {v[0]*id,v[1]*id};
            return {itcsToPacs * itcs,id};
        }
        else
            return {{0,0},0};
    };
    return mapCall(vertsFccs,fn);
}

bool                isRendered(Arr<ProjVert,3> const & pvt)
{
    Arr3D               invDepths = mapMember(pvt,&ProjVert::invDepth);
    if (!allGtZero(invDepths))
        return false;                  // ignore any tris not entirely in front of camera plane
    Arr<Vec2D,3>        pacs = mapMember(pvt,&ProjVert::pacs);
    // PACS Z is "into" the image thus the signed area of CC winding will be negative for facets
    // facing the camera (ie. in the -Z direction):
    if (cArea(pacs) < 0)           // camera-facing non-degenerate
        return true;
    else
        return false;
}

Mat44F              Frustum::asD3dProjection() const
{
    // Projects from D3VS to D3PS. ie. maps:
    // * Near plane to Z=0, far plane to Z=1
    // * Left/right clip planes to X=-1/1
    // * Bottom/top clip planes to Y=-1/1
    double              a = nearDist / nearHalfWidth,
                        b = nearDist / nearHalfHeight,
                        del = farDist - nearDist,
                        c = farDist / del,
                        d = -nearDist * farDist / del;
    Mat44D              ret
    {
        a, 0, 0, 0,
        0, b, 0, 0,
        0, 0, c, d,
        0, 0, 1, 0
    };
    return Mat44F{ret};
}

ostream &           operator<<(ostream & os,Frustum const & f)
{
    return os << fgnl
        << "nearHalfWidth: " << f.nearHalfWidth
        << "nearHalfHeight: " << f.nearHalfHeight
        << "nearDist: " << f.nearDist
        << "farDist: " << f.farDist;
}

Mat44F              Camera::projectPacs(Vec2UI dims) const
{
    Mat44D          projection {
        1, 0, 0, 0,
        0,-1, 0, 0,
        0, 0, 0, 1,
        0, 0,-1, 0
    };
    AxAffine2D      iucsToPacs = cIucsToPacs<double>(dims),
                    itcsToPacs = iucsToPacs * itcsToIucs;
    Mat44D          itcsToPacs4H(0);
    itcsToPacs4H.rc(0,0) = itcsToPacs.scales[0];
    itcsToPacs4H.rc(1,1) = itcsToPacs.scales[1];
    itcsToPacs4H.rc(0,3) = itcsToPacs.trans[0];
    itcsToPacs4H.rc(1,3) = itcsToPacs.trans[1];
    itcsToPacs4H.rc(2,2) = 1;
    itcsToPacs4H.rc(3,3) = 1;
    return Mat44F(itcsToPacs4H * projection * asHomogMat(modelview));
}

CameraSquare::CameraSquare(Mat32D const & mdlBnds,double hFovTan,uint imgDim,double fillRatio)
{
    FGASSERT(hFovTan > 0);
    FGASSERT(imgDim > 0);
    FGASSERT(fillRatio > 0);
    Vec3D           centre = mdlBnds * Vec2D{0.5},
                    sz = mdlBnds * Vec2D{-1,1};
    FGASSERT(allGtZero(sz.m));
    double          maxDim = cMaxElem(sz),
                    camDist = 0.5 * maxDim / (hFovTan * fillRatio);
    toFccs = Trans3D{0,0,camDist} * cRotateX(pi) * Trans3D{-centre};
    double          hPix = imgDim * 0.5;
    itcsToPacs = {hPix/hFovTan,{hPix,hPix}};
}

Camera              CameraParams::camera(Vec2UI imgDims) const
{
    FGASSERT(imgDims.elemsProduct() > 0);
    Vec3D               dims = modelBounds * Vec2D{-1,1},
                        centre = modelBounds * Vec2D{0.5,0.5};
    double              maxDim = cMaxElem(dims);
    if (cMinElem(dims) < 0) {       // handle unset bounds
        maxDim = 2;
        centre = {0,0,0};
    }
    else if (maxDim == 0)           // Handle degenerate bounds
        maxDim = 2;
    Vec3D               trans {relTrans[0],relTrans[1],0};
    // Hack orthographic by relying on precision (below 0.01 degrees we get visible Z-fighting)
    double              fovDegClamp = clamp(fovMaxDeg,0.01,120.0),
                        modelHalfDimMax = maxDim * 0.5,
                        imgDimMax = cMaxElem(imgDims),
                        relScale = exp(logRelScale),
                        halfFovMaxItcs = std::tan(degToRad(fovDegClamp) * 0.5),
                        // Place the model at a distance such that it's max dim is equal to the given FOV max dim:
                        zCentreFillImage = modelHalfDimMax / halfFovMaxItcs,
                        // Adjust the distance to relatively scale the object:
                        zCentre = zCentreFillImage / relScale;
    if (cMinElem(imgDims) == 0)
        imgDims = Vec2UI(1);     // Avoid NaNs
    Vec2D               aspect = Vec2D(imgDims) / imgDimMax;
    double              // sqrt(3) ~= 1.7 is distance to BB corner relative to distance to plane:
                        zfar = zCentre + modelHalfDimMax * 1.7,
                        znearRaw = zCentre - modelHalfDimMax * 1.7,
                        zmin = zfar * 0.01,     // Precision issues below this
                        znear = cMax(znearRaw,zmin),
                        frustumNearHalfWidth = modelHalfDimMax * znear / zCentreFillImage;
    trans *= modelHalfDimMax;
    trans[2] = -zCentre;
    Camera              ret;
    ret.modelview =  SimilarityD{trans} * SimilarityD{pose} * SimilarityD{-centre};
    ret.frustum.nearHalfWidth  = frustumNearHalfWidth * aspect[0];
    ret.frustum.nearHalfHeight = frustumNearHalfWidth * aspect[1];
    ret.frustum.nearDist = znear;
    ret.frustum.farDist = zfar;
    Vec2D               halfFovItcs = aspect * halfFovMaxItcs;
    ret.itcsToIucs = cItcsToIucs(halfFovItcs);
    return ret;
}

ImgRgba8
Lighting::createSpecularMap() const
{
    FGASSERT(lights.size() > 0);

    float   fresnelLow = 1.5f,      // 90 degree angle brightness
            fresnelHigh = 1.5f,     // 0 degree angle brightness
            falloffStd = 0.1f;      // Must be greater than 0 and less than 1/sqrt(2).

    float                    invVar = 1.0f / (2.0f * sqr(falloffStd));
    ImgRgba8              img(128,128);
    for (uint py=0; py<128; py++) {
        float    yy = ((float)py - 63.5f) / 64.0f;
        for (uint px=0; px<128; px++) {
            float   xx = ((float)px - 63.5f) / 64.0f,
                    sq = xx*xx + yy*yy;
            if (sq > 1.0f)                    // Outside valid spherical region.
                img.xy(px,py) = Rgba8(0,0,0,1);
            else {
                float   aa = 2.0f * sqrt(1.0f - sq),
                        sr = sqrt(sq),
                        fresnel = (1.0f - sr) * fresnelLow + sr * fresnelHigh;
                Vec3F        r(aa*xx,aa*yy,1.0f-2.0f*sq);
                RgbaF         pix(0,0,0,255);
                for (uint ll=0; ll<lights.size(); ll++)
                {
                    float       diffSqr = (r - lights[ll].direction).magD(),
                                bright = exp(-diffSqr * invVar) * fresnel * 255.0f;
                    pix.red() += lights[ll].colour[0] * bright;
                    pix.green() += lights[ll].colour[1] * bright;
                    pix.blue() += lights[ll].colour[2] * bright;
                }
                if (pix.red() > 255.0f) pix.red() = 255.0f;
                if (pix.green() > 255.0f) pix.green() = 255.0f;
                if (pix.blue() > 255.0f) pix.blue() = 255.0f;
                mapCast_(pix,img.xy(px,py));
            }
        }
    }
    return img;
}

}
