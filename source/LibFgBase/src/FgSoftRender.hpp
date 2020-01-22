//
// Coypright (c) 2020 Singular Inversions Inc. (facegen.com)
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
#include "FgSimilarity.hpp"
#include "Fg3dCamera.hpp"

namespace Fg {

enum class RenderSurfPoints { never, whenVisible, always };

struct  ProjectedSurfPoint
{
    String          label;
    Vec2F           posIucs;    // Not necessarily in image
    bool            visible;    // In view of camera, not occluded, camera facing
};
typedef Svec<ProjectedSurfPoint>   ProjectedSurfPoints;

struct  RenderOptions
{
    Lighting            lighting;   // In OECS (not transformed)
    // Values in range [0,255]. Alpha = 0 is transparent and all color values must be alpha-weighted:
    RgbaF               backgroundColor=RgbaF(0);
    // Values in range [1,8]. Higher is slower:
    uint                antiAliasBitDepth=3;
    // Render marked surface points in meshes as green dots:
    RenderSurfPoints    renderSurfPoints=RenderSurfPoints::never;
    // If defined, place the projected surface point data here:
    Sptr<ProjectedSurfPoints> projSurfPoints;
    bool                useMaps = true;     // Turn off to see raw geometry
    bool                allShiny = false;

    FG_SERIALIZE6(lighting,backgroundColor,antiAliasBitDepth,renderSurfPoints,useMaps,allShiny);
};

struct  RenderXform
{
    SimilarityD            modelview;
    AffineEw2D             itcsToIucs;

    explicit RenderXform(Camera const & c) : modelview(c.modelview), itcsToIucs(c.itcsToIucs) {}
    RenderXform(SimilarityD const & s,AffineEw2D const & a) : modelview(s), itcsToIucs(a) {}
};

inline RenderXform
interpolate(RenderXform const & rx0,RenderXform const & rx1,double val)
{
    return RenderXform {
        interpolateAsModelview(rx0.modelview,rx1.modelview,val),
        interpolate(rx0.itcsToIucs,rx1.itcsToIucs,val)
    };
}

ImgC4UC
renderSoft(
    Vec2UI                  pixelSize,
    Meshes const &          meshes,
    SimilarityD             meshToOecs,     // aka Modelview
    // This fully specifies the projection transform since we assume the optical centre is at the centre of the
    // image and the bounds are implicitly [0,1] in IUCS:
    AffineEw2D              itcsToIucs,
    RenderOptions const &   options=RenderOptions());

inline
ImgC4UC
renderSoft(
    Vec2UI                  pixelSize,
    Meshes const &          meshes,
    RenderXform const &     transform,
    RenderOptions const &   options=RenderOptions())
{return renderSoft(pixelSize,meshes,transform.modelview,transform.itcsToIucs,options); }

// Render with default camera:
ImgC4UC
renderSoft(
    Vec2UI                  pixelSize,
    Meshes const &          meshes,
    RgbaF                   bgColor);

#endif

}

// */
