//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

//
// Parameterized 3D camera transforms
//

#ifndef FG3DCAMERA_HPP
#define FG3DCAMERA_HPP

#include "FgQuaternion.hpp"
#include "FgAffineC.hpp"
#include "FgAffineCwC.hpp"

namespace Fg {

// Only represent frustums with principal point at centre and far plane at infinity:
struct  Frustum
{
    double          nearHalfWidth;      // > 0
    double          nearHalfHeight;     // > 0
    double          nearDist;           // > 0
    double          farDist;            // > nearDist

    FG_SERIALIZE4(nearHalfWidth,nearHalfHeight,nearDist,farDist);
};

struct  Camera
{
    Affine3D        modelview;      // Transform world to OECS
    Frustum         frustum;        // Defines camera projection
    AffineEw2D      itcsToIucs;     // Defines projection for raycasting

    FG_SERIALIZE3(modelview,frustum,itcsToIucs);

    // Projection from world to IPCS (and depth to inverse depth) for this camera (homogenous representation):
    Mat44F
    projectIpcs(Vec2UI dims) const;
};

struct  CameraParams
{
    Mat32D          modelBounds;
    // Model rotation around centre of model bounds:
    QuaternionD   pose;
    // Model translation parallel to image plane, relative to half max model bound:
    Vec2D           relTrans;
    // Scale relative to automatically determined size of object in image:
    double          logRelScale;
    // Field of view of larger image dimension (degrees). Must be > 0 but can set as low as 0.0001
    // to simulate orthographic projection:
    double          fovMaxDeg;

    // By default, scale the object a bit smaller than the image to leave some boundary and
    // some room for perspective enlargement of parts closer than the median plane:
    CameraParams() : logRelScale(-0.1), fovMaxDeg(17) {}

    explicit
    CameraParams(Mat32D bounds)
    : modelBounds(bounds), logRelScale(-0.1), fovMaxDeg(17) {}

    Camera
    camera(Vec2UI viewport) const;
};

}

#endif
