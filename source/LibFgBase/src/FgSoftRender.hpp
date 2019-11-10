//
// Copyright (c) 2019 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Anti-aliased ray-casting software renderer
//

#ifndef FG_SOFTRENDER_HPP
#define FG_SOFTRENDER_HPP

#include "Fg3dMesh.hpp"
#include "Fg3dNormals.hpp"
#include "FgLighting.hpp"
#include "FgImage.hpp"

namespace Fg {

enum class FgRenderSurfPoints { never, whenVisible, always };

struct  FgProjSurfPoint
{
    String          label;
    Vec2F           posIucs;    // Not necessarily in image
    bool            visible;    // In view of camera, not occluded, camera facing
};
typedef Svec<FgProjSurfPoint>   FgProjSurfPoints;

struct  FgRenderOptions
{
    FgLighting          lighting;   // In OECS (not transformed)
    // Values in range [0,255]. Alpha = 0 is transparent and all color values must be alpha-weighted:
    RgbaF             backgroundColor=RgbaF(0);
    // Values in range [1,8]. Higher is slower:
    uint                antiAliasBitDepth=3;
    // Render marked surface points in meshes as green dots:
    FgRenderSurfPoints  renderSurfPoints=FgRenderSurfPoints::never;
    // If defined, place the projected surface point data here:
    std::shared_ptr<FgProjSurfPoints> projSurfPoints;

    FG_SERIALIZE4(lighting,backgroundColor,antiAliasBitDepth,renderSurfPoints);
};

ImgC4UC
renderSoft(
    Vec2UI                  pixelSize,
    Meshes const &           meshes,
    Affine3D                modelview,              // Transform verts into OECS
    // This fully specifies the projection transform since we assume the optical centre is at the centre of the
    // image and the bounds are implicitly [0,1] in IUCS:
    AffineEw2D              itcsToIucs,
    FgRenderOptions const & options=FgRenderOptions());

#endif

}

// */
