//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     April 6, 2011
//

#include "stdafx.h"

#include "Fg3dCamera.hpp"
#include "FgMath.hpp"
#include "FgQuaternion.hpp"
#include "FgBounds.hpp"
#include "FgAffineC.hpp"
#include "FgAffineCwC.hpp"
#include "FgCoordSystem.hpp"

using namespace std;
using namespace fgMath;

FgMat44F
Fg3dCamera::projectIpcs(FgVect2UI dims) const
{
    FgMat44D        projection = fgProjectOecsToItcs<double>();
    FgAffineCw2D    iucsToIpcs(FgMat22D(0,1,0,1),FgMat22D(0,dims[0],0,dims[1])),
                    itcsToIpcs = iucsToIpcs * itcsToIucs;
    FgMat44D        itcsToIpcs4H(0);
    itcsToIpcs4H.rc(0,0) = itcsToIpcs.m_scales[0];
    itcsToIpcs4H.rc(1,1) = itcsToIpcs.m_scales[1];
    itcsToIpcs4H.rc(0,3) = itcsToIpcs.m_trans[0];
    itcsToIpcs4H.rc(1,3) = itcsToIpcs.m_trans[1];
    itcsToIpcs4H.rc(2,2) = 1;
    itcsToIpcs4H.rc(3,3) = 1;
    return FgMat44F(itcsToIpcs4H * projection * modelview.asHomogenous());
}

Fg3dCamera
Fg3dCameraParams::camera(FgVect2UI imgDims) const
{
    Fg3dCamera      ret;
    FgVect3D        min = modelBounds.colVec(0),
                    max = modelBounds.colVec(1),
                    dims = max - min,
                    centre = (min+max) * 0.5,
                    trans(relTrans[0],relTrans[1],0.0);
    // Handle degenerate model bounds:
    if (dims == FgVect3D(0))
        dims = FgVect3D(1);
    else if (dims.cmpntsProduct() == 0)
        dims = FgVect3D(fgMaxElem(dims));
    // Hack orthographic by relying on precision:
    double          fovMaxDegClamp = (fovMaxDeg < 0.0001) ? 0.0001 : fovMaxDeg;
    double          modelHalfDimMax = fgMaxElem(dims) * 0.5,
                    imgDimMax = fgMaxElem(imgDims),
                    relScale = exp(logRelScale),
                    halfFovMaxItcs = std::tan(fgDegToRad(fovMaxDegClamp) * 0.5),
                    // Place the model at a distance such that it's max dim is equal to the given FOV max dim:
                    zCentreFillImage = modelHalfDimMax / halfFovMaxItcs,
                    // Adjust the distance to relatively scale the object:
                    zCentre = zCentreFillImage / relScale;
    if (fgMinElem(imgDims) == 0)
        imgDims = FgVect2UI(1);     // Avoid NaNs
    FgVect2D        aspect = FgVect2D(imgDims) / imgDimMax;
    // Limit how close the object can be to the camera, to avoid clipping while still keeping the
    // near clip plane just far enough from the camera that depth precision isn't compromised:
    if (zCentre < modelHalfDimMax * 1.3)
        zCentre = modelHalfDimMax * 1.3;
    double          znear = zCentre - modelHalfDimMax * 1.2,
                    zfar = zCentre + modelHalfDimMax * 1.2,
                    frustumNearHalfWidth = modelHalfDimMax * znear / zCentreFillImage;
    trans *= modelHalfDimMax;
    trans[2] = -zCentre;
    ret.modelview =  FgAffine3D(trans) * FgAffine3D(-centre,pose.asMatrix());
    ret.frustum = FgVect6D(
        -frustumNearHalfWidth*aspect[0],
        frustumNearHalfWidth*aspect[0],
        -frustumNearHalfWidth*aspect[1],
        frustumNearHalfWidth*aspect[1],
        znear,zfar);
    FgVect2D        halfFovItcs = aspect * halfFovMaxItcs;
    ret.itcsToIucs = FgAffineCw2D(
        FgMat22D(-halfFovItcs[0],halfFovItcs[0],-halfFovItcs[1],halfFovItcs[1]),
        FgMat22D(0,1,0,1));
    return ret;
}
