//
// Coypright (c) 2022 Singular Inversions Inc. (facegen.com)
// Use, modification and distribution is subject to the MIT License,
// see accompanying file LICENSE.txt or facegen.com/base_library_license.txt
//

#include "stdafx.h"

#include "FgSoftRender.hpp"
#include "FgSampler.hpp"
#include "Fg3dMesh.hpp"
#include "FgAffine.hpp"
#include "FgGridTriangles.hpp"
#include "FgRayCaster.hpp"
#include "FgMath.hpp"
#include "FgTestUtils.hpp"
#include "Fg3dMeshIo.hpp"
#include "Fg3dCamera.hpp"
#include "FgTime.hpp"
#include "FgMain.hpp"
#include "FgCommand.hpp"
#include "FgImgDisplay.hpp"

using namespace std;
using namespace std::placeholders;

namespace Fg {

ImgRgba8
renderSoft(
    Vec2UI                  pxSz,
    Meshes const &          meshes,
    SimilarityD             modelview,
    AffineEw2D              itcsToIucs,
    RenderOptions const &   options)
{
    ImgRgba8                 img;
    VecF2                   colorBounds = cBounds(options.backgroundColor.m_c);
    FGASSERT((colorBounds[0] >= 0.0f) && (colorBounds[1] <= 255.0f));
    RayCaster               rc(meshes,modelview,itcsToIucs,
        options.lighting,options.backgroundColor,options.useMaps,options.allShiny);
    // The 'cref' for the 'rc' arg is critical; otherwise 'rc' gets copied on every call:
    img = sampleAdaptive(pxSz,bind(&RayCaster::cast,cref(rc),_1),options.antiAliasBitDepth);

    // Calculate where the surface points land:
    ProjectedSurfPoints     spps;
    for (size_t mm=0; mm<meshes.size(); ++mm) {
        Mesh const &            mesh = meshes[mm];
        Vec3Fs const &          verts = rc.vertss[mm];
        MeshNormals const &     norms = rc.normss[mm];
        for (size_t ss=0; ss<mesh.surfaces.size(); ++ss) {
            Surf const &            surf = mesh.surfaces[ss];
            for (size_t ii=0; ii<surf.surfPoints.size(); ++ii) {
                SurfPoint const &       sp = surf.surfPoints[ii];
                ProjectedSurfPoint      spp;
                spp.label = sp.label;
                Vec3F               spOecs = surf.surfPointPos(verts,ii),
                                    spNorm = norms.facet[ss].triEquiv(sp.point.triEquivIdx);
                spp.visible = (cDot(spOecs,spNorm) < 0);           // Point is camera-facing
                Vec3F               spIucs = rc.oecsToIucs(spOecs);
                spp.posIucs = Vec2F(spIucs[0],spIucs[1]);
                if (spIucs[2] > 0) {                                // Point is in front of the camera
                    BestN<float,RayCaster::Intersect,8>  intscts = rc.closestIntersects(Vec2F(spIucs[0],spIucs[1]));
                    if (!intscts.empty()) {                         // Point is in view of camaera
                        RayCaster::Intersect  intsct = intscts[0].second;     // First is closest
                        if ((intsct.triInd.meshIdx != mm) ||
                            (intsct.triInd.surfIdx != ss) ||
                            (intsct.triInd.triIdx != sp.point.triEquivIdx)) {         // Point is occluded
                            spp.visible = false;
                        }
                    }
                    else
                        spp.visible = false;
                }
                else
                    spp.visible = false;
                spps.push_back(spp);
            }
        }
    }
    if (options.projSurfPoints)
        *options.projSurfPoints = spps;
    // Composite surface points:
    if (options.renderSurfPoints != RenderSurfPoints::never) {
        for (const ProjectedSurfPoint & spp : spps) {
            if (spp.visible || (options.renderSurfPoints == RenderSurfPoints::always)) {
                Vec2F               p = spp.posIucs;
                p[0] *= img.width();
                p[1] *= img.height();
                Vec2I               posIrcs = Vec2I(p);
                Vec4UC              color {mapCast<uchar>(options.surfPointColor.m_c * 255.f)};
                paintDot(img,posIrcs,color,2);
                img.paint(posIrcs,Rgba8(color.m));       // Ignores out-of-bounds points
            }
        }
    }
    return img;
}

ImgRgba8
renderSoft(Vec2UI pixelSize,Meshes const & meshes,RgbaF bgColor)
{
    CameraParams        camPrms {Mat32D(cBounds(meshes))};
    Camera              camera = camPrms.camera(pixelSize);
    RenderOptions       ro;
    ro.backgroundColor = bgColor;
    return renderSoft(pixelSize,meshes,camera.modelview,camera.itcsToIucs,ro);
}

static void
testSoftRender(CLArgs const &)
{
    PushDir         pd(dataDir()+"base/test/render/");

    // Set up structures required for rendering:
    Meshes          meshes(1);
    Mesh &          mesh = meshes[0];
    mesh.surfaces.resize(1);
    Surf &          surf = mesh.surfaces[0];
    SimilarityD     modelview;      // Default is identity
    AffineEw2D      itcsToIucs(Vec2D(0.5),Vec2D(0.5));
    RenderOptions   ro;

    // Model a single triangle of equal width and height intersected by the optical axis in OECS at the barycentric centre:
    mesh.verts = { {-1,1.5,-4}, {-1,-1.5,-4}, {2,0,-4} };
    surf.tris.vertInds = {{0,1,2}};         // CC winding
    surf.surfPoints = svec(SurfPoint(0,Vec3F(1.0f/3.0f)));
    // Test that the point is visible in the current configuration:
    ro.renderSurfPoints = RenderSurfPoints::whenVisible;
    ImgRgba8     img = renderSoft(Vec2UI(64),meshes,modelview,itcsToIucs,ro);
    regressTestApprox<ImgRgba8>(img,"t0.png",bind(fgImgApproxEqual,_1,_2,2U));
    // Flip the winding to test the surface point is not visible from behind:
    surf.tris.vertInds.back() = {1,0,2};
    img = renderSoft(Vec2UI(64),meshes,modelview,itcsToIucs,ro);
    regressTestApprox<ImgRgba8>(img,"t4.png",bind(fgImgApproxEqual,_1,_2,2U));
    surf.tris.vertInds.back() = {0,1,2};    // Restore
    // Place a triangle just in front to test occlusion of the surface point:
    mesh.verts.push_back( {2,0,-3.9} );
    surf.tris.vertInds.push_back( {0,1,3} );
    img = renderSoft(Vec2UI(64),meshes,modelview,itcsToIucs,ro);
    regressTestApprox<ImgRgba8>(img,"t3.png",bind(fgImgApproxEqual,_1,_2,2U));


    // Model 2 right angle triangles making a sqaure with a checkerboard color map (preserving aspect ratio):
    mesh.verts = {{-1,1.5,-4}, {-1,-1.5,-4}, {2,1.5,-4}, {2,-1.5,-4}};
    mesh.uvs = {{0,1}, {0,0}, {1,1}, {1,0}};
    surf.tris.vertInds = {{0,1,2}, {2,1,3}};
    surf.tris.uvInds = {{0,1,2}, {2,1,3}};
    ImgRgba8     map(128,128);
    for (Iter2UI it(map.dims()); it.valid(); it.next()) {
        bool        alternate = (it()[0] & 16) != (it()[1] & 16);
        map[it()] = alternate ? Rgba8(0,0,0,255) : Rgba8(255,255,255,255);
    }
    surf.material.albedoMap = make_shared<ImgRgba8>(map);
    // View undistorted checkerboard flat on:
    img = renderSoft(Vec2UI(256),meshes,modelview,itcsToIucs,ro);
    regressTestApprox<ImgRgba8>(img,"t1.png",bind(fgImgApproxEqual,_1,_2,2U));
    // View at an angle to see perspective distortion:
    modelview = SimilarityD(Vec3D(0,0,-4)) * SimilarityD(cRotateY(1.0)) * SimilarityD(Vec3D(0,0,4));
    img = renderSoft(Vec2UI(256),meshes,modelview,itcsToIucs,ro);
    regressTestApprox<ImgRgba8>(img,"t2.png",bind(fgImgApproxEqual,_1,_2,2U));
}

Cmd
testSoftRenderInfo()
{return Cmd(testSoftRender,"rend","renderSoft function"); }

}

// */
