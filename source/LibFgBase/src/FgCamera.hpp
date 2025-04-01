//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
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
// OECS := OpenGL Eye CS / Camera Frame / same as FHCS
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
//    Z - inverse depth (near to far)  [-1,1]
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

#ifndef FG3DCAMERA_HPP
#define FG3DCAMERA_HPP

#include "FgTransform.hpp"
#include "FgTransform.hpp"
#include "FgTransform.hpp"

namespace Fg {

AxAffine2D          cItcsToIucs(Vec2D const & halfFovItcs);     // Assuming principal point at centre of image
inline QuaternionD  cOecsToFccs() {return cRotateX(pi); }     // Same as negating Y,Z
inline QuaternionD  cFhcsToFccs() {return cRotateX(pi); }     // "
// given the model bounds and the camera tangent value of the larger half-field-of-view, return the rigid transform that moves
// the object to so it's bounds fill the image and rotates from FMCS to FCCS (aka modelview transform):
Rigid3D             moveFmcsToFccs(Mat32D const & mdlBounds,double halfFovTanMax);
// given the tangent value of the larger half-field-of-view, and the image pixel size,
// return the transform from ITCS to PACS (assumes principal point at centre):
ScaleTrans2D        cItcsToPacs(double halfFovTanMax,Vec2UI imgDims);

struct      ProjVert
{
    Vec2D               pacs;           // where vertex projects into the render image (PACS)
    double              invDepth {0};   // FCCS inverse depth > 0. If zero, 'pacs' is not valid. Never negative.

    bool                valid() const {return (invDepth != 0); }
};
typedef Svec<ProjVert>  ProjVerts;
typedef Svec<ProjVerts> ProjVertss;

// vertices with FCCS Z <= 0 are set to invalid:
ProjVerts           projectVerts(Vec3Ds const & vertsFccs,ScaleTrans2D const & itcsToPacs);
// returns true if the given tri is entirely in front of the camera plane, non-degenerate, and camera-facing:
bool                isRendered(Arr<ProjVert,3> const & pvt);

// Only represents frustums with principal point at centre:
struct      Frustum
{
    double          nearHalfWidth;      // > 0
    double          nearHalfHeight;     // > 0
    double          nearDist;           // > 0
    double          farDist;            // > nearDist

    // transform from D3D View Space (D3VS) to D3D Projection Space (D3PS):
    Mat44F          asD3dProjection() const;
};
std::ostream &      operator<<(std::ostream &,Frustum const &);

struct      Camera
{
    SimilarityD     modelview;      // Transform world to OECS
    Frustum         frustum;        // Defines projection for GPUs
    AxAffine2D      itcsToIucs;     // Defines projection for raycasting
    // Projection from world to PACS (and depth to inverse depth) for this camera (homogeneous representation):
    Mat44F          projectPacs(Vec2UI dims) const;
};

struct      CameraSquare                    // simple square image camera
{
    Rigid3D             toFccs;
    ScaleTrans2D        itcsToPacs;

    // the camera assumes the model is oriented Y upward and facing in the Z direction (FMCS) and will
    // view it in the -Z direction from a +Z distance:
    CameraSquare(
        Mat32D const &      modelBounds,
        double              halfFovTan,         // Camera half FOV tangent value
        uint                imgDim,             // pixel dimension
        double              fillRatio=0.7);     // fill ratio of image of max bound at centre distance from camera
};

struct      CameraParams
{
    Mat32D              modelBounds;            // will handle unset or degenerate values
    // Model rotation around centre of model bounds:
    QuaternionD         pose;
    // Model translation parallel to image plane, relative to half max model bound:
    Vec2D               relTrans = Vec2D(0);
    // Scale relative to automatically determined size of object in image.
    // By default, scale the object a bit smaller than the image to leave some boundary and
    // some room for perspective enlargement of parts closer than the median plane:
    double              logRelScale = -0.1;
    // Field of view of larger image dimension (degrees). Must be > 0 but can set as low as 0.0001
    // to simulate orthographic projection:
    double              fovMaxDeg = 17.0;

    CameraParams() {}
    explicit CameraParams(Mat32D bounds) : modelBounds(bounds) {}

    Camera          camera(Vec2UI viewport) const;
};

struct  Light
{
    Vec3F       colour {0};             // RGB range [0,1]
    Vec3F       direction {0,0,1};      // Unit direction vector to light in OECS (all lights at infinity)
    FG_SER(colour,direction)

    Light() {}
    Light(Vec3F c) : colour(c) {}
    Light(Vec3F c,Vec3F d) : colour(c), direction(d) {}
};
typedef Svec<Light>    Lights;

struct  Lighting
{
    Vec3F               ambient;    // RGB range [0,1]
    Lights              lights;
    FG_SER(ambient,lights)

    Lighting() : ambient{0.4f}, lights{Light{Vec3F{0.6f}}} {}
    Lighting(Vec3F a) : ambient{a} {}
    Lighting(Vec3F a,Light l) : ambient{a}, lights{l} {}
    Lighting(Vec3F a,Lights const & l) : ambient{a}, lights{l} {}
    Lighting(Light const & l) : ambient{0}, lights{l} {}

    ImgRgba8
    createSpecularMap() const;
};

}

#endif
