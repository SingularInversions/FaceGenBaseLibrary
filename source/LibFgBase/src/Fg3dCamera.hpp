//
// Coypright (c) 2021 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Parameterized 3D camera transforms
//
// Coordinate system definitions:
//
// FHCS := FaceGen Head CS
//    Origin in face centre on saggital plane
//    Units are base head millimetres ie. an IPD of ~ 63.5mm on the base head.
//    X - Head’s left.
//    Y - Head’s up.
//    Z - Head’s forward. 
//
// FCCS := FaceGen Camera CS
//    Origin at effective pinhole, no units specified
//    X - camera’s right
//    Y - camera’s down
//    Z - camera’s facing 
//
// ITCS := Image Tangent CS
//    Origin at principal point (where optical axis intersects image plane), units are tangent of view angle.
//    X - viewer’s right
//    Y - viewer’s down 
//
// OpenGL Overview:
//    modelview: World frame -> OECS
//    projection matrix: OECS -> ONDCS
//    viewport transform: ONDCS -> OWCS 
//
// OECS := OpenGL Eye CS / Camera Frame
//    Origin at effective pinhole, no units specified.
//    X - viewer’s right
//    Y - viewer’s up
//    Z - opposite of camera viewing direction
//
// OITCS := OpenGL image tangent CS
//    After dividing an OECS coord by -z you get tangent image coordinates with origin at principal point:
//    X - viewer's right
//    Y - viewer's up
//
// ONDCS := OpenGL Normalized Device CS / Canonical Clipping Volume (LEFT-handed CS)
//    X - left to right [-1,1]
//    Y - bottom to top [-1,1]
//    Z - near to far   [-1,1]
//    The view volume (frustum) is transformed into (x,y,z) in the range [-1,1].
//    Note that the Z values are inverted, so the most distant objects have the largest Z values
//    and the values are non-linear (one over the distance from the camera). 
//
// OICS := OpenGL Image CS
//    Informal term referring to just the X,Y elements of ONDCS:
//    X - left to right [-1,1]
//    Y - bottom to top [-1,1]
//
// OWCS := OpenGL Window CS (Screen Coordinates once z is dropped)
//    X - viewer’s right    [0,1]
//    Y - viewer’s up       [0,1]
//    Z - Negative inverse of the depth in frustum, used for z-buffer.
//
// OTCS := OGL Texture CS
//    X - viewer’s right    [0,1]
//    Y - viewer’s up       [0,1]
//

#ifndef FG3DCAMERA_HPP
#define FG3DCAMERA_HPP

#include "FgQuaternion.hpp"
#include "FgAffineC.hpp"
#include "FgAffineCwC.hpp"
#include "FgSimilarity.hpp"

namespace Fg {

inline AffineEw2D   cOtcsToIucs() {return AffineEw2D {Vec2D{1,-1},Vec2D{0,1}}; }
inline AffineEw2D   cIucsToOtcs() {return AffineEw2D {Vec2D{1,-1},Vec2D{0,1}}; }
inline AffineEw2D   cOtcsToIpcs(Vec2UI imgDims) {return AffineEw2D {Mat22D(0,1,1,0),Mat22D(0,imgDims[0],0,imgDims[1])}; }
inline QuaternionD  cOecsToFccs() {return QuaternionD(pi(),0); }      // Same as negating Y,Z
AffineEw2D          cItcsToIucs(Vec2D const & halfFovItcs);     // Assuming principal point at centre of image

template<typename T>
Mat<T,4,4>
projectOecsToItcs()
{
    return Mat<T,4,4> {{{
        1, 0, 0, 0,
        0,-1, 0, 0,
        0, 0, 0, 1,
        0, 0,-1, 0
    }}};
}

// Only represent frustums with principal point at centre and far plane at infinity:
struct  Frustum
{
    double          nearHalfWidth;      // > 0
    double          nearHalfHeight;     // > 0
    double          nearDist;           // > 0
    double          farDist;            // > nearDist

    FG_SERIALIZE4(nearHalfWidth,nearHalfHeight,nearDist,farDist);
};
std::ostream &
operator<<(std::ostream &,Frustum const &);

struct  Camera
{
    SimilarityD     modelview;      // Transform world to OECS
    Frustum         frustum;        // Defines camera projection
    AffineEw2D      itcsToIucs;     // Defines projection for raycasting

    FG_SERIALIZE3(modelview,frustum,itcsToIucs);

    // Projection from world to IPCS (and depth to inverse depth) for this camera (homogeneous representation):
    Mat44F          projectIpcs(Vec2UI dims) const;
};

struct  CameraParams
{
    Mat32D          modelBounds;
    // Model rotation around centre of model bounds:
    QuaternionD     pose;
    // Model translation parallel to image plane, relative to half max model bound:
    Vec2D           relTrans = Vec2D(0);
    // Scale relative to automatically determined size of object in image.
    // By default, scale the object a bit smaller than the image to leave some boundary and
    // some room for perspective enlargement of parts closer than the median plane:
    double          logRelScale = -0.1;
    // Field of view of larger image dimension (degrees). Must be > 0 but can set as low as 0.0001
    // to simulate orthographic projection:
    double          fovMaxDeg = 17.0;

    CameraParams() {}
    explicit CameraParams(Mat32D bounds) : modelBounds(bounds) {}

    Camera          camera(Vec2UI viewport) const;
};

}

#endif
