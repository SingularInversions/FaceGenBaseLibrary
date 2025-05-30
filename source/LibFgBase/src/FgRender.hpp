//
// Copyright (c) 2025 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//
// Anti-aliased ray-casting software renderer
//

#ifndef FG_SOFTRENDER_HPP
#define FG_SOFTRENDER_HPP

#include "FgSampler.hpp"
#include "Fg3dMesh.hpp"
#include "FgCamera.hpp"

namespace Fg {

// Accepts a pixel index in IRCS and a coordinate in PACS and returns the RGBA for that position,
// which must have linear alpha-premultiplied RGBA values for correct averaging, and alpha must be in [0,1].
// The pixel index is helpful for determining cache bins since some samples are taken on pixel boundaries.
typedef Sfun<RgbaF(Vec2UI ircs,Vec2F pacs)>  SampleFn;

// Samples RGBA values for each pixel, adaptively sub-sampling based on the difference between
// neighouring values, recursing until precision of the given bit depth is very likely.
// If pixel density <= nyquist frequency implicit in 'sampleFn', artifacts may result.
// Alpha channel may have machine precision errors and not be exactly 1 even where fully sampled.
// Returned image channel values remain alpha-premultiplied (as 'sampleFn' must provide):
ImgC4F              sampleAdaptiveF(
    Vec2UI              dims,                   // Must be non-zero
    SampleFn const &    sampleFn,
    float               channelBound=1,         // Sampler must return channel values in [0,channelBound)
    uint                antiAliasBitDepth=3);   // Must be in [1,16]

enum class RenderSurfPoints { never, whenVisible, always };
std::any            toReflect(RenderSurfPoints r);
void                fromReflect_(std::any const & a,RenderSurfPoints & r);
template<> struct TS<RenderSurfPoints> {static uint64 typeSig() {return 0x5E92C9783665A77AULL; } };
inline void         srlz_(RenderSurfPoints r,Bytes & b) {srlzRaw_(static_cast<uint>(r),b); }
inline void         dsrlz_(Bytes const & b,size_t & p,RenderSurfPoints & r)
{
    r = scast<RenderSurfPoints>(dsrlzT_<uint>(b,p));
}

template<class T>
struct      ProjTri                     // projected triangle
{
    Arr<Vec2D,3>        pacs;           // projected vertex coordinates in PACS
    Arr3D               invDepths;      // respective inverse depth values
    T                   td;             // application-specific triangle data
};

// 'triIdx' is required for looking up UVs if applicable, 'triVertInds' is just a cache:
template<class T> using FnTriDat = Sfun<T(uint triIdx,Arr3UI triVertInds)>;

template<class T>
void                accProjTriIndex_(
    uint                pixPerBin,      // linear factor for pixels per grid bin
    Arr3UIs const &     tris,           // elements are indices into 'projVerts' below
    ProjVerts const &   projVerts,      // verts on or behind the camera plane must be invalid
    FnTriDat<T> const & cTriDat,        // given tri indices, return required client data
    Img<Svec<ProjTri<T>>> & grid)       // accumulate valid projected tris into grid index
{
    double              binsPerPix = 1.0 / pixPerBin;
    for (size_t ii=0; ii<tris.size(); ++ii) {
        Arr3UI              tri = tris[ii];
        Arr<ProjVert,3>     pvt = mapIndex(tri,projVerts);
        if (isRendered(pvt)) {          // discard back-facing and tris that cross the camera plane
            Arr<Vec2D,3>        pacs = mapMember(pvt,&ProjVert::pacs);
            Arr3D               invDepths = mapMember(pvt,&ProjVert::invDepth);
            ProjTri<T>          pt {pacs,invDepths,cTriDat(scast<uint>(ii),tri)};
            Mat22D              bounds = catH(cBounds(pacs));
            Mat22UI             boundsEub {intersectBoundsToImage(bounds*binsPerPix,grid.dims())};
            for (Iter2 it{boundsEub}; it.valid(); it.next())
                grid[it()].push_back(pt);
        }
    }
}

template<class T>
Img<Svec<ProjTri<T>>>   cProjTriIndex(
    Vec2UI              dims,
    uint                pixPerBin,
    Arr3UIs const &     tris,
    ProjVerts const &   pvs,
    FnTriDat<T> const & cTriDat)
{
    Img<Svec<ProjTri<T>>>   ret {dims};
    accProjTriIndex_(pixPerBin,tris,pvs,cTriDat,ret);
    return ret;
}

template<class T>                       // triangle application-specific data type
struct          RayIsct
{
    ProjTri<T> const *  prjTri;         // nullptr if not yet set
    double              invDepth;       // 0 if not yet set
    Arr3D               bcs;            // screen-space barycentric coordinate of intersection

    RayIsct() : prjTri{nullptr}, invDepth{0} {}      // invalid and at infinite depth
    RayIsct(ProjTri<T> const & p,double i,Arr3D const & b) : prjTri{&p}, invDepth{i}, bcs{b} {}

    bool                valid() const {return (prjTri != nullptr); }
    // get camera (model) space barycentric coordinate of intersection (gridTri must be valid):
    Arr3D               bcm() const {return mapMul(bcs,prjTri->invDepths) / invDepth; }
};

// find the closest ray intersect (above an optional minimum depth) in the given list of tris.
// return value invalid if no intersections found.
template<class T>
RayIsct<T>          cClosestIsct(
    Svec<ProjTri<T>> const & prjTris,       // All must have non-null tris and invDepths > 0 (neither are checked)
    Vec2D               rayPacs,
    double              maxInvDepth=lims<double>::max())
{
    RayIsct<T>          closest;
    for (ProjTri<T> const & pt : prjTris) {
        Arr3D               bcs = cBarycentricCoord(rayPacs,pt.pacs).value();
        if (allGteZero(bcs)) {          // intersection
            double              invDepth = multAcc(bcs,pt.invDepths);
            if ((invDepth > closest.invDepth) && (invDepth < maxInvDepth))
                closest = {pt,invDepth,bcs};
        }
    }
    return closest;
}

template<class GT>                  // type containing information about the intersected tri
struct          RayIsect
{
    GT const *      gridTri;        // nullptr if not yet set
    double          invDepth;       // 0 if not yet set (distance infinity)
    Arr3D           bcs;            // screen-space barycentric coordinate of intersection

    RayIsect() : gridTri{nullptr}, invDepth{0} {}      // invalid and at infinite depth
    RayIsect(GT const & gt,double id,Arr3D const & b) : gridTri{&gt}, invDepth{id}, bcs{b} {}

    bool            valid() const {return (gridTri != nullptr); }
    // get camera (model) space barycentric coordinate of intersection (gridTri must be valid):
    Arr3D           bcm() const {return mapMul(bcs,gridTri->invDepths) / invDepth; }
};
// find the ray intersect with the largest inverse depth (less than the optional max) in the given list of tris.
// class GT must contain:
// Arr<Vec3D,3>     pacs;       // projection of triangle vertices into image in PACS CS
// Arr3D            invDepths   // respective inverse depths in camera CS
template<class GT>
RayIsect<GT>        cClosestIntersect(
    Svec<GT> const &    gridTris,           // All GTs must have non-null tris and invDepths > 0 (neither are checked)
    Vec2D               rayPacs,
    double              maxInvDepth=lims<double>::max())
{
    RayIsect<GT>        closest;
    for (GT const & gt : gridTris) {
        Arr3D               bcs = cBarycentricCoord(rayPacs,gt.pacs).value();
        if (allGteZero(bcs)) {          // intersection
            double              invDepth = multAcc(bcs,gt.invDepths);
            if ((invDepth > closest.invDepth) && (invDepth < maxInvDepth))
                closest = {gt,invDepth,bcs};
        }
    }
    return closest;
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
    FG_SER(lighting,backgroundColor,antiAliasBitDepth,renderSurfPoints,useMaps,allShiny)
};

struct  RenderXform
{
    SimilarityD            modelview;
    AxAffine2D             itcsToIucs;

    explicit RenderXform(Camera const & c) : modelview(c.modelview), itcsToIucs(c.itcsToIucs) {}
    RenderXform(SimilarityD const & s,AxAffine2D const & a) : modelview(s), itcsToIucs(a) {}
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
    AxAffine2D              itcsToIucs,
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
