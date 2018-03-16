//
// Copyright (c) 2015 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Authors:     Andrew Beatty
// Created:     April 6, 2011
//
// Parameterized 3D camera transforms
//

#ifndef FG3DCAMERA_HPP
#define FG3DCAMERA_HPP

#include "FgQuaternion.hpp"
#include "FgAffineC.hpp"
#include "FgAffineCwC.hpp"

struct  Fg3dCamera
{
    FgAffine3D      modelview;      // Transform to OECS
    FgVect6D        frustum;        // Defines camera projection for OpenGL
    FgAffineCw2D    itcsToIucs;     // Defines projection for raycasting

    // Projection from world to IPCS (and depth to inverse depth) for this camera (homogenous representation):
    FgMat44F
    projectIpcs(FgVect2UI dims) const;
};

struct  Fg3dCameraParams
{
    FgMat32D        modelBounds;
    // Model rotation around centre of model bounds:
    FgQuaternionD   pose;
    // Model translation parallel to image plane, relative to half max model bound:
    FgVect2D        relTrans;
    // Scale relative to automatically determined size of object in image:
    double          logRelScale;
    // Field of view of larger image dimension (degrees). Must be > 0 but can set as low as 0.0001
    // to simulate orthographic projection:
    double          fovMaxDeg;

    // By default, scale the object a bit smaller than the image to leave some boundary and
    // some room for perspective enlargement of parts closer than the median plane:
    Fg3dCameraParams() : logRelScale(-0.1), fovMaxDeg(17) {}

    explicit
    Fg3dCameraParams(FgMat32D bounds)
    : modelBounds(bounds), logRelScale(-0.1), fovMaxDeg(17) {}

    Fg3dCamera
    camera(FgVect2UI viewport) const;
};

#endif
