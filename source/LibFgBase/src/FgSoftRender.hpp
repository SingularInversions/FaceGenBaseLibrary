//
// Copyright (c) 2023 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Anti-aliased ray-casting software renderer
//

#ifndef FG_SOFTRENDER_HPP
#define FG_SOFTRENDER_HPP

#include "Fg3dMesh.hpp"
#include "FgLighting.hpp"
#include "FgImage.hpp"
#include "FgSimilarity.hpp"
#include "Fg3dCamera.hpp"

namespace Fg {

enum class RenderSurfPoints { never, whenVisible, always };
inline std::any     toReflect(RenderSurfPoints r) {return static_cast<double>(r); }
inline void         fromReflect_(std::any const & a,RenderSurfPoints & r)
{
    r = scast<RenderSurfPoints>(std::any_cast<double>(a));
}
template<> struct TypeSig<RenderSurfPoints> {static uint64 typeSig() {return 0x5E92C9783665A77AULL; } };
inline void         srlz_(RenderSurfPoints r,Bytes & b) {srlzRaw_(static_cast<uint>(r),b); }
inline void         dsrlz_(Bytes const & b,size_t & p,RenderSurfPoints & r)
{
    r = scast<RenderSurfPoints>(dsrlzT_<uint>(b,p));
}

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
    RgbaF               backgroundColor {0};
    uint                antiAliasBitDepth {3};      // range [1,16]. Higher is slower.
    // Composite marked surface points  as small circles on image:
    RenderSurfPoints    renderSurfPoints=RenderSurfPoints::never;
    RgbaF               surfPointColor {0,1,0,1};           // Values in [0,1], default green
    float               surfPointRadius {1.f};
    // If defined, place the projected surface point data here:
    Sptr<ProjectedSurfPoints> projSurfPoints;
    bool                useMaps = true;     // Turn off to see raw geometry
    bool                allShiny = false;
    FG_SER6(lighting,backgroundColor,antiAliasBitDepth,renderSurfPoints,useMaps,allShiny)
};

struct  RenderXform
{
    SimilarityD            modelview;
    AffineEw2D             itcsToIucs;

    explicit RenderXform(Camera const & c) : modelview(c.modelview), itcsToIucs(c.itcsToIucs) {}
    RenderXform(SimilarityD const & s,AffineEw2D const & a) : modelview(s), itcsToIucs(a) {}
};

inline RenderXform  interpolate(RenderXform const & rx0,RenderXform const & rx1,double val)
{
    return RenderXform {
        interpolateAsModelview(rx0.modelview,rx1.modelview,val),
        interpolate(rx0.itcsToIucs,rx1.itcsToIucs,val)
    };
}

ImgRgba8            renderSoft(
    Vec2UI                  pixelSize,
    Meshes const &          meshes,
    SimilarityD             meshToOecs,     // aka Modelview
    // This fully specifies the projection transform since we assume the optical centre is at the centre of the
    // image and the bounds are implicitly [0,1] in IUCS:
    AffineEw2D              itcsToIucs,
    RenderOptions const &   options=RenderOptions());

inline ImgRgba8     renderSoft(
    Vec2UI              pixelSize,
    Meshes const &      meshes,
    RenderXform const & transform,
    RenderOptions const & options=RenderOptions())
{
    return renderSoft(pixelSize,meshes,transform.modelview,transform.itcsToIucs,options);
}

inline ImgRgba8     renderSoft(
    Vec2UI              pixelSize,
    Meshes const &      meshes,
    Camera const &      camera,
    RenderOptions const & options=RenderOptions())
{
    return renderSoft(pixelSize,meshes,camera.modelview,camera.itcsToIucs,options);
}

// Render with default camera:
ImgRgba8            renderSoft(Vec2UI pixelSize,Meshes const & meshes,RgbaF bgColor);

struct  TriIdxSM
{
    uint32          triIdx;
    uint16          surfIdx = 0;
    uint16          meshIdx = 0;

    TriIdxSM() {}
    TriIdxSM(size_t t,size_t s,size_t m) : triIdx(uint32(t)), surfIdx(uint16(s)), meshIdx(uint16(m)) {}
};
typedef Svec<TriIdxSM>  TriIdxSMs;

}

#endif

// */
