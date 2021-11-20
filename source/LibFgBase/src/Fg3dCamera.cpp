//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "Fg3dCamera.hpp"
#include "FgMath.hpp"
#include "FgQuaternion.hpp"
#include "FgBounds.hpp"
#include "FgAffine.hpp"

using namespace std;

namespace Fg {

AffineEw2D
cItcsToIucs(Vec2D const & hfi)
{
    Mat22D          domain {
        -hfi[0],    hfi[0],
        -hfi[1],    hfi[1],
    },
                    range {
        0,          1,
        0,          1,
    };
    return AffineEw2D {domain,range};
}

ostream &
operator<<(ostream & os,Frustum const & f)
{
    return os << fgnl
        << "nearHalfWidth: " << f.nearHalfWidth
        << "nearHalfHeight: " << f.nearHalfHeight
        << "nearDist: " << f.nearDist
        << "farDist: " << f.farDist;
}

Mat44F
Camera::projectIpcs(Vec2UI dims) const
{
    Mat44D        projection = projectOecsToItcs<double>();
    AffineEw2D    iucsToIpcs(Mat22D(0,1,0,1),Mat22D(0,dims[0],0,dims[1])),
                    itcsToIpcs = iucsToIpcs * itcsToIucs;
    Mat44D        itcsToIpcs4H(0);
    itcsToIpcs4H.rc(0,0) = itcsToIpcs.m_scales[0];
    itcsToIpcs4H.rc(1,1) = itcsToIpcs.m_scales[1];
    itcsToIpcs4H.rc(0,3) = itcsToIpcs.m_trans[0];
    itcsToIpcs4H.rc(1,3) = itcsToIpcs.m_trans[1];
    itcsToIpcs4H.rc(2,2) = 1;
    itcsToIpcs4H.rc(3,3) = 1;
    return Mat44F(itcsToIpcs4H * projection * asHomogMat(modelview));
}

Camera
CameraParams::camera(Vec2UI imgDims) const
{
    FGASSERT(imgDims.cmpntsProduct() > 0);
    Camera          ret;
    Vec3D           min = modelBounds.colVec(0),
                    max = modelBounds.colVec(1),
                    dims = max - min,
                    centre = (min+max) * 0.5,
                    trans(relTrans[0],relTrans[1],0.0);
    // Handle degenerate model bounds:
    if (dims == Vec3D(0))
        dims = Vec3D(1);
    else if (dims.cmpntsProduct() == 0)
        dims = Vec3D(cMaxElem(dims));
    // Hack orthographic by relying on precision (below 0.01 degrees we get visible Z-fighting)
    double          fovDegClamp = clamp(fovMaxDeg,0.01,120.0),
                    modelHalfDimMax = cMaxElem(dims) * 0.5,
                    imgDimMax = cMaxElem(imgDims),
                    relScale = exp(logRelScale),
                    halfFovMaxItcs = std::tan(degToRad(fovDegClamp) * 0.5),
                    // Place the model at a distance such that it's max dim is equal to the given FOV max dim:
                    zCentreFillImage = modelHalfDimMax / halfFovMaxItcs,
                    // Adjust the distance to relatively scale the object:
                    zCentre = zCentreFillImage / relScale;
    if (cMinElem(imgDims) == 0)
        imgDims = Vec2UI(1);     // Avoid NaNs
    Vec2D        aspect = Vec2D(imgDims) / imgDimMax;
    double          // sqrt(3) ~= 1.7 is distance to BB corner relative to distance to plane:
                    zfar = zCentre + modelHalfDimMax * 1.7,
                    znearRaw = zCentre - modelHalfDimMax * 1.7,
                    zmin = zfar * 0.01,     // Precision issues below this
                    znear = cMax(znearRaw,zmin),
                    frustumNearHalfWidth = modelHalfDimMax * znear / zCentreFillImage;
    trans *= modelHalfDimMax;
    trans[2] = -zCentre;
    ret.modelview =  SimilarityD{trans} * SimilarityD{pose} * SimilarityD{-centre};
    ret.frustum.nearHalfWidth  = frustumNearHalfWidth * aspect[0];
    ret.frustum.nearHalfHeight = frustumNearHalfWidth * aspect[1];
    ret.frustum.nearDist = znear;
    ret.frustum.farDist = zfar;
    Vec2D        halfFovItcs = aspect * halfFovMaxItcs;
    ret.itcsToIucs = AffineEw2D(
        Mat22D(-halfFovItcs[0],halfFovItcs[0],-halfFovItcs[1],halfFovItcs[1]),
        Mat22D(0,1,0,1));
    return ret;
}

}
